#include "strpg.h"
#include "layoutprv.h"

/* stupidest layout, 1 dimension, nodes preordered by id */

/* FIXME: it's all factors of unit vectors */

/* FIXME: layout does not consider window size or whatever; display code will
 * translate the drawing to center it */
/* FIXME: layout here considers that nodes are single points, ie. there's a
 * distance of exactly 1 between two immediate neighbors */

static Layer
compute(Graph *g)
{
	usize i;
	Layer ll;
	Vertex v;

	kv_init(ll.nodes);
	for(v=ZV, i=0; i<kv_size(g->nodes); i++, v.x++)
		kv_A(ll.nodes, i) = v;
	return ll;
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
