#include "strpg.h"

Graph *graphs;
int ngraphs;

Node *
id2n(Graph *g, usize i)
{
	usize v;

	dprint("id2n %#p k=%zd ", g, i);
	if(idget(g->id2n, i, &v) < 0){
		dprint("→ nope\n");
		return nil;
	}
	dprint("→ %zd\n", v);
	return vecp(&g->nodes, v);
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
	n.q.u.x = nrand(view.dim.v.x);
	n.q.u.y = nrand(view.dim.v.y);
	vecpush(&g->nodes, &n, &i);
	return idput(g->id2n, id, i);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, usize u, usize v, char *overlap, double w)
{
	Edge e, *ep;
	Node *up, *vp;

	dprint("addedge %zd,%zd:%.2f len=%zd %#p (vec sz %zd elsz %d)\n", u, v, w, g->edges.len, (uchar *)g->edges.buf + g->edges.len-1, g->edges.len, g->edges.elsz);
	/* FIXME: this api is RETARDED */
	e.overlap = estrdup(overlap);
	e.w = w;
	e.u = u;
	e.v = v;
	ep = vecpush(&g->edges, &e, nil);
	// FIXME: better print
//	dprint("addedge %s%zd → %s%zd: %s",
//		NDIRS(u), NID(u), NDIRS(v), NID(v), overlap);
	if((up = id2n(g, u >> 1)) == nil
	|| (vp = id2n(g, v >> 1)) == nil)
		return -1;
	vecpush(&up->out, &ep, nil);
	vecpush(&vp->in, &ep, nil);
	return 0;
}

void
rendernew(void)
{
	Graph *g;

	for(g=graphs; g<graphs+ngraphs; g++){
		if(g->ll != nil)	// FIXME: weak check
			continue;
		dprint("rendernew %#p\n", g);
		if(dolayout(&graphs[0], deflayout) < 0)
			sysfatal("dolayout: %s\n", error());
		if(render(&graphs[0]) < 0)
			sysfatal("render: %s\n", error());
	}
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
	srand(time(nil));
	return g;
}
