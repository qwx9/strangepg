#include "strpg.h"
#include "em.h"
#include "index.h"

Graph *graphs;

ssize
id2n(Graph *g, char *k)
{
	usize u;

	if(idget(g->id2n, k, &u) < 0)
		return -1;
	assert(u < dylen(g->nodes));
	return u;
}

ssize
nodeid(Graph *g, usize u)
{
	Node *n;

	if((n = getnode(g, u)) == nil)
		return -1;
	return n->eid;
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
		warn("e[%04zux] %zx[=%zx] → %zx[=%zx]\n",
			e - g->edges, packedid(g, e->u), e->u,
			packedid(g, e->v), e->v);
	for(n=g->nodes, ne=n+dylen(g->nodes); n<ne; n++){
		warn("n[%04zux] %zx weight %d → [ ",
			n - g->nodes, n->eid, n->weight);
		for(np=n->out, nq=np+dylen(n->out); np<nq; np++)
			warn("%zx ", *np);
		warn("] ← [ ");
		for(np=n->in, nq=np+dylen(n->in); np<nq; np++)
			warn("%zx ", *np);
		warn("]\n");
	}
}

/* preserve extant indices */
// FIXME: api: pass pointer?
void
popnode(Graph *g, ssize i)
{
	Node *n;

	if(i < 0 || i >= dylen(g->nodes))
		return;
	n = g->nodes + i;
	popinode(g, n);
}
void
popedge(Graph *g, ssize i)
{
	Edge *e;

	if((e = getedge(g, i)) == nil)
		return;
	e->u = Bupkis;
}

static Node
newnode(void)
{
	Node u = {0};

	memset(&u, 0, sizeof u);
	u.weight = 1;
	return u;
}

static Edge
newedge(void)
{
	return (Edge){0, 0};
}

ssize
pushedge(Graph *g, usize pu, usize pv, int udir, int vdir)
{
	ssize i;
	Edge e;
	Node *n;

	DPRINT(Debugcoarse, "pushedge %zx,%zx", pu, pv);
	e = newedge();
	e.u = pu << 1 | udir;
	e.v = pv << 1 | vdir;
	i = dylen(g->edges);
	dypush(g->edges, e);
	n = getnode(g, pu);
	assert(n != nil);
	dypush(n->out, i);
	n = getnode(g, pv);
	assert(n != nil);
	dypush(n->in, i);
	return i;
}

/* id's in edges are always packed with direction bit */
ssize
pushnamededge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	ssize u, v;

	// FIXME: check for duplicate/redundancy? (vec → set)
	if((u = id2n(g, eu)) < 0)
		return -1;
	if((v = id2n(g, ev)) < 0)
		return -1;
	return pushedge(g, u, v, d1, d2);
}

ssize
pushnode(Graph *g, usize idx)
{
	Node n;

	DPRINT(Debugcoarse, "pushnode %zx", idx);
	n = newnode();
	return pushinode(g, idx, &n);
}

int
pushnamednode(Graph *g, char *id)
{
	ssize i;

	if(id2n(g, id) >= 0){
		werrstr("duplicate node id");
		return 0;
	}
	i = pushnode(g, dylen(g->nodes));
	return idput(g->id2n, estrdup(id), i);
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
	dydelete(graphs, g-graphs);
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
