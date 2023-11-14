#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* FIXME: there's a problem showing inverted edge, perhaps even sd's */

/* the actual level index is small and can remain in memory */
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
	DPRINT(Debugcoarse, "ct: nv+ns %zd ne %zd nl %d",
		g->nnodes, g->nedges, g->nlevels);
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

static Graph *
load(char *path)
{
	Graph *g;

	if((g = initgraph()) == nil)
		sysfatal("load: %s", error());
	if(initindex(g) == nil)
		sysfatal("load: initindex: %s", error());
	if(readtree(g, path) < 0)
		sysfatal("load: failed to read tree %s: %s", path, error());
	if(setgraphdepth(g, 0) < 0)
		sysfatal("load: failed to load first level: %s", error());
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
