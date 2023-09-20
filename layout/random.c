#include "strpg.h"
#include "layout.h"

/* pushnamednodes() already spawns nodes with random coordinates, so
 * no need to do anything */

static void
compute(Graph *g)
{
	int x;
	Node *u, *ue;

	for(u=g->nodes, ue=u+dylen(g->nodes), x=0; u<ue; u++, x+=Nodesz+10*Ptsz)
		putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
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
