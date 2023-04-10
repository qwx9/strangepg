#include "strpg.h"

Graph *graphs;
int ngraphs;

Node *
id2n(Graph *g, usize i)
{
	if(idget(g->id2n, i, &i) < 0)
		return nil;
	return vecp(&g->nodes, i);
}

int
addnode(Graph *g, usize id, char *seq)
{
	usize i;
	Node n;

	dprint("addnode %zd %#p (vec sz %zd elsz %d)\n", id, (uchar *)g->nodes.buf + g->nodes.len-1, g->nodes.len, g->nodes.elsz);
	memset(&n, 0, sizeof n);
	n.id = id;
	n.seq = estrdup(seq);
	n.in = vec(0, sizeof(Edge*));
	n.out = vec(0, sizeof(Edge*));
	vecpush(&g->nodes, &n, &i);
	return idput(g->id2n, id, i);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, usize u, usize v, char *overlap, double w)
{
	Edge e, *ep;
	Node *up, *vp;

	/* FIXME: this api is RETARDED */
	e.overlap = estrdup(overlap);
	e.w = w;
	e.u = u;
	e.v = v;
	ep = vecpush(&g->edges, &e, nil);
	// FIXME: better print
//	dprint("addedge %s%zd → %s%zd: %s",
//		NDIRS(u), NID(u), NDIRS(v), NID(v), overlap);
	if((up = id2n(g, u)) == nil
	|| (vp = id2n(g, v)) == nil)
		return -1;
	vecpush(&up->out, &ep, nil);
	vecpush(&vp->in, &ep, nil);
	return 0;
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

	ngraphs++;
	graphs = erealloc(graphs,
		(ngraphs+1) * sizeof *graphs, ngraphs * sizeof *graphs);
	g = graphs + ngraphs++;
	g->nodes = vec(0, sizeof(Node));
	g->edges = vec(0, sizeof(Edge));
	g->id2n = idmap();
	return g;
}
