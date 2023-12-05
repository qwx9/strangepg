#include "strpg.h"
#include "em.h"
#include "index.h"

int
pushsupernode(Graph *g, ssize idx, ssize old, ssize new, int weight)
{
	int ret;
	ssize i;
	khiter_t k;
	Node *n;
	Coarse *c;

	c = g->c;
	DPRINT(Debugcoarse, "pushsupernode: [%zx] %zx → %zx", idx, old, new);
	k = kh_get(toinode, c->inodes, new);
	if(k == kh_end(c->inodes)){	/* not instanciated */
		if((i = pushnode(g, idx)) < 0)
			return -1;
		k = kh_put(toinode, c->inodes, new, &ret);
		kh_val(c->inodes, k) = i;
	}else{
		i = kh_val(c->inodes, k);
		DPRINT(Debugcoarse, "pushsupernode: super[%zx] → %zx already exists at %zx", idx, new, i);
	}
	k = kh_put(toinode, c->inodes, old, &ret);	/* for unreferenced older nodes */
	kh_val(c->inodes, k) = i;
	k = kh_put(tosuper, c->supers, idx, &ret);
	kh_val(c->supers, k) = new;
	n = g->nodes + i;
	n->sid = new;
	n->weight = weight;
	return 0;
}

int
pushsuperedge(Graph *g, ssize u, ssize v, int θ1, int θ2)
{
	return pushedge(g, u, v, θ1, θ2);
}

/* FIXME: performance critical */
Node *
getinode(Graph *g, ssize s)
{
	ssize i;
	Coarse *c;
	khiter_t k;

	if(s < 0){
		werrstr("getinode: invalid id %zd", s);
		return nil;
	}
	c = g->c;
	if(c == nil){
		if(s >= dylen(g->nodes))
			return nil;
		return g->nodes + s;
	}
	k = kh_get(toinode, c->inodes, s);
	if(k == kh_end(c->inodes))
		return nil;
	i = kh_val(c->inodes, k);
	assert(i < dylen(g->nodes));
	return g->nodes + i;
}

ssize
getsupernode(Graph *g, ssize idx)
{
	ssize i;
	Coarse *c;
	khiter_t k;

	c = g->c;
	k = kh_get(tosuper, c->supers, idx);
	if(k == kh_end(c->supers))
		return -1;
	i = kh_val(c->supers, k);
	return i;
}

/* FIXME: expensive af */
Node *
gethypernode(Graph *g, ssize idx)	/* lol */
{
	ssize s;

	if((s = getsupernode(g, idx)) < 0)
		return nil;
	return getinode(g, s);
}

Edge *
getedge(Graph *g, ssize i)
{
	Edge *e;

	if(i >= dylen(g->edges))
		return nil;
	e = g->edges + i;
	if(e->u == Bupkis)	// FIXME: still?
		return nil;
	return e;
}

static int
subcoarsen(Graph *, usize)
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
	DPRINT(Debugcoarse, "coarsen level %d by %d", g->c->level, Δ);
	loadlevel(g, g->c->level, -Δ);
	return 0;
}

static int
uncoarsen(Graph *g, int Δ)
{
	DPRINT(Debugcoarse, "uncoarsen level %d by %d", g->c->level, Δ);
	loadlevel(g, g->c->level+1, Δ);
	return 0;
}

int
setgraphdepth(Graph *g, int z)
{
	int r, lvl;

	DPRINT(Debugcoarse, "set global graph to depth %d/%llud", z, dylen(g->c->levels));
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
	Coarse *c;

	c = g->c;
	if(c == nil)
		return 0;
	if(setgraphdepth(g, c->level + Δ) < 0){
		warn("setgraphdepth: %s\n", error());
		return -1;
	}
	return 0;
}

void
clearindex(Graph *g)
{
	Coarse *c;

	c = g->c;
	kh_clear(tosuper, c->supers);
	kh_clear(toinode, c->inodes);
}

void
nukeindex(Graph *g)
{
	Coarse *c;

	c = g->c;
	kh_destroy(tosuper, c->supers);
	kh_destroy(toinode, c->inodes);
	free(c);
}

Coarse *
initindex(Graph *g)
{
	Coarse *c;

	c = emalloc(sizeof *c);
	g->c = c;
	c->level = -1;
	c->supers = kh_init(tosuper);
	c->inodes = kh_init(toinode);
	return c;
}
