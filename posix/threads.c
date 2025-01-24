#include "strpg.h"
#include "threads.h"

int mainstacksize = 2*1024*1024;

struct Thread{
	pthread_t p;
	pthread_key_t key;
	void *arg;
	void *data;
	void (*fn)(void*);
	void (*cleanfn)(void*);
};

/* couldn't figure out a better way */
static __thread pthread_key_t pkey;

static void
wipe(void *tp)
{
	Thread *th;

	if((th = tp) == NULL)
		return;
	if(th->cleanfn != nil)
		th->cleanfn(th->data);
	free(th);
}

/* don't use pthread_exit explicitely:
 * https://www.sourceware.org/bugzilla/show_bug.cgi?id=13199 */
static void *
_thread(void *tp)
{
	Thread *th;

	th = tp;
	pthread_cleanup_push(wipe, th);
	pkey = th->key;
	if(pthread_setspecific(pkey, th) != 0)
		sysfatal("newthread: pthread_setspecific: %s", error());
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	th->fn(th->arg);
	pthread_cleanup_pop(1);
	return NULL;
}

void
newthread(void (*fn)(void*), void (*cleanfn)(void*), void *arg, void *data, char *name, uint stacksize)
{
	usize st;
	pthread_attr_t a;
	Thread *th;
	pthread_t *p;

	th = emalloc(sizeof *th);
	p = &th->p;
	th->fn = fn;
	th->cleanfn = cleanfn;
	th->arg = arg;
	th->data = data;
	if(pthread_attr_init(&a) != 0)
		sysfatal("newthread: pthread_attr_init: %s", error());
	if(pthread_attr_getstacksize(&a, &st) != 0)
		sysfatal("newthread: pthread_attr_getstacksize: %s", error());
	if(st < stacksize){
		st = stacksize;
		if(pthread_attr_setstacksize(&a, st) != 0)
			sysfatal("newthread: pthread_attr_setstacksize: %s", error());
	}
	if(pthread_key_create(&th->key, NULL) != 0)
		sysfatal("newthread: pthread_key_create: %s", error());
	if(pthread_create(p, &a, _thread, th) != 0)
		sysfatal("newthread: pthread_create: %s", error());
	if(pthread_attr_destroy(&a) != 0)
		sysfatal("newthread: pthread_attr_destroy: %s", error());
	/* pthread_*_np is non-portable gnushit
	if(pthread_setname_np(*p, name) != 0)
		warn("pthread_setname_np: %s", error());
	*/
}

void *
threadstore(void *p)
{
	Thread *th;

	if((th = pthread_getspecific(pkey)) == NULL)
		sysfatal("newthread: pthread_getspecific: %s", error());
	if(p == NULL)
		return th->data;
	else
		return th->data = p;
}

void
killthread(Thread *th)
{
	if(th == nil)
		return;
	pthread_cancel(th->p);
}

void
initrwlock(RWLock *l)
{
	if(pthread_rwlock_init(l, NULL) != 0)
		sysfatal("pthread_rwlock_init: %s", error());
}

Channel *
chancreate(int elsize, int nel)
{
	USED(elsize);
	return chan_init(nel);
}

int
sendul(Channel *c, ulong n)
{
	uintptr v;

	v = (uintptr)n;
	if(chan_send(c, (void*)v) < 0)
		return -1;
	return 0;
}

void *
recvp(Channel *c)
{
	void *p;

	if(chan_recv(c, &p) < 0)
		return nil;
	return p;
}

ulong
recvul(Channel *c)
{
	uintptr v;

	if(chan_recv(c, (void **)&v) < 0)
		return 0;
	return (ulong)v;
}

int
nbsendp(Channel *c, void *p)
{
	if(chan_select(nil, 0, nil, &c, 1, &p) < 0)
		return 0;
	return 1;
}

int
nbsendul(Channel *c, ulong n)
{
	uintptr v;

	v = (uintptr)n;
	if(chan_select(nil, 0, nil, &c, 1, (void**)&v) < 0)
		return 0;
	return 1;
}

ulong
nbrecvul(Channel *c)
{
	uintptr v;

	if(chan_select(&c, 1, (void **)&v, nil, 0, nil) < 0)
		return 0;
	return (ulong)v;
}

void*
nbrecvp(Channel *c)
{
	void *p;

	if(chan_select(&c, 1, &p, nil, 0, nil) < 0)
		return nil;
	return p;
}
