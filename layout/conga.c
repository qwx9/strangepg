#include "strpg.h"
#include "layoutprv.h"

/* stupidest layout: 1D, nodes preordered by id */
/* nodes and edges are duplicated, but realize that everything else is
 * supposed to in fine unravel/unload while we go */

static Layer
compute(Graph *g)
{
	usize i;
	Layer l;
	Node *v, *ve;
	Edge *e, *ee;

	kv_init(l.nodes);
	kv_init(l.edges);
	for(v=&kv_A(g->nodes, 0), ve=v+kv_size(g->nodes), i=0; v<ve; v++, i++)
		putnode(&l, i, i, 0, 1);
	for(e=&kv_A(g->edges, 0), ee=e+kv_size(g->edges), i=0; e<ee; e++, i++)
		putedge(&l, i, e->u, 0, e->v, 0, e->w);
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
