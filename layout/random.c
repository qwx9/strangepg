#include "strpg.h"
#include "layout.h"

/* addnodes() already spawns nodes with random coordinates, so
 * no need to do anything */

static int
compute(Graph *g)
{
	int x;
	Node *u, *ue;

	for(u=g->nodes.buf, ue=u+g->nodes.len, x=0; u<ue; u++, x+=Nodesz+10*Ptsz)
		putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
	return 0;
}

static Layout ll = {
	.name = "random",
	.compute = compute,
};

Layout *
regrandom(void)
{
	return &ll;
}
