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

usize
nodeid(Graph *g, usize u)
{
	Node *n;

	n = getithnode(g, u);
	return n->id;
}

usize
packedid(Graph *g, usize u)
{
	return nodeid(g, u >> 1);
}

void
printgraph(Graph *g)
{
	Edge *e, *ee;
	Node *n, *ne;
	usize *np, *nq;

	if(debug == 0)
		return;
	warn("graph %#p nn %zd ne %zd\n", g, g->nnodes, g->nedges);
	for(e=g->edges, ee=e+dylen(g->edges); e<ee; e++)
		warn("e%08zux %zux[=%zux] → %zux[=%zux]\n",
			e - g->edges, packedid(g, e->u), e->u,
			packedid(g, e->v), e->v);
	for(n=g->nodes, ne=n+dylen(g->nodes); n<ne; n++){
		warn("n%08zux → %zux weight %d out [",
			n - g->nodes, n->id, n->weight);
		for(np=n->out, nq=np+dylen(n->out); np<nq; np++)
			warn("%s%zx", np==n->out?"":" ", *np);
		warn("] in [");
		for(np=n->in, nq=np+dylen(n->in); np<nq; np++)
			warn("%s%zx", np==n->in?"":" ", *np);
		warn("]\n");
	}
}

// FIXME: subgraphs: not always the last node/edge, so we need
// to be able to add/remove at any index
void
popnode(Graph *, Node *)
{
	// FIXME
}

void
popedge(Graph *, Edge *)
{
	// FIXME
}

static Node
newnode(void)
{
	Node u;

	memset(&u, 0, sizeof u);
	u.weight = 1;
	return u;
}

static Edge
newedge(void)
{
	return (Edge){0, 0};
}

usize
pushpackededge(Graph *g, usize pu, usize pv, usize ei)
{
	usize i;
	Edge e;
	Node *n;

	e = newedge();
	e.u = pu;
	e.v = pv;
	dypush(g->edges, e);
	i = dylen(g->edges) - 1;
	dprint(Debugcoarse, "pushpackededge %zux at %zux: %zux,%zux:", ei, i, pu, pv);
	n = getithnode(g, pu >> 1);
	dprint(Debugcoarse, " in i=%zux id=%zux", n->id, n - g->nodes);
	dypush(n->out, ei);
	n = getithnode(g, pv >> 1);
	dprint(Debugcoarse, " out i=%zux id=%zux\n", n->id, n - g->nodes);
	dypush(n->in, ei);
	return i;
}

/* id's in edges are always packed with direction bit */
int
pushnamededge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	usize u, v;
	Node *n;

	// FIXME: check for duplicate/redundancy? (vec → set)
	dprint(Debugtheworld, "pushnamededge %s,%s (index %zd)\n", eu, ev,
		dylen(g->edges));
	if((n = id2n(g, eu)) == nil)
		return -1;
	u = n - g->nodes << 1 | d1;
	if((n = id2n(g, ev)) == nil)
		return -1;
	v = n - g->nodes << 1 | d2;
	pushpackededge(g, u, v, dylen(g->edges));
	return 0;
}

usize
pushnode(Graph *g, usize u, int w)
{
	Node n;

	dprint(Debugcoarse, "pushnode %zd\n", u);
	n = newnode();
	n.id = u;
	n.weight = w;
	dypush(g->nodes, n);
	return dylen(g->nodes) - 1;
}

int
pushnamednode(Graph *g, char *id)
{
	dprint(Debugtheworld, "pushname id=%s\n", id);
	if(id2n(g, id) != nil){
		werrstr("duplicate node id");
		return 0;
	}
	pushnode(g, dylen(g->nodes), 1);
	return idput(g->id2n, estrdup(id), dylen(g->nodes)-1);
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
	freefs(g->f);
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
