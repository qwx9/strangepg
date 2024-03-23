#include "strpg.h"
#include "layout.h"
#include "threads.h"

int deflayout = LLforce;

static Layout *lltab[LLnil];

static thret_t
layproc(void *th)
{
	Graph *g;
	Layout *ll;

	namethread(th, "layproc");
	g = ((Thread *)th)->arg;
	ll = g->layout.ll;
	DPRINT(Debuglayout, "new job layout %s g %#p", ll->name, g);
	ll->compute(g);
	g->layout.f &= ~LFonline;
	reqdraw(Reqrender);
	exitthread(th, nil);
}

void
stoplayout(Graph *g)
{
	if((g->layout.f & LFonline) == 0)
		return;
	g->layout.f &= ~LFonline;
	killthread(g->layout.thread);
	g->layout.thread = nil;
}

void
runlayout(Graph *g)
{
	if((g->layout.f & LFarmed) == 0){
		warn("runlayout: %#p layout unarmed\n", g);
		return;
	}
	stoplayout(g);
	g->layout.f |= LFonline;
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

// FIXME: restrict update to selection
int
updatelayout(Graph *g)
{
	stoplayout(g);
	// FIXME: actually implement update
	runlayout(g);
	return 0;
}

int
resetlayout(Graph *g)
{
	stoplayout(g);
	g->layout.ll->init(g);
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
	ll->init(g);
	g->layout.f |= LFarmed;
	runlayout(g);
	return 0;
}

void
initlayout(void)
{
	lltab[LLconga] = regconga();
	lltab[LLforce] = regforce();
	lltab[LLrandom] = regrandom();
}
