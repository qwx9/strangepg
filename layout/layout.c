#include "strpg.h"
#include "layout.h"
#include "threads.h"

int deflayout = LLfr;

static Layout *lltab[LLnil];

static thret_t
layproc(void *th)
{
	vlong t;
	Graph *g;
	Layout *ll;

	namethread(th, "layproc");
	g = ((Thread *)th)->arg;
	ll = g->layout.ll;
	DPRINT(Debuglayout, "new job layout %s g %#p", ll->name, g);
	t = (debug & Debugperf) != 0 ? μsec() : 0;
	ll->compute(g);
	DPRINT(Debugperf, "layout: %lld μs", μsec() - t);
	g->layout.f &= ~LFbusy;
	reqdraw(Reqrender);
	exitthread(th, nil);
}

void
stoplayout(Graph *g)
{
	if((g->layout.f & LFbusy) == 0)
		return;
	g->layout.f &= ~LFbusy;
	killthread(g->layout.thread);
	g->layout.thread = nil;
}

void
runlayout(Graph *g)
{
	if((g->layout.f & LFonline) == 0){
		warn("runlayout: %#p layout unarmed\n", g);
		return;
	}
	stoplayout(g);
	g->layout.f |= LFbusy;
	g->layout.thread = newthread(layproc, g, mainstacksize);
	if(!noui)
		startdrawclock();
}

// FIXME: kill it
void
putnode(Node *u, int x, int y)
{
	u->vrect.o = Vec2(x, y);
	u->vrect.v = ZV;
}

int
updatelayout(Graph *g)
{
	stoplayout(g);
	runlayout(g);
	return 0;
}

int
resetlayout(Graph *g)
{
	stoplayout(g);
	g->layout.f &= ~LFarmed;
	if(g->layout.aux != nil){
		free(g->layout.aux);
		g->layout.aux = nil;
	}
	runlayout(g);
	return 0;
}

int
newlayout(Graph *g, int type)
{
	Layout *ll;

	if(dylen(g->nodes) < 1){
		werrstr("empty graph");
		return -1;
	}
	if(type < 0){
		ll = lltab[deflayout];
	}else if(type >= LLnil){
		werrstr("invalid layout");
		return -1;
	}else
		ll = lltab[type];
	assert(ll != nil);
	g->layout.ll = ll;
	DPRINT(Debuglayout, "newlayout g %#p ll %#p %s", g, ll, ll->name);
	if(ll->compute == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	if(ll->init != nil)
		ll->init(g);
	g->layout.f |= LFonline;
	runlayout(g);
	return 0;
}

void
initlayout(void)
{
	lltab[LLconga] = regconga();
	lltab[LLforce] = regforce();
	lltab[LLrandom] = regrandom();
	lltab[LLfr] = regfr();
	lltab[LLlinear] = reglinear();
}
