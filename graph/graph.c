#include "strpg.h"

Graph *graphs;
int ngraphs;

/* not actually editing anything, but this is not transparent, unless
 * it provides an iterator, otherwise we'll always have to check the
 * value of .erased */
void
removenode(Graph *g, Node *n)
{
	n->erased = 1;
	g->len--;
	memset(n, 0, sizeof *n);
}
void
removeedge(Graph *g, usize i)
{
	Edge *e;

	// FIXME: this must "remove" the edge from both ends, make sure,
	// though this should be enough
	e = g->edges + i;
	e->erased = 1;
}

Node *
id2n(Graph *g, char *k)
{
	usize v;

	if(idget(g->id2n, k, &v) < 0)
		return nil;
	assert(v < dylen(g->nodes));
	return g->nodes + v;
}

/* n is an unshifted packed node with orientation */
Node *
e2n(Graph *g, usize n)
{
	n >>= 1;
	assert(n < dylen(g->nodes));
	return g->nodes + n;
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
	Node n;

	dprint("addnode id=%s (vec sz %zd)\n", id, dylen(g->nodes));
	if(id2n(g, id) != nil){
		werrstr("duplicate node id");
		return 0;
	}
	n = newnode();
	n.w = 1.0;
	n.parent = -1;
	dypush(g->nodes, n);
	g->len++;
	return idput(g->id2n, estrdup(id), dylen(g->nodes)-1);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, char *from, char *to, int d1, int d2, char *overlap, double w)
{
	usize i;
	Edge e;
	Node *u, *v;

	// FIXME: check for duplicate/redundancy? (vec → set)
	dprint("addedge %s,%s:%.2f len=%zd %#p\n", from, to,
		w, dylen(g->edges), g->edges + dylen(g->edges)-1);
	USED(overlap);
	e = newedge();
	e.w = w;
	if((u = id2n(g, from)) == nil
	|| (v = id2n(g, to)) == nil)
		return -1;
	e.from = u - g->nodes << 1 | d1;
	e.to =  v - g->nodes << 1 | d2;
	dypush(g->edges, e);
	i = dylen(g->edges) - 1;
	dypush(u->out, i);
	dypush(v->in, i);
	return 0;
}

void
nukegraph(Graph *g)
{
	Node *n;

	dyfree(g->edges);
	for(n=g->nodes; n<g->nodes+dylen(g->nodes); n++){
		dyfree(n->in);
		dyfree(n->out);
	}
	dyfree(g->nodes);
	dyfree(g->levels);
	freefs(g->infile);
	free(g->layout.aux);
	if(g->id2n != nil)
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
	g->layout.tid = -1;
	return g;
}
