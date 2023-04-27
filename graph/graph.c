#include "strpg.h"

Graph *graphs;
int ngraphs;

Node *
id2n(Graph *g, char *k)
{
	usize v;

	if(idget(g->id2n, k, &v) < 0)
		return nil;
	return vecp(&g->nodes, v);
}

/* n is an unshifted packed node with orientation */
Node *
e2n(Graph *g, usize n)
{
	n >>= 1;
	assert(n < g->nodes.len);
	return vecp(&g->nodes, n);
}

int
addnode(Graph *g, char *id, char *seq)
{
	usize i;
	Node n;

	dprint("addnode %s %#p (vec sz %zd elsz %d)\n", id, (uchar *)g->nodes.buf + g->nodes.len-1, g->nodes.len, g->nodes.elsz);
	if(id2n(g, id) != nil){
		werrstr("duplicate node id");
		return -1;
	}
	memset(&n, 0, sizeof n);
	n.id = estrdup(id);	// FIXME: necessary?
	n.seq = estrdup(seq);
	n.in = vec(0, sizeof(Edge*));
	n.out = vec(0, sizeof(Edge*));
	n.q = ZQ;
	vecpush(&g->nodes, &n, &i);
	return idput(g->id2n, n.id, i);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, char *from, char *to, int d1, int d2, char *overlap, double w)
{
	Edge e, *ep;
	Node *u, *v;

	// FIXME: check for duplicate/redundancy? (vec → set)
	dprint("addedge %s,%s:%.2f len=%zd %#p (vec sz %zd elsz %d)\n", from, to, w, g->edges.len, (uchar *)g->edges.buf + g->edges.len-1, g->edges.len, g->edges.elsz);
	e.overlap = estrdup(overlap);
	e.w = w;
	if((u = id2n(g, from)) == nil
	|| (v = id2n(g, to)) == nil)
		return -1;
	e.from = (u - (Node *)g->nodes.buf) << 1 | d1;
	e.to =  (v - (Node *)g->nodes.buf) << 1 | d2;
	// FIXME: the memcpy'ing in vec makes this really dangerous
	//	at least call it something other than push, it doesn't push
	ep = vecpush(&g->edges, &e, nil);
	vecpush(&u->out, &ep, nil);
	vecpush(&v->in, &ep, nil);
	return 0;
}

void
rendernew(void)
{
	Graph *g;

	for(g=graphs; g<graphs+ngraphs; g++)
		if(g->stale)
			triggerlayout(g);
}

void
nukegraph(Graph *g)
{
	vecnuke(&g->edges);
	vecnuke(&g->nodes);
	idnuke(g->id2n);
	memset(g, 0, sizeof *g);
}

Graph*
initgraph(void)
{
	Graph *g;

	graphs = erealloc(graphs,
		(ngraphs+1) * sizeof *graphs, ngraphs * sizeof *graphs);
	g = graphs + ngraphs++;
	g->nodes = vec(0, sizeof(Node));
	g->edges = vec(0, sizeof(Edge));
	g->id2n = idmap();
	g->stale = 1;
	return g;
}
