#include "strpg.h"

Graph *graphs;

Node *
id2n(Graph *g, char *k)
{
	usize u;

	if(idget(g->id2n, k, &u) < 0)
		return nil;
	assert(u < dylen(g->nodes));
	return g->nodes + u;
}

/* u is an unshifted packed node with orientation */
Node *
e2n(Graph *g, usize u)
{
	u >>= 1;
	assert(u < dylen(g->nodes));
	return g->nodes + u;
}

static Node
newnode(void)
{
	Node u = {0};

	u.metaoff = -1;
	u.weight = 1.0;
	u.vrect = ZQ;	/* FIXME: ugh */
	u.q1 = ZQ;
	u.q2 = ZQ;
	return u;
}

static Edge
newedge(void)
{
	Edge e = {0};

	e.metaoff = -1;
	return e;
}

int
addnode(Graph *g, char *id)
{
	Node u;

	dprint(Debugtheworld, "addnode id=%s (index %zd)\n", id, dylen(g->nodes));
	if(id2n(g, id) != nil){
		werrstr("duplicate node id");
		return 0;
	}
	u = newnode();
	u.realid = dylen(g->nodes);
	dypush(g->nodes, u);
	return idput(g->id2n, estrdup(id), dylen(g->nodes)-1);
}

/* id's in edges are always packed with direction bit */
int
addedge(Graph *g, char *from, char *to, int d1, int d2)
{
	usize i;
	Edge e;
	Node *u, *v;

	// FIXME: check for duplicate/redundancy? (vec → set)
	dprint(Debugtheworld, "addedge %s,%s (index %zd)\n", from, to, dylen(g->edges));
	e = newedge();
	if((u = id2n(g, from)) == nil
	|| (v = id2n(g, to)) == nil)
		return -1;
	e.u = u - g->nodes << 1 | d1;
	e.v = v - g->nodes << 1 | d2;
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
	Graph g = {0};

	g.layout.tid = -1;
	dypush(graphs, g);
	return graphs;
}
