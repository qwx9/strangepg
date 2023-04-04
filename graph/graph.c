#include "strpg.h"

Graph *graph;

void
addnode(Graph *g, char *label)
{
	Node n;

	dprint("addnode %s %#p (vec sz %zd elsz %d)\n", label, (uchar *)g->nodes.buf + g->nodes.len-1, g->nodes.len, g->nodes.elsz);
	n.label = estrdup(label);
	n.in = vec(0, sizeof(Edge*));
	n.out = vec(0, sizeof(Edge*));
	vecpush(&g->nodes, &n);
}

void
addedge(Graph *g, char *label, usize u, usize v, double w)
{
	Edge e, *ep;

	e.label = estrdup(label);
	e.w = w;
	if(w == 0.0)
		warn("edge %s has 0 weight", label);
	dprint("addedge %s %zd,%zd ", label, u, v);
	e.u = vecget(&g->nodes, u);
	e.v = vecget(&g->nodes, v);
	ep = vecpush(&g->edges, &e);
	dprint("%#p %#p,%#p\n", ep, e.u, e.v);
	vecpush(&e.u->out, &ep);
	vecpush(&e.v->in, &ep);
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
