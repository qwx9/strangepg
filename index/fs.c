#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

// FIXME: pack into/unpack from specific supernode
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

/* FIXME: yuck, can we avoid (in a simple way) to destroy the graph and
 * then recreate it despite obvious redundancies? wrt partial updates,
 * we need to be able to only clear the relevant parts of the graph */

void
loadlevel(Graph *g, int z, int Δ)
{
	int s, j;
	usize idx, new, weight, u, v, i;
	Coarse *c;
	Level *l;
	File *f;

	// FIXME: reversed direction

	f = g->f;
	c = g->c;
	DPRINT(Debugcoarse, "loadlevel %#p cur %d z %d Δ %d\n",
		g, c->level, z, Δ);
	cleargraph(g);
	dyclear(g->edges);
	c->level = z + Δ - 1;
	l = c->levels + c->level;
	seekfs(f, l->noff);
	for(i=0; i<l->nnodes; i++){
		u = get64(f);
		idx = get64(f);
		new = get64(f);
		weight = get64(f);
		if(pushsupernode(g, idx, new, u, weight) < 0){
			warn("loadlevel %d → %d failed: %s\n", c->level-z, c->level, error());
			return;
		}
	}
	s = Δ < 0 ? -1 : 1;
	//l -= s * (Δ - 1);
	Δ *= s;
	for(j=0; j<Δ; j++, l+=s){
		assert(l >= c->levels);
		seekfs(f, l->eoff);
		for(i=0; i<l->nedges; i++){
			// FIXME
			if(s < 0){
				dypop(g->edges);
				continue;
			}
			u = get64(f);
			v = get64(f);
			pushsuperedge(g, u, v, Edgesense, Edgesense);
		}
	}
	printgraph(g);
}
