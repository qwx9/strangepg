#include "strpg.h"
#include <thread.h>

static void
layproc(void *gp)
{
	Graph *g;
	Layout *ll;

	threadsetname("layproc");
	g = gp;
	ll = g->layout.ll;
	dprint(Debuglayout, "new layout job %d layout %s g %#p\n", getpid(), ll->name, g);
	ll->compute(g);
	g->layout.tid = -1;
	reqdraw(Reqrefresh);
	threadexits(nil);
}

void
stoplayout(Graph *g)
{
	if(g->layout.tid < 0)
		return;
	threadkill(g->layout.tid);
	g->layout.tid = -1;
}

void
runlayout(Graph *g)
{
	if((g->layout.tid = proccreate(layproc, g, mainstacksize)) < 0)
		sysfatal("runlayout: %r");
	if(!noui)
		startdrawclock();
}
