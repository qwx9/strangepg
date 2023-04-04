#include "strpg.h"

Graph *graph;

void
addnode(Graph *g, char *label)
{
	Node n;

	warn("addnode %#p lab %s sz %zd elsz %zd\n", (uchar *)g->nodes.buf + g->nodes.len-1, label, g->nodes.len, g->nodes.elsz);
	n.label = estrdup(label);
	n.in = vec(0, sizeof(Edge*));
	n.out = vec(0, sizeof(Edge*));
	vecpush(&g->nodes, &n);
	warn("addnode %#p lab %s\n", (uchar *)g->nodes.buf + g->nodes.len-1, label);
}

void
addedge(Graph *g, char *label, usize u, usize v, double w)
{
	Edge e, *ep;
	Node *n;

	e.label = estrdup(label);
	e.w = w;
	if(w == 0.0)
		warn("edge %s has 0 weight", label);
	warn("addedge %#p %zd,%zd", ep, u, v);
	e.u = vecget(&g->nodes, u);
	e.v = vecget(&g->nodes, v);
	ep = vecpush(&g->edges, &e);
	vecpush(&e.u->out, &ep);
	vecpush(&e.v->in, &ep);
	warn("u %#p v %#p\n", e.u, e.v);
}

Graph *
initgraph(Graph *h)
{
	Graph *g;

	// FIXME: don't alloc on heap either
	if(h != nil)
		g = h;
	else
		g = emalloc(sizeof *g);
	g->nodes = vec(0, sizeof(Node));
	g->edges = vec(0, sizeof(Edge));
	return g;
}
