#include "strpg.h"
#include "em.h"
#include "index.h"

static int
subcoarsen(Graph *, usize )
{
	return -1;
}

static int
subuncoarsen(Graph *, usize)
{
	return -1;
}

static int
contractnode(Graph *, usize)
{
	return -1;
}

static int
expandnode(Graph *, usize)
{
	return -1;
}

static int
coarsen(Graph *g, int Δ)
{
	dprint(Debugcoarse, "coarsen level %d by %d", g->c->level, Δ);
	unloadlevels(g, g->c->level, Δ);
	return 0;
}

static int
uncoarsen(Graph *g, int Δ)
{
	dprint(Debugcoarse, "uncoarsen level %d by %d", g->c->level, Δ);
	loadlevels(g, g->c->level+1, Δ);
	return 0;
}

int
setgraphdepth(Graph *g, int z)
{
	int r, lvl;

	dprint(Debugcoarse, "set global graph to depth %d/%llud", z, dylen(g->c->levels));
	if(z < 0 || z >= dylen(g->c->levels)){
		werrstr("invalid level %d", z);
		return -1;
	}
	lvl = g->c->level;
	stoplayout(g);
	if(z > lvl)
		r = uncoarsen(g, z - lvl);
	else if(z < lvl)
		r = coarsen(g, lvl - z);
	else{
		werrstr("not reloading same zoom level");
		return -1;
	}
	if(r < 0)
		warn("setgraphdepth: %s\n", error());
	assert(g->c->level == z);
	// FIXME: make sure there's no layer violation here
	if(g->layout.ll != nil)
		return resetlayout(g);
	return 0;
}

int
zoomgraph(Graph *g, int Δ)
{
	if(setgraphdepth(g, g->c->level + Δ) < 0){
		warn("setgraphdepth: %s\n", error());
		return -1;
	}
	return 0;
}

Coarse *
initindex(Graph *g)
{
	Coarse *c;

	c = emalloc(sizeof *c);
	g->c = c;
	c->level = -1;
	return c;
}
