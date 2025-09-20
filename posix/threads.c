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

static void *
_thread(void *tp)
{
	int r;
	Thread *th;

	th = tp;
	pthread_cleanup_push(wipe, th);
	pkey = th->key;
	if((r = pthread_setspecific(pkey, th)) != 0)
		sysfatal("pthread_setspecific: %s", strerror(r));
	if((r = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) != 0)
		sysfatal("pthread_setcancelstate: %s", strerror(r));
	if((r = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) != 0)
		sysfatal("pthread_setcanceltype: %s", strerror(r));
	th->fn(th->arg);
	pthread_cleanup_pop(1);
	return NULL;
}

void
newthread(void (*fn)(void*), void (*cleanfn)(void*), void *arg, void *data, char *name, uint stacksize)
{
	int r;
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
	if((r = pthread_attr_init(&a)) != 0)
		sysfatal("newthread: pthread_attr_init: %s", strerror(r));
	if((r = pthread_attr_getstacksize(&a, &st)) != 0)
		sysfatal("newthread: pthread_attr_getstacksize: %s", strerror(r));
	if(st < stacksize){
		st = stacksize;
		if((r = pthread_attr_setstacksize(&a, st)) != 0)
			sysfatal("newthread: pthread_attr_setstacksize: %s", strerror(r));
	}
	if((r = pthread_key_create(&th->key, NULL)) != 0)
		sysfatal("newthread: pthread_key_create: %s", strerror(r));
	if((r = pthread_create(p, &a, _thread, th)) != 0)
		sysfatal("newthread: pthread_create: %s", strerror(r));
	if((r = pthread_attr_destroy(&a)) != 0)
		sysfatal("newthread: pthread_attr_destroy: %s", strerror(r));
	/* pthread_*_np is non-portable gnushit
	if((r = pthread_setname_np(*p, name)) != 0)
		warn("pthread_setname_np: %s", strerror(r));
	*/
}

void *
threadstore(void *p)
{
	Thread *th;

	if((th = pthread_getspecific(pkey)) == NULL)
		sysfatal("newthread: pthread_getspecific: invalid key");
	if(p == NULL)
		return th->data;
	else
		return th->data = p;
}

void
killthread(Thread *th)
{
	int r;

	if(th == nil)
		return;
	if((r = pthread_cancel(th->p)) != 0)
		warn("pthread_cancel: %s\n", strerror(r));
}

void
initqlock(QLock *l)
{
	int r;

	if((r = pthread_mutex_init(l, NULL)) != 0)
		sysfatal("pthread_mutex_init: %s", strerror(r));
}

void
initrwlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_init(l, NULL)) != 0)
		sysfatal("pthread_rwlock_init: %s", strerror(r));
}

void
nukerwlock(RWLock *l)
{
	int r;

	if((r = pthread_rwlock_destroy(l)) != 0)
		sysfatal("pthread_rwlock_destroy: %s", strerror(r));
}

Channel *
chancreate(int elsize, int nel)
{
	assert(elsize <= sizeof(void *));	/* fuck. you. */
	return chan_init(nel);
}

int
sendul(Channel *c, ulong n)
{
	uintptr v;

	v = (uintptr)n;
	return chan_send(c, (void*)v);
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

	if(chan_recv(c, (void **)&v) <= 0)
		return -1;
	return (ulong)v;
}

int
nbsendp(Channel *c, void *p)
{
	return chan_nbsend(c, p);
}

int
nbsendul(Channel *c, ulong n)
{
	uintptr v;

	v = (uintptr)n;
	return chan_nbsend(c, (void *)v);
}

ulong
nbrecvul(Channel *c)
{
	int r;
	uintptr v;

	if((r = chan_nbrecv(c, (void **)&v)) <= 0)
		return 0;
	return (ulong)v;
}

void*
nbrecvp(Channel *c)
{
	int r;
	void *p;

	if((r = chan_nbrecv(c, &p)) <= 0)
		return nil;
	return p;
}
