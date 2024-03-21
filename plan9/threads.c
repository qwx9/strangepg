#include "strpg.h"
#include "threads.h"

RWLock renderlock, drawlock;

Thread *
newthread(thret_t (*fp)(void*), void *arg, uint stacksize)
{
	Thread *t;

	t = emalloc(sizeof *t);
	t->arg = arg;
	// FIXME: don't always use mainstacksize in calls
	if((t->pid = proccreate(fp, t, stacksize)) < 0)
		sysfatal("newthread: %r");
	return t;
}

void
initthread(Thread *, char *s)
{
	threadsetname(s);
}

void
killthread(Thread *t)
{
	if(t == nil)
		return;
	threadkill(t->pid);
	//waitpid(t->aux, NULL, 0);
	free(t->aux);
	free(t);
}

void
exitthread(Thread *t, char *err)
{
	if(err)
		warn("thread %d: %s\n", t->pid, err);
	free(t->aux);
	free(t);
	threadexits(err);
}
