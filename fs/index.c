#include "strpg.h"
#include "fs.h"

/* NOTE: in a directed graph, arcs are (tail,head) between two end points,
 * tail and head, rather than the reverse; an edge's head degree is its
 * left /from outfree */

/* FIXME: possibility: R trees or something to select which branch to
 * expand, instead of all of them; this would require storing things
 * again amenable for a dfs, but could  */

/* FIXME: updateable layout: add/remove edges from already computed forcefield layout */

static File meta;
static vlong metaoff;	/* FIXME: handle meta */

static int
loadlevel(Graph *g, int lvl)
{
	usize i;
	intptr Δl;
	Level *l, *lp;
	Node n, *up, *vp;
	Edge e;
	File *f;

	/* recheck rest of the code either to make it work the same way, or to
	 * do that here */
	lp = vecp(&g->levels, lvl);
	Δl = lp - g->lvl;
	// FIXME: free vecs and shit
	vecresize(&g->nodes, lp->ntot);
	vecresize(&g->edges, lp->etot);
	if(Δl <= 0)
		return 0;
	f = g->index;
	seekfs(f, lp->noff);
	for(l=g->lvl; l<=lp; l++){
		for(i=0; i<lp->nnel; i++){
			n.id = get64(f);
			n.seq = get64(f);
			/* fuck it; this is hardly any kind of bottleneck and
			 * hardly different than having node dict, offsets, lengths */
			n.out = vec(sizeof(usize), get32(f));
			n.in = vec(sizeof(usize), get32(f));
			n.w = getdbl(f);
			if(get32(f) < lvl)	/* expiration date */
				continue;
			vecpush(&g->nodes, &n, &i);
		}
	}
	seekfs(f, lp->eoff);
	for(l=g->lvl; l<=lp; l++){
		/* FIXME: redundancies with gfa */
		for(i=0; i<lp->enel; i++){
			e.overlap = get64(f);
			e.from = get64(f);
			e.to = get64(f);
			e.w = getdbl(f);
			if(get32(f) < lvl)	/* expiration date */
				continue;
			vecpush(&g->edges, &e, &i);
			up = vecp(&g->nodes, e.from >> 1);
			vp = vecp(&g->nodes, e.to >> 1);
			vecpush(&up->out, &i, nil);
			vecpush(&vp->in, &i, nil);
		}
	}
	closefs(f);
	return 0;
}

static Graph *
loaddicts(char *path)
{
	File *f;
	Level *lp, *le;
	Graph *g;

	dprint("loadindex %s\n", path);
	if((g = initgraph()) == nil)
		sysfatal("loadindex: %r");
	f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		return nil;
	g->index = f;
	g->nnodes = get64(f);
	g->nedges = get64(f);
	g->nlevels = get64(f);
	g->levels = vec(sizeof(Level), g->nlevels);
	for(lp=g->levels.buf, le=lp+g->nlevels; lp<le; lp++){
		lp->noff = get64(f);
		lp->nnel = get64(f);
		lp->ntot = get64(f);
		lp->eoff = get64(f);
		lp->enel = get64(f);
		lp->etot = get64(f);
	}
	closefs(f);
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
