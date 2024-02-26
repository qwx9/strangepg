#include "strpg.h"
#include "layout.h"

/* pushnamednodes() already spawns nodes with random coordinates, so
 * no need to do anything */

static void
init(Graph *g)
{
	ssize i;
	Node *u;

	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
	}
}

static void
compute(Graph *)
{
}

static Layout ll = {
	.name = "random",
	.init = init,
	.compute = compute,
};

Layout *
regrandom(void)
{
	return &ll;
}
