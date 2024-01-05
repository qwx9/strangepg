#include "strpg.h"
#include "fs.h"
#include "em.h"

typedef struct Level Level;

struct Level{
	vlong noff;
	usize eoff;
	usize nnodes;
	usize nedges;
};
struct Coarse{
	Level *levels;
};

// FIXME: this is a fucked way to encode a tree for this purpose
void
expandnode(Graph *g, ssize id)
{
	ssize i, u, v, par, w;
	Level *l;
	Coarse *c;
	File *f;
	Node *pp, *p, *n, *m;

	f = g->f;
	c = g->c;
	printgraph(g);
	if((pp = getnode(g, id)) == nil)
		abort();	// FIXME
	stoplayout(g);
	DPRINT(Debugcoarse, "split %#p node %zx level %d", g, id, pp->lvl);
	l = c->levels + pp->lvl;
	seekfs(f, l->noff);
	for(i=0, p=nil; i<l->nnodes; i++){
		u = get64(f);
		get64(f);	/* idx */
		par = get64(f);	/* par */
		w = get64(f);	/* weight */
		if(par != id)	// FIXME: performance
			continue;
		p = p == nil ? pushchild(g, u, pp, w) : pushsibling(g, u, p, w);
	}
	seekfs(f, l->eoff);
	for(i=0; i<l->nedges; i++){
		u = get64(f);
		v = get64(f);
		if((n = getnode(g, u)) == nil || (m = getnode(g, v)) == nil){
			warn("expandnode: unsupported edge %zx,%zx\n", u, v);
			continue;
		}
		if(n->pid == id || m->pid == id)
			pushedge(g, n, m, Edgesense, Edgesense);
	}
	updatelayout(g);
	printgraph(g);
}

void
retractnode(Graph *g, ssize id)
{
	Node *pp;

	if((pp = getnode(g, id)) == nil)
		abort();	// FIXME
	DPRINT(Debugcoarse, "merge %#p node %zx level %d", g, id, pp->lvl);
	poptree(g, pp);
	printgraph(g);
}

int
zoomgraph(Graph *g, int Δ)
{
	USED(g, Δ);
	DPRINT(Debugcoarse, "zoomgraph %#p %d", g, Δ);
	return 0;
}

static int
readtree(Graph *g, char *path)
{
	usize nl;
	File *f;
	Level *l;
	Coarse *c;

	if(path == nil){
		werrstr("no input file");
		return -1;
	}
	DPRINT(Debugcoarse, "readtree %s", path);
	f = g->f = emalloc(sizeof *f);	// FIXME: this sucks, wrap the alloc
	if(openfs(f, path, OREAD) < 0)
		return -1;
	g->nnodes = get64(f);
	g->nedges = get64(f);
	g->nsuper = get64(f);
	nl = g->nlevels = get64(f);
	DPRINT(Debugcoarse, "ct: nv %zd nv+ns %zd ne %zd nl %d",
		g->nnodes, g->nsuper, g->nedges, g->nlevels);
	c = g->c;
	dyprealloc(c->levels, nl);
	for(l=c->levels; l<c->levels+dylen(c->levels); l++){
		l->nnodes = get64(f);
		l->nedges = get64(f);
		l->noff = get64(f);
		l->eoff = get64(f);
		DPRINT(Debugcoarse, "level %zd off %zd %zd len %zd %zd",
			l-c->levels, l->noff, l->eoff, l->nnodes, l->nedges);
	}
	/* file remains open */
	DPRINT(Debugcoarse, "readtree done");
	return 0;
}

static void
loadlevel(Graph *g, int lvl)
{
	usize par, w, u, v, i;
	Coarse *c;
	Level *l;
	File *f;
	Node *n, *m;

	f = g->f;
	c = g->c;
	DPRINT(Debugcoarse, "loadlevel %#p %d", g, lvl);
	l = c->levels + lvl;
	seekfs(f, l->noff);
	for(i=0, n=nil; i<l->nnodes; i++){
		u = get64(f);
		get64(f);	/* idx */
		par = get64(f);
		w = get64(f);
		n = n != nil ? pushsibling(g, u, n, w) : pushnode(g, u, par, w);
	}
	seekfs(f, l->eoff);
	for(i=0; i<l->nedges; i++){
		u = get64(f);
		v = get64(f);
		n = getnode(g, u);
		m = getnode(g, v);
		assert(n != nil && m != nil);
		pushedge(g, n, m, Edgesense, Edgesense);
	}
}

static Graph *
load(char *path)
{
	Graph *g;
	Coarse *c;

	if((g = initgraph()) == nil)
		sysfatal("loadgfa1: %r");
	g->c = emalloc(sizeof *c);	// FIXME
	if(readtree(g, path) < 0)
		sysfatal("load: failed to read tree %s: %s", path, error());
	pushnode(g, g->nsuper, -1, 1);
	expandnode(g, g->nsuper);
	//loadlevel(g, 0);
	return g;
}

static int
save(Graph *)
{
	return 0;
}

static void
nuke(Graph *g)
{
	free(g->c);
	nukegraph(g);
}

static Filefmt ff = {
	.name = "index",
	.load = load,
	.save = save,
	.nuke = nuke,
};

Filefmt *
regindex(void)
{
	return &ff;
}
