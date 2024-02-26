#include "strpg.h"
#include "layout.h"

/* stupidest layout: 1D, nodes preordered by id */
/* nodes and edges are duplicated, but realize that everything else is
 * supposed to in fine unravel/unload while we go */

static void
init(Graph *g)
{
	int x;
	ssize i;
	Node *u;

	for(i=g->node0.next, x=0; i>=0; i=u->next, x+=Nodesz+10*Ptsz){
		u = g->nodes + i;
		putnode(u, x, 0);
	}
}

static void
compute(Graph *)
{
}

static Layout ll = {
	.name = "congaline",
	.init = init,
	.compute = compute,
};

Layout *
regconga(void)
{
	return &ll;
}
