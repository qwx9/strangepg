#include "strpg.h"
#include "threads.h"

/* fuck you pthreads */
Thread *
newthread(thret_t (*fp)(void*), void *arg, uint stacksize)
{
	int tid;
	Thread *t;

	t = emalloc(sizeof *t);
	t->arg = arg;
	if((tid = proccreate(fp, t, stacksize)) < 0)
		sysfatal("newthread: %r");
	t->tid = tid;
	return t;
}

void
namethread(Thread *, char *s)
{
	threadsetname(s);
}

void
killthread(Thread *t)
{
	if(t == nil)
		return;
	threadkill(t->tid);
	waitpid();
	free(t);
}
