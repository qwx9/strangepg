#include "strpg.h"
#include "threads.h"

struct Thread{
	int tid;
	int pid;
	void *arg;
	void *data;
	char *name;
	void (*fn)(void*);
	void (*cleanfn)(void*);
};

static void
wipe(void)
{
	Thread *th;

	warn("wipe %#p\n", *procdata());
	if((th = *procdata()) == nil)
		return;
	if(th->cleanfn != nil)
		th->cleanfn(th->data);
	free(th);
}

static void
_thread(void *tp)
{
	Thread *th;

	th = tp;
	*procdata() = th;
	atexit(wipe);
	if(th->name != nil)
		threadsetname(th->name);
	th->pid = getpid();
	threadsetgrp(th->pid);
	th->fn(th->arg);
	exits(nil);
}

Thread *
newthread(void (*fn)(void*), void (*cleanfn)(void*), void *arg, void *data, char *name, uint stacksize)
{
	int tid;
	Thread *th;

	th = emalloc(sizeof *th);
	th->fn = fn;
	th->cleanfn = cleanfn;
	th->arg = arg;
	th->data = data;
	th->name = name;
	if((tid = procrfork(_thread, th, stacksize, RFNOTEG)) < 0)
		sysfatal("newthread: %r");
	th->tid = tid;
	return th;
}

void *
threadstore(void *p)
{
	Thread *th;

	if((th = *procdata()) == nil)
		return nil;
	if(p == nil)
		return th->data;
	else
		return th->data = p;
}

/* FIXME: if the thread doesn't yield in time, we're fucked */
void
killthread(Thread *th)
{
	warn("kill %#p %s\n", th, th!=nil?th->name:"");
	if(th == nil)
		return;
	/* FIXME */
	threadkill(th->tid);
	postnote(PNGROUP, threadpid(th->tid), "die yankee pigdog");
	threadkillgrp(th->pid);
	if(th->cleanfn != nil)
		th->cleanfn(th->data);
}
