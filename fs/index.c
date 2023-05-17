#include "strpg.h"
#include "fs.h"

/* file format:
 * header[] ldict[] nodes[] edges[] edgelinks[] meta[]
 * header: nnodes[8] nedges[8] nlevels[8]
 * ldict: { nodeoff[8] nnodes[8] nodetot[8] inoutoff[8]
 *          edgeoff[8] nedges[8] edgetot[8] }[nlevels]
 * nodes: { nin[8] nout[8] }[nnodes]
 * edges: { from[7] fromdir[1] to[7] todir[1] }[nedges]
 * edgelinks: { edge[8] }[∑nin+nout ∀nodes]
 *	same order as nodes
 * meta: classified.
 * assuming allows random access for nodes and edges since they are of
 * fixed size.
 */

/* FIXME: no input validation whatsoever; could be done on the go, or
 * at the end once all sizes are known */

static int
readedges(Graph *g, Level *lp, File *f)
{
	usize nn;
	Edge *e, *es, *ee;

	if(g->level >= lp)
		return 0;
	es = g->edges.tail;
	nn = lp->etot - g->level->etot;
	/* don't do any caching for now */
	vecresize(&g->edges, g->edges.len + nn);
	seekfs(f, lp->eoff);
	for(e=es, ee=e+nn; e<ee; e++){
		e->from = get64(f);
		e->to = get64(f);
	}
	return 0;
}

static int
readnodes(Graph *g, Level *lp, File *f)
{
	usize n, nn, *ep, *ee;
	Node *u, *us, *ue;

	if(g->level >= lp)
		return 0;
	us = g->nodes.tail;
	nn = lp->ntot - g->level->ntot;
	/* don't do any caching for now */
	vecresize(&g->nodes, g->nodes.len + nn);
	seekfs(f, lp->noff);
	for(u=us, ue=u+nn; u<ue; u++){
		if((n = get64(f)) > 0)
			u->in = vec(sizeof(usize), n);
		if((n = get64(f)) > 0)
			u->out = vec(sizeof(usize), n);
	}
	seekfs(f, lp->linkoff);
	for(u=us, ue=u+nn; u<ue; u++){
		for(ep=u->in.buf, ee=ep+u->in.len; ep<ee; ep++)
			*ep = get64(f);
		for(ep=u->out.buf, ee=ep+u->out.len; ep<ee; ep++)
			*ep = get64(f);
	}
	return 0;
}

/* indices in g->nodes/g->edges are the same as in the index file */
int
loadlevel(Graph *g, int nl)
{
	File f;
	Level *lp;

	if(g->index == nil){
		werrstr("no index");
		return -1;
	}
	assert(nl >= 0 && nl < g->levels.len);
	lp = vecp(&g->levels, nl);
	if(lp == g->level)
		return 0;
	if(openfs(&f, g->index, OREAD) < 0)
		return -1;
	if(readnodes(g, lp, &f) < 0)
		return -1;
	if(readedges(g, lp, &f) < 0)
		return -1;
	closefs(&f);
	g->level = lp;
	g->stale = 1;
	return 0;
}

/* FIXME: later: partitioning: subtrees based on
 * location/locality/topology (simple quadtrees?) */
/* FIXME: don't load anything besides the offset tables;
 * once loaded, don't free upper levels, just append,
 * unless we run out of memory */
/* FIXME: error checking */
/* FIXME: preprocess metadata as well, appended to index */
static Graph*
loaddicts(char *path)
{
	usize n;
	File f;
	Level *lp, *le;
	Graph *g;

	dprint("loadindex %s\n", path);
	if((g = initgraph()) == nil)
		sysfatal("loadindex: %r");
	memset(&f, 0, sizeof f);
	if(openfs(&f, path, OREAD) < 0)
		return nil;
	get64(&f);	// nnodes
	get64(&f);	// nedges
	n = get64(&f);
	vecresize(&g->levels, n);
	for(lp=g->levels.buf, le=lp+n; lp<le; lp++){
		lp->noff = get64(&f);
		lp->nlen = get64(&f);
		lp->ntot = get64(&f);
		lp->linkoff = get64(&f);
		lp->eoff = get64(&f);
		lp->elen = get64(&f);
		lp->etot = get64(&f);
	}
	closefs(&f);
	g->index = estrdup(path);
	return g;
}

static int
save(Graph *)
{
	/* FIXME: index.py here */
	return 0;
}

static Filefmt ff = {
	.name = "index",
	.load = loaddicts,
	.save = save,
};

Filefmt *
regindex(void)
{
	return &ff;
}
