#include "strpg.h"
#include "layoutprv.h"

/* stupidest layout: 1D, nodes preordered by id */
/* nodes and edges are duplicated, but realize that everything else is
 * supposed to in fine unravel/unload while we go */

static Layer
compute(Graph *g)
{
	int x;
	usize i;
	Layer l;
	Node *v, *ve;

	kv_init(l.nodes);
	for(v=&kv_A(g->nodes, 0), ve=v+kv_size(g->nodes), i=0, x=0; v<ve; v++, i++, x+=2)
		putnode(&l, i << 1, x, 0);
	return l;
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
