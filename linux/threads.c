#include "strpg.h"
#include "threads.h"

int mainstacksize = 2*1024*1024;	/* FIXME: default for pthreads? */

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
	void *p;
	Thread *th;

	th = tp;
	pkey = th->key;
	if(pthread_setspecific(pkey, th) != 0)
		sysfatal("newthread: pthread_setspecific: %s", error());
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_cleanup_push(wipe, th);
	th->fn(th->arg);
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
	return NULL;
}

void
newthread(void (*fn)(void*), void (*cleanfn)(void*), void *arg, void *data, char *, uint stacksize)
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
	/* pthread_*_np is non-portable gnushit */
	/*if(pthread_setname_np(p, name) != 0)
		sysfatal("pthread_setname_np: %s", error());*/
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

Channel *
chancreate(int elsize, int nel)
{
	return chan_init(elsize * nel);
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
	u32int v;

	if(chan_recv_int32(c, &v) < 0)
		return 0;
	return v;
}

int
nbsendp(Channel *c, void *p)
{
	if(chan_select(nil, 0, nil, &c, 1, &p) < 0)
		return 0;
	return 1;
}

/* ONLY the _send_type functions alloc memory, so if one of the
 * _recv_type functions gets something sent via this select shit,
 * it will get an invalid pointer and will also try to free it */
int
nbsendul(Channel *c, ulong n)
{
	ulong *ass;

	ass = emalloc(sizeof *ass);
	*ass = n;
	if(chan_select(nil, 0, nil, &c, 1, (void**)&ass) < 0)
		return 0;
	return 1;
}

ulong
nbrecvul(Channel *c)
{
	u32int n;

	if(chan_select(&c, 1, (void **)&n, nil, 0, nil) < 0)
		return 0;
	return n;
}

void*
nbrecvp(Channel *c)
{
	void *p;

	if(chan_select(&c, 1, &p, nil, 0, nil) < 0)
		return nil;
	return p;
}
