#include "strpg.h"
#include "layout.h"

int deflayout = LLforce;

static Layout *lltab[LLnil];

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
	if(g->layout.tid >= 0){
		werrstr("updatelayout: already running");
		return -1;
	}
	// FIXME: actually implement update
	runlayout(g);
	return 0;
}

int
resetlayout(Graph *g)
{
	stoplayout(g);
	runlayout(g);
	return 0;
}

int
newlayout(Graph *g, int type)
{
	Layout *ll;

	stoplayout(g);
	if(g->len < 1){
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
	dprint(Debuglayout, "newlayout g %#p ll %#p %s\n", g, ll, ll->name);
	if(ll->compute == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
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
