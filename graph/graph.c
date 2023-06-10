#include "strpg.h"

Graph *graphs;
int ngraphs;

/* not actually editing anything, but this is not transparent, unless
 * it provides an iterator, otherwise we'll always have to check the
 * value of .erased */
void
removenode(Graph *, Node *n)
{
	n->erased = 1;
}
void
removeedge(Graph *g, usize i)
{
	Edge *e;

	// FIXME: this must "remove" the edge from both ends, make sure,
	// though this should be enough
	e = vecp(&g->edges, i);
	e->erased = 1;
}

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

/* don't know about this... */
Node
newnode(void)
{
	Node n = {0};

	n.id = -1;
	n.seq = -1;
	n.vrect = ZQ;
	n.q1 = ZQ;
	n.q2 = ZQ;
	return n;
}
Edge
newedge(void)
{
	Edge e = {0};

	e.overlap = -1;
	return e;
}

int
addnode(Graph *g, char *id, char *)
{
	usize i;
	Node n;

	dprint("addnode %s %#p (vec sz %zd elsz %d)\n", id, (uchar *)g->nodes.buf + g->nodes.len-1, g->nodes.len, g->nodes.elsz);
	if(id2n(g, id) != nil){
		werrstr("duplicate node id");
		return 0;
	}
	n = newnode();
	n.in = vec(sizeof(usize), 0);
	n.out = vec(sizeof(usize), 0);
	n.w = 1.0;
	n.parent = -1;
	veccopy(&g->nodes, &n, &i);
	return idput(g->id2n, estrdup(id), i);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, char *from, char *to, int d1, int d2, char *overlap, double w)
{
	usize i;
	Edge e;
	Node *u, *v;

	// FIXME: check for duplicate/redundancy? (vec → set)
	dprint("addedge %s,%s:%.2f len=%zd %#p (vec sz %zd elsz %d)\n", from, to,
		w, g->edges.len, (uchar *)g->edges.buf + g->edges.len-1,
		g->edges.len, g->edges.elsz);
	USED(overlap);
	e = newedge();
	e.w = w;
	if((u = id2n(g, from)) == nil
	|| (v = id2n(g, to)) == nil)
		return -1;
	e.from = (u - (Node *)g->nodes.buf) << 1 | d1;
	e.to =  (v - (Node *)g->nodes.buf) << 1 | d2;
	veccopy(&g->edges, &e, &i);
	veccopy(&u->out, &i, nil);
	veccopy(&v->in, &i, nil);
	return 0;
}

void
nukegraph(Graph *g)
{
	vecnuke(&g->edges);
	vecnuke(&g->nodes);
	memset(g, 0, sizeof *g);
}

Graph*
initgraph(void)
{
	Graph *g;

	graphs = erealloc(graphs,
		(ngraphs+1) * sizeof *graphs, ngraphs * sizeof *graphs);
	g = graphs + ngraphs++;
	g->nodes = vec(sizeof(Node), 0);
	g->edges = vec(sizeof(Edge), 0);
	g->levels = vec(sizeof(Level), 0);
	g->level = 0;
	g->id2n = idmap();
	g->layout.tid = -1;
	return g;
}
