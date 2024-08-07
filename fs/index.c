#include "strpg.h"
#include "fs.h"
#include "graph.h"
#include "threads.h"
#include "em.h"
#include "index.h"

void
expandnode(Graph *g, Node *pp)
{
	/*
	ioff i, idx, u, v, par, w;
	Level *l;
	Coarse *c;
	File *f;
	Node *p, *n, *m;

	f = g->f;
	if((c = g->c) == nil){
		warn("expandnode: nothing to expand");
		return;
	}
	l = c->levels + pp->lvl;
	if(pp->lvl >= dylen(c->levels))
		return;
	printgraph(g);
	DPRINT(Debugcoarse, "split %#p node %zx level %d idx %zx", g, pp->id, pp->lvl, pp->idx);
	warn("pp %#p %zx\n", pp, pp->id);
	seekfs(f, l->noff);
	for(i=0, p=nil; i<l->nnodes; i++){
		u = eget64(f);
		idx = eget64(f);
		par = eget64(f);
		w = eget64(f);
		warn("pp %#p %zx\n", pp, pp->id);
		DPRINT(Debugcoarse, "check %zx %zx %zx against %zx", u, idx, par, pp->id);
		// FIXME: better way?
		if(par != pp->id){
			touchnode(g, u, par, idx, w);
			continue;
		}
		DPRINT(Debugcoarse, "expanding node [%zx]%zx←%zx par=%zx", idx, u, par, pp->id);
		p = p == nil ? pushchild(g, u, pp, idx, w) : pushsibling(g, u, p, idx, w);
	}
	seekfs(f, l->eoff);
	warn("pp %#p %zx\n", pp, pp->id);
	for(i=0; i<l->nedges; i++){
		u = eget64(f);
		v = eget64(f);
		eget64(f);	// ei
		DPRINT(Debugcoarse, "expanding edge %c%zx,%c%zx", (u&1)?'-':'+',u>>1, (v&1)?'-':'+',v>>1);
		n = getnode(g, u >> 1);
		m = getnode(g, v >> 1);
		warn("pp %#p %zx\n", pp, pp->id);
		assert(n != nil && m != nil);
		if(n->pid != pp->id && m->pid != pp->id){
			DPRINT(Debugcoarse, "skip foreign child (%zx,%zx) not %zx's", n->pid, m->pid, pp->id);
			continue;
		}
		n = getactivenode(g, n);
		m = getactivenode(g, m);
		pushiedge(g, n, m, u&1, v&1);
	}
	printgraph(g);
	*/
}

void
retractnode(Graph *g, Node *pp)
{
	/*
	DPRINT(Debugcoarse, "merge %#p node %zx level %d", g, pp->id, pp->lvl);
	if(g->c == nil){
		warn("expandnode: nothing to retract");
		return;
	}
	poptree(g, pp);
	printgraph(g);
	*/
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
	/*
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
	dylen(g->nodes) = eget64(f);
	dylen(g->edges) = eget64(f);
	g->nsuper = eget64(f);
	nl = g->nlevels = eget64(f);
	DPRINT(Debugcoarse, "ct: nv %zd nv+ns %zd ne %zd nl %d",
		dylen(g->nodes), g->nsuper, dylen(g->edges), g->nlevels);
	g->c = emalloc(sizeof *c);
	c = g->c;
	dyprealloc(c->levels, nl);
	for(l=c->levels; l<c->levels+dylen(c->levels); l++){
		l->nnodes = eget64(f);
		l->nedges = eget64(f);
		l->noff = eget64(f);
		l->eoff = eget64(f);
		DPRINT(Debugcoarse, "level %zd off %zd %zd len %zd %zd",
			l-c->levels, l->noff, l->eoff, l->nnodes, l->nedges);
	}
	// file remains open
	DPRINT(Debugcoarse, "readtree done");
	return 0;
	*/
	return -1;
}

static void
load(void *arg)
{
	/*
	char *path;
	Graph g;

	path = arg;
	g = initgraph(FFindex);
	if(readtree(&g, path) < 0)
		sysfatal("load: failed to read tree %s: %s", path, error());
	free(path);
	// FIXME: don't rely on this number, there isn't always just one anyway
	// instead look at the top level and make parents
	pushinode(&g, g.nsuper, -1, g.nsuper, 1);
	expandnode(&g, g.nodes);
	pushgraph(g);
	*/
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
