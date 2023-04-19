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
	/* 0-size quad */
	u->q.u = (Vertex){x, y};
	u->q.v = u->q.u;
}

int
dolayout(Graph *g, int type)
{
	Layout *l;

	if(type < 0 || type >= LLnil){
		werrstr("invalid layout");
		return -1;
	}
	l = ll[type];
	assert(l != nil);
	if(l->compute == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	g->ll = l;
	return l->compute(g);
}

void
initlayout(void)
{
	ll[LLconga] = regconga();
	ll[LLforce] = regforce();
	ll[LLrandom] = regrandom();
}
