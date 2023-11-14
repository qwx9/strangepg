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

void
unloadlevels(Graph *g, int z, int Δ)
{
	int i, j, weight;
	usize u, old, new;
	Coarse *c;
	Level *l;
	File *f;

	f = g->f;
	c = g->c;
	l = c->levels + z;
	for(j=0; j<Δ; j++, l--, c->level--){
		seekfs(f, l->noff);
		for(i=0; i<l->nnodes; i++){
			u = get64(f);
			old = get64(f);
			new = get64(f);
			weight = get64(f);
			remapnode(g, u, old, new, weight);
		}
		c->level--;
		// FIXME: smarter given reuse
		for(i=0, j=dylen(g->nodes)-l->nnodes; i<j; i++)
			popnode(g, dylen(g->nodes) - 1);
	}
	c->level = z;	/* FIXME: external functions may assume depth */
	for(j=0; j<Δ; j++, l--, c->level--)
		for(i=0, j=dylen(g->edges)-l->nedges; i<j; i++)
			popedge(g, dylen(g->edges) - 1);
	printgraph(g);
}

// FIXME: don't do dynamic arrays for level data, it's constant
void
loadlevels(Graph *g, int z, int Δ)
{
	usize old, new, weight, u, v, i, j;
	Coarse *c;
	Level *l;
	File *f;

	f = g->f;
	c = g->c;
	DPRINT(Debugcoarse, "loadlevels %#p cur %d z %d Δ %d\n",
		g, c->level, z, Δ);
	l = c->levels + z;	/* z is current + 1 */
	c->level = z - 1;
	for(j=0; j<Δ; j++, l++, c->level++){
		seekfs(f, l->noff);
		for(i=0; i<l->nnodes; i++){
			u = get64(f);
			old = get64(f);
			new = get64(f);
			weight = get64(f);
			remapnode(g, u, new, old, weight);
		}
	}
	l = c->levels + z;
	c->level = z - 1;	/* external functions may assume depth */
	seekfs(f, l->eoff);
	for(j=0; j<Δ; j++, l++, c->level++){
		seekfs(f, l->eoff);
		for(i=0; i<l->nedges; i++){
			u = get64(f);
			v = get64(f);
			pushedge(g, u, v, Edgesense, Edgesense);
		}
	}
	printgraph(g);
}
