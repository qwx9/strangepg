#include "strpg.h"
#include "layout.h"
#include "threads.h"

int deflayout = LLfr;

static Layout *lltab[LLnil];

static void
layproc(void *arg)
{
	vlong t;
	Graph *g;
	Layout *ll;

	g = arg;
	ll = g->layout.ll;
	DPRINT(Debuglayout, "new job layout %s g %#p", ll->name, g);
	t = (debug & Debugperf) != 0 ? μsec() : 0;
	ll->compute(g);
	g->layout.th = nil;
	DPRINT(Debugperf, "layout: %lld μs", μsec() - t);
	g->layout.f &= ~LFbusy;
	reqdraw(Reqrender);
}

static void
runlayout(Graph *g)
{
	Layouting *l;

	l = &g->layout;
	if((l->f & LFonline) == 0){
		warn("runlayout: %#p layout unarmed\n", g);
		return;
	}
	l->f |= LFbusy;
	l->th = newthread(layproc, l->ll->cleanup, g, nil, l->ll->name, mainstacksize);
	if(!noui)
		startdrawclock();
}

void
stoplayout(Graph *g)
{
	Layouting *l;

	l = &g->layout;
	if((l->f & LFbusy) == 0)
		return;
	l->f &= ~LFbusy;
	killthread(l->th);
	l->th = nil;	/* FIXME: potential race */
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
	Layouting *l;

	stoplayout(g);
	l = &g->layout;
	l->f &= ~LFarmed;
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
	lltab[LLpfr] = regpfr();
	lltab[LLpline] = regpline();
}
