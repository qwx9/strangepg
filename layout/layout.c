#include "strpg.h"
#include "layout.h"

int deflayout = LLforce;

/* nodes are points in space, distances are in unit vectors */
/* edges: retrieved from graph; could be filtered, but not here */
/* representation: pretty much a binary map except we need node ids;
 * 	it's renderer's job to inflate nodes, draw's job to put edges */

static Layout *ll[LLnil];

void
putnode(Node *u, int x, int y)
{
	u->vrect.o = Vec2(x, y);
	u->vrect.v = ZV;
}

int
dolayout(Graph *g, int type)
{
	if(g->nodes.len < 1){
		werrstr("empty graph");
		return -1;
	}
	if(type < 0){
		if(g->ll == nil)
			g->ll = ll[deflayout];
	}else if(type >= LLnil){
		werrstr("invalid layout");
		return -1;
	}else
		g->ll = ll[type];
	assert(g->ll != nil);
	if(g->ll->compute == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	runlayout(g);
	return 0;
}

void
initlayout(void)
{
	ll[LLconga] = regconga();
	ll[LLforce] = regforce();
	ll[LLrandom] = regrandom();
}
