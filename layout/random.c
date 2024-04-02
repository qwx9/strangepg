#include "strpg.h"
#include "layout.h"

static void
init(Graph *)
{
}

static void
compute(Graph *g)
{
	ssize i;
	Node *u;
	Layouting *l;

	l = &g->layout;
	if((l->f & LFarmed) == 0){
		for(i=g->node0.next; i>=0; i=u->next){
			u = g->nodes + i;
			putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
		}
		l->f |= LFarmed;
	}
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
