#include "strpg.h"

static Node *
lab2n(Graph *g, usize labid)
{
	usize id;
	Node *n;

	id = kh_get(usize, g->nid, labid);
	n = &kv_A(g->nodes, id);
	return n;
}

void
addnode(Graph *g, usize id)
{
	Node n;

	n.id = id;
	(n.in.n = n.in.m = 0,
		n.in.a = 0);
	kv_init(n.in);
	kv_init(n.out);
	kv_push(Node, g->nodes, n);
}

void
addedge(Graph *g, usize id, usize from, usize to, double w)
{
	int ret;	// FIXME: error checks
	usize eid;
	khiter_t k;
	Edge e;
	Node *n;

	e.from = from;
	e.to = to;
	e.w = w;
	kv_push(Edge, g->edges, e);
	eid = kv_size(g->edges) - 1;
	k = kh_put(usize, g->eid, id, &ret);
	kh_value(g->eid, k) = eid;
	n = lab2n(g, from);
	kv_push(usize, n->out, eid);
	n = lab2n(g, to);
	kv_push(usize, n->in, eid);
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
	g->eid = kh_init(usize);
	g->nid = kh_init(usize);
	return g;
}
