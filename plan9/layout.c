#include "strpg.h"
#include "layout.h"

static void
layproc(void *gp)
{
	Graph *g;
	Layout *ll;

	threadsetname("layproc");
	g = gp;
	ll = g->layout.ll;
	DPRINT(Debuglayout, "new layout job %d layout %s g %#p", getpid(), ll->name, g);
	ll->compute(g);
	reqdraw(Reqrefresh);
	g->layout.tid = -1;
	threadexits(nil);
}

/* we interrupt layouting to reset it, otherwise it just runs its course,
 * thus only kill layout proc and let the rest die naturally */
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
	if(!g->layout.armed)
		return;
	assert(g->layout.tid < 0);
	if((g->layout.tid = proccreate(layproc, g, mainstacksize)) < 0)
		sysfatal("runlayout: %r");
	if(!noui)
		startdrawclock();
}
