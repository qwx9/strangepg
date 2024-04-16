#include "strpg.h"
#include "threads.h"

struct Thread{
	int tid;
	int closing;
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

	if((th = *procdata()) == nil)
		return;
	th->closing++;
	if(th->closing != 1)
		return;
	if(th->cleanfn != nil){
		th->cleanfn(th->data);
		th->data = nil;
	}
	free(th);
}

static void
_thread(void *tp)
{
	Thread *th;

	th = tp;
	*procdata() = th;
	if(th->name != nil)
		threadsetname(th->name);
	th->fn(th->arg);
	wipe();
	threadexits(nil);
}

/* any error here is severe and treated as unrecoverable */
void
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
	if((tid = proccreate(_thread, th, stacksize)) < 0)
		sysfatal("newthread: %r");
	th->tid = tid;
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
