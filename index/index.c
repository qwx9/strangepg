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
	dprint(Debugcoarse, "coarsen level %d by %d\n", g->c->level, Δ);
	unloadlevels(g, g->c->level, Δ);
	printgraph(g);
	return 0;
}

static int
uncoarsen(Graph *g, int Δ)
{
	dprint(Debugcoarse, "uncoarsen level %d by %d\n", g->c->level, Δ);
	loadlevels(g, g->c->level+1, Δ);
	return 0;
}

/* FIXME: don't think these would work
void
advancetonode(Graph *g)
{
	INode *n;

	n = g->ctree.t.nodelist + g->ctree.lastnode++;
	pushnode(g, n->u, n->s, n->w);
}

void
rollbacknode(Graph *g)
{
	INode *n;

	n = g->ctree.t.nodelist + g->ctree.lastnode--;
	pushnode(g, n->u, n->s, n->w);
}
*/

int
setgraphdepth(Graph *g, int z)
{
	int lvl;

	dprint(Debugcoarse, "set global graph to depth %d\n", z);
	if(z < 0 || z >= dylen(g->c->t.levels)){
		werrstr("invalid level %d", z);
		return -1;
	}
	lvl = g->c->level;
	stoplayout(g);
	if(z > lvl)
		uncoarsen(g, z - lvl);
	else if(z < lvl)
		coarsen(g, lvl - z);
	else{
		werrstr("not reloading same zoom level");
		return -1;
	}
	// FIXME: make sure there's no layer violation here
	if(g->layout.ll != nil)
		return resetlayout(g);
	return 0;
}

int
zoomgraph(Graph *g, int Δ)
{
	return setgraphdepth(g, g->c->level + Δ);
}

void
nukeindex(Graph *g)
{
	Coarse *c;

	c = g->c;
	freefs(c->i.f);
	emclose(c->inodes);
	emclose(c->iedges);
	free(c);
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
