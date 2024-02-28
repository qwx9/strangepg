#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

void
expandnode(Graph *g, Node *pp)
{
	ssize i, idx, u, v, par, w;
	Level *l;
	Coarse *c;
	File *f;
	Node *p, *n, *m;

	f = g->f;
	c = g->c;
	l = c->levels + pp->lvl;
	if(pp->lvl >= dylen(c->levels))
		return;
	printgraph(g);
	DPRINT(Debugcoarse, "split %#p node %zx level %d idx %zx", g, pp->id, pp->lvl, pp->idx);
	seekfs(f, l->noff);
	for(i=0, p=nil; i<l->nnodes; i++){
		u = get64(f);
		idx = get64(f);
		par = get64(f);
		w = get64(f);
		DPRINT(Debugcoarse, "expanding node [%zx]%zx←%zx", idx, u, par);
		// FIXME: better way?
		if(par != pp->id){
			touchnode(g, u, par, idx, w);
			continue;
		}
		p = p == nil ? pushchild(g, u, pp, idx, w) : pushsibling(g, u, p, idx, w);
	}
	seekfs(f, l->eoff);
	for(i=0; i<l->nedges; i++){
		u = get64(f);
		v = get64(f);
		get64(f);	/* ei */
		DPRINT(Debugcoarse, "expanding edge %c%zx,%c%zx", (u&1)?'-':'+',u>>1, (v&1)?'-':'+',v>>1);
		n = getnode(g, u >> 1);
		m = getnode(g, v >> 1);
		assert(n != nil && m != nil);
		if(n->pid != pp->id && m->pid != pp->id)
			continue;
		n = getactivenode(g, n);
		m = getactivenode(g, m);
		pushedge(g, n, m, u&1, v&1);
	}
	printgraph(g);
}

void
retractnode(Graph *g, Node *pp)
{
	DPRINT(Debugcoarse, "merge %#p node %zx level %d", g, pp->id, pp->lvl);
	poptree(g, pp);
	printgraph(g);
}

int
zoomgraph(Graph *g, int Δ)
{
	USED((g, Δ));
	DPRINT(Debugcoarse, "zoomgraph %#p %d", g, Δ);
	return 0;
}

int
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
	g->c = emalloc(sizeof *c);
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
load(void *path)
{
	Graph g;

	g = initgraph(FFindex);
	if(readtree(&g, path) < 0)
		sysfatal("load: failed to read tree %s: %s", path, error());
	// FIXME: don't rely on this number, there isn't always just one anyway
	// instead look at the top level and make parents
	pushnode(&g, g.nsuper, -1, g.nsuper, 1);
	expandnode(&g, g.nodes);
	pushgraph(g);
	threadexits(nil);
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
