#include "strpg.h"
#include "layout.h"

/* stupidest layout: 1D, nodes preordered by id */
/* nodes and edges are duplicated, but realize that everything else is
 * supposed to in fine unravel/unload while we go */

static void
compute(Graph *g)
{
	int x;
	Node *u, *ue;

	for(u=g->nodes, ue=u+dylen(g->nodes), x=0; u<ue; u++, x+=Nodesz+10*Ptsz){
		putnode(u, x, 0);
		yield();
	}
}

static Layout ll = {
	.name = "congaline",
	.compute = compute,
};

Layout *
regconga(void)
{
	return &ll;
}
