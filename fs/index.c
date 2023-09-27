#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* FIXME: there's a problem showing inverted edge, perhaps even sd's */

extern char *indexpath;	/* FIXME */

static int
readindex(Graph *g, char *path)
{
	File *f;
	Coarse *c;

	dprint(Debugcoarse, "readindex %s\n", path);
	if((c = initindex(g)) == nil)
		sysfatal("load: failed to set index up: %r");
	f = c->i.f = emalloc(sizeof *f);	// FIXME: this sucks, wrap the alloc
	if(openfs(f, path, OREAD) < 0)
		return -1;
	g->nnodes = get64(f);
	g->nedges = get64(f);
	dprint(Debugcoarse, "readindex nn=%zd ne=%zd\n", g->nnodes, g->nedges);
	c->inodes = emcreate(g->nnodes * 8);
	c->iedges = emcreate(g->nedges * 8);
	dprint(Debugcoarse, "readindex done\n");
	return 0;
}

/* the actual level index is small and can remain in memory */
static int
readtree(Graph *g, char *path)
{
	usize nn, ne, nl;
	File *f;
	Level *l;
	Ctree *ct;

	dprint(Debugcoarse, "readtree %s\n", path);
	f = g->f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		return -1;
	nn = get64(f);	// nv + ns
	ne = get64(f);
	nl = g->nlevels = get64(f);
	dprint(Debugcoarse, "ct: nv+ns %zd ne %zd nl %zd; index: nv %zd ne %zd\n", nn, ne, nl, g->nnodes, g->nedges);
	ct = &g->c->t;
	dyprealloc(ct->levels, nl);
	/* FIXME: refactor later; all we need in-memory are one offset and
	 * indices into the array; EM would just mirror that once commited to
	 * disk in whatever format... see also: dfc */
	for(l=ct->levels; l<ct->levels+dylen(ct->levels); l++){
		l->noff = get64(f);
		l->eoff = get64(f);
		l->nnodes = get64(f);
		l->nedges = get64(f);
		dprint(Debugcoarse, "level %zd off %zd %zd len %zd %zd\n", l-ct->levels, l->noff, l->eoff, l->nnodes, l->nedges);
	}
	/* file remains open */
	dprint(Debugcoarse, "readtree done\n");
	return 0;
}

static Graph *
load(char *treepath)
{
	Graph *g;

	if((g = initgraph()) == nil)
		sysfatal("load: %r");
	if(indexpath == nil){
		werrstr("index/load: missing index file");
		return nil;
	}
	if(readindex(g, indexpath) < 0)
		sysfatal("load: failed to read index %s: %r", indexpath);
	if(readtree(g, treepath) < 0)
		sysfatal("load: failed to read tree %s: %r", treepath);
	if(setgraphdepth(g, 0) < 0)
		sysfatal("load: failed to load first level: %r");
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
	nukeindex(g);
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
