#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

Node *
getnode(Graph *g, usize i)
{
	assert(i < dylen(g->nodes));
	return g->nodes + i;
}

Edge *
getedge(Graph *g, usize i)
{
	assert(i < dylen(g->edges));
	return g->edges + i;
}

int
unloadbranch(Graph *, Level *, usize)
{
	// FIXME
	return -1;
}

static int
loadbranch(Graph *, Level *, usize)
{
	// FIXME
	return -1;
}

void
unloadlevels(Graph *g, int z, int Δ)
{
	int i;
	usize par, w, s0;
	Node *n, *n0;
	Coarse *c;
	Level *l;
	File *ft;

	ft = g->f;
	c = g->c;
	l = c->levels + z;
	n0 = z > 1 ? g->nodes + l[-1].firstnode : g->nodes;
	s0 = n0->id;
	seekfs(ft, l->noff);
	for(i=0; i<l->nnodes; i++){
		get64(ft);			/* u */
		par = get64(ft);
		w = get64(ft);
		n = n0 + par - s0;
		if(n->id >= s0)
			n->weight += w;
		else
			n->id = par;
		/* no need to update inodes */
	}
	for(i=0; i<l->nnodes; i++)
		dypop(g->nodes);
	/* no need to update iedges */
	g->c->level -= Δ;
	printgraph(g);
}

void
loadlevels(Graph *g, int z, int Δ)
{
	int w;
	usize u, v, par, i, j, x;
	Coarse *c;
	Level *l;
	File *ft;

	ft = g->f;
	c = g->c;
	l = c->levels + z;	/* note that z is current + 1 */
	g->c->level = z - 1;
	seekfs(ft, l->noff);
	for(j=0; j<Δ; j++, l++){
		g->c->level++;
		dprint(Debugcoarse, "loadlevels: level %zd: add %zd nodes",
			l - c->levels, l->nnodes);
		l->firstnode = dylen(g->nodes);
		for(i=0; i<l->nnodes; i++){
			u = get64(ft);
			par = get64(ft);
			w = get64(ft);
			dprint(Debugcoarse, "loadlevels old=%zux new=%zux w=%d",
				u, par, w);
			x = u - l->firstnode;
			pushnodeat(g, u, w, x);
			dprint(Debugcoarse, "%s node[%zx] ← %zx\n",
				x<dylen(g->nodes) ? "remap" : "push", x, u);
		}
	}
	l = c->levels + z;
	g->c->level = z - 1;
	seekfs(ft, l->eoff);
	for(j=0; j<Δ; j++, l++){
		g->c->level++;
		dprint(Debugcoarse, "loadlevels: level %zd: add %zd edges",
			l - c->levels, l->nedges);
		l->firstedge = dylen(g->edges);
		for(i=0; i<l->nedges; i++){
			u = get64(ft);
			v = get64(ft);
			dprint(Debugcoarse, "loadlevels edge %zux,%zux", u>>1, v>>1);
			i = pushpackededge(g, u, v);
		}
	}
	g->c->level = z - 1 + Δ;
	printgraph(g);
}
