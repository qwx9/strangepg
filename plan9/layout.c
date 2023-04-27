#include "strpg.h"
#include <thread.h>

static Channel *echan;

static void
wproc(void *gp)
{
	Graph *g;
	Layout *l;

	threadsetname("workproc");
	g = gp;
	assert((l = g->ll) != nil);
	dprint("new job %d: compute layout %s for graph %#p\n", getpid(), l->name, g);
	if(l->compute(g) < 0)
		warn("compute: %r");
	g->stale = 0;
	g->working = 0;
	nbrecvul(echan);
	threadexits(nil);
}

int
earlyexit(void)
{
	return nbrecvul(echan) != 0;
}

int
runlayout(Graph *g)
{
	if(echan == nil && (echan = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	dprint("runlayout %s %#p\n", g->ll == nil ? "" : g->ll->name, g);
	if(g->working){
		nbsendul(echan, 0);
		while(wait() != nil){
			warn("runlayout: still waiting\n");
			sleep(10);
		}
	}
	g->working = 1;
	if(proccreate(wproc, g, mainstacksize) < 0)
		sysfatal("proccreate: %r");
	return 0;
}
