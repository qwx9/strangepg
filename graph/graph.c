#include "strpg.h"

Graph *graph;

static Node *
lab2node(Graph *g, usize label)
{
	usize i;
	Node *n;

	i = kh_get(usize, g->lab2node, label);
	n = &kv_A(g->nodes, i);
	return n;
}

// FIXME: label is a string
void
addnode(Graph *g, usize label)
{
	int ret;
	Node n;
	khiter_t k;

	n.lab = label;
	kv_init(n.in);
	kv_init(n.out);
	kv_push(Node, g->nodes, n);
	k = kh_put(usize, g->lab2node, label, &ret);
	kh_value(g->lab2node, k) = kv_size(g->nodes);
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

	// FIXME: more useful to store a Node*; but be mindful that
	// we'll end up using external memory for accesses
	// in other words, the footprint should be minimal
	// and the "index" can be big, who cares (to start with)
	//assert(kh_exist(g->lab2node, u));
	//n = lab2node(g, u);
	assert(u < kv_size(g->nodes));
	n = &kv_A(g->nodes, u);
	//assert(kh_exist(g->lab2node, v));
	kv_push(Edge*, n->out, ep);
	ep->u = n;
	//assert(kh_exist(g->lab2node, v));
	//n = lab2node(g, v);
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
	g->lab2node = kh_init(usize);
	return g;
}
