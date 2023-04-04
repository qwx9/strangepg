#include "strpg.h"
#include "layoutprv.h"

/* stupidest layout: 1D, nodes preordered by id */
/* nodes and edges are duplicated, but realize that everything else is
 * supposed to in fine unravel/unload while we go */

static int
compute(Graph *g)
{
	int x;
	Node *u, *ue;

	for(u=g->nodes.buf, ue=u+g->nodes.len, x=0; u<ue; u++, x+=2)
		putnode(u, x, 0);
	return 0;
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
