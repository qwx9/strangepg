#include "strpg.h"
#include "threads.h"

int mainstacksize = 2*1024*1024;	/* FIXME: default for pthreads? */

Thread *
newthread(thret_t (*fp)(void*), void *arg, uint stacksize)
{
	int r;
	usize st;
	pthread_attr_t a;
	Thread *t;
	pthread_t *p;

	t = emalloc(sizeof *t);
	p = &t->p;
	t->arg = arg;
	if((r = pthread_attr_init(&a)) != 0)
		sysfatal("newthread: pthread_attr_init: %d %s", r, error());
	if((r = pthread_attr_getstacksize(&a, &st)) != 0)
		sysfatal("newthread: pthread_attr_getstacksize: %d %s", r, error());
	if(st < stacksize){
		st = stacksize;
		if((r = pthread_attr_setstacksize(&a, st)) != 0)
			sysfatal("newthread: pthread_attr_setstacksize: %d %s", r, error());
	}
	if((r = pthread_create(p, &a, fp, t)) != 0)
		sysfatal("newthread: pthread_create: %d %s", r, error());
	if((r = pthread_attr_destroy(&a)) != 0)
		sysfatal("newthread: pthread_attr_destroy: %d %s", r, error());
	return t;
}

void
cleanthread(Thread *t)
{
	free(t->arg);
	free(t);
}

/* pthread_setname_np is non-portable gnushit */
void
namethread(Thread *, char *)
{
	/*if((r = pthread_setname_np(p, s)) != 0)
		sysfatal("pthread_setname_np: %d %s", r, error());*/
}

void
killthread(Thread *t)
{
	if(t == nil)
		return;
	pthread_cancel(t->p);
	//waitpid(t->aux, NULL, 0);
	cleanthread(t);
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
	int r;

	if(chan_select(nil, 0, nil, &c, 1, &p) < 0)
		return 0;
	return 1;
}
