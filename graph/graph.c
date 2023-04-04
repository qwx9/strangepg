#include "strpg.h"

Graph *graph;

void
addnode(Graph *g, usize label)
{
	int ret;
	Node n;

	n.lab = label;
	kv_init(n.in);
	kv_init(n.out);
	kv_push(Node, g->nodes, n);
	warn("addnode %#p lab %zd\n", &kv_A(g->nodes, kv_size(g->nodes)-1), label);
}

void
addedge(Graph *g, usize id, usize u, usize v, double w)
{
	int ret;
	Edge e, *ep;
	Node *n;

	ret = 0;
	USED(ret);	// FIXME: error checks
	USED(id);	// FIXME: edges and nodes have a name, not just id

	e.w = w;
	if(w == 0.0)
		warn("edge %llud has 0 weight", id);
	kv_push(Edge, g->edges, e);
	ep = &kv_A(g->edges, kv_size(g->edges) - 1);

	warn("addedge %#p %zd,%zd", ep, u, v);

	assert(u < kv_size(g->nodes));
	n = &kv_A(g->nodes, u);
	kv_push(Edge*, n->out, ep);
	ep->u = n;
	assert(v < kv_size(g->nodes));
	n = &kv_A(g->nodes, v);
	kv_push(Edge*, n->in, ep);
	ep->v = n;
	warn("u %#p v %#p\n", ep->u, ep->v);
}

Graph *
initgraph(Graph *h)
{
	Graph *g;

	if(h != nil)
		g = h;
	else
		g = emalloc(sizeof *g);
	kv_init(g->nodes);
	kv_init(g->edges);
	return g;
}
