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
static __thread Thread *thread;

static void
wipe(void *tp)
{
	Thread *th;

	if((th = tp) == NULL)
		return;
	warn("wipe %#p\n", th);
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
	thread = th;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	//pthread_cleanup_push(th->cleanfn, th->data);
	pthread_cleanup_push(wipe, th);
	/*
	if(pthread_key_create(&th->key, wipe) != 0)
		sysfatal("newthread: pthread_key_create: %s", error());
	if(pthread_setspecific(th->key, th) != 0)
		sysfatal("newthread: pthread_setspecific: %s", error());
	*/
	th->fn(th->arg);
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
	return NULL;
}

Thread *
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
	if(pthread_create(p, &a, _thread, th) != 0)
		sysfatal("newthread: pthread_create: %s", error());
	if(pthread_attr_destroy(&a) != 0)
		sysfatal("newthread: pthread_attr_destroy: %s", error());
	/* pthread_*_np is non-portable gnushit */
	/*if(pthread_setname_np(p, name) != 0)
		sysfatal("pthread_setname_np: %s", error());*/
	return th;
}

void *
threadstore(void *p)
{
	Thread *th;

	if((th = thread) == nil)
		return nil;
	if(p == nil)
		return th->data;
	else
		return th->data = p;
	return p;
}

/* FIXME: this implementation allows for borrowing the Thread* from
 * newthread then accessing it again after it was freed here; we should
 * instead have a different, *simple* and opaque way to do this (thread
 * groups?); ie. never kill a thread that is about to exit on its own */
void
killthread(Thread *th)
{
	if(th == nil)
		return;
	pthread_cancel(th->p);
	//waitpid(th->aux, NULL, 0);
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
	if(chan_select(nil, 0, nil, &c, 1, (void**)&n) < 0)
		return 0;
	return 1;
}

ulong
nbrecvul(Channel *c)
{
	ulong n;

	if(chan_select(&c, 1, (void **)&n, nil, 0, nil) < 0)
		return 0;
	return n;
}
