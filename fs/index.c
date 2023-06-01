#include "strpg.h"
#include "fs.h"

/* FIXME: possibility: R trees or something to select which branch to
 * expand, instead of all of them; this would require storing things
 * again amenable for a dfs, but could  */

/* FIXME: there's a problem showing inverted edge, perhaps even sd's */

static File meta;
static vlong metaoff;	/* FIXME: handle meta */

/* FIXME: counterpart to fulltrotsky: undeath for supernode and related edges
static void
reanimate(..)
{

}
*/

static void
fulltrotsky(Node *lev, Graph *g)
{
	usize *ip, *ie;

	/* castrate */
	for(ip=lev->out.buf, ie=ip+lev->out.len; ip<ie; ip++)
		removeedge(g, *ip >> 1);
	/* excommunicate */
	for(ip=lev->in.buf, ie=ip+lev->in.len; ip<ie; ip++)
		removeedge(g, *ip >> 1);
	/* erase */
	lev->erased = 1;
}

/* nodes and edges are arranged such that these fields are valid and correct
 * indices with correct orientation; no  idmap is needed */
static Node *
readnode(File *f, Graph *g)
{
	usize i;
	Node n, *parent;

	n = newnode();
	n.out = vec(sizeof(usize), get64(f));
	n.in = vec(sizeof(usize), get64(f));
	n.w = getdbl(f);
	parent = vecp(g->nodes.buf, get64(f));
	fulltrotsky(parent, g);
	veccopy(&g->nodes, &n, &i);
	return (Node *)g->nodes.buf + i;
}
/* don't try to be clever about reclaiming edges for now,
 * just delete old, add new */
static Edge *
readedge(File *f, Graph *g)
{
	usize i;
	Edge e;
	Node *up, *vp;

	e = newedge();
	e.from = get64(f);
	e.to = get64(f);
	e.w = get64(f);
	e.parent = get64(f);
	veccopy(&g->edges, &e, &i);
	up = vecp(&g->nodes, e.from >> 1);
	vp = vecp(&g->nodes, e.to >> 1);
	veccopy(&up->out, &i, nil);
	veccopy(&vp->in, &i, nil);
	return (Edge *)g->edges.buf + i;
}

static int
loadlevel(Graph *g, int lvl)
{
	int i;
	File *f;
	Level *l, *lp;

	fprint(2, "%zd %zd %zd\n", g->nnodes, g->nedges, g->nlevels);
	fprint(2, "loadlevel %d %zd\n", lvl, g->levels.len);
	if(lvl > g->levels.len){
		werrstr("no such level %d", lvl);
		return -1;
	}
	if(lvl == g->level){
		werrstr("loadlevel: not reloading same zoom level");
		return -1;
	}
	/* recheck rest of the code either to make it work the same way, or to
	 * do that here */
	lp = vecp(&g->levels, lvl);
	fprint(2, "%d lp %zd %zd %zd %zd\n", lvl, lp->nnel, lp->ntot, lp->enel, lp->etot);

	l = (Level *)g->levels.buf + lvl;

	f = g->infile;

	/* zoom out: unwind descent */
	for(; l>lp; l--){

		// FIXME

	}
	/* zoom in: descend tree
	 * current implementation probably doesn't make it safe to process all
	 * nodes then all edges in one go */
	for(; l<lp; l++){
		seekfs(f, l->noff);
		for(i=0; i<l->nnel; i++)
			readnode(f, g);
		seekfs(f, l->eoff);
		for(i=0; i<l->enel; i++)
			readedge(f, g);
	}
	for(i=0; i<g->nodes.len; i++)
		fprint(2, "n %p\n", vecp(&g->nodes, i));
	for(i=0; i<g->edges.len; i++)
		fprint(2, "e %p\n", vecp(&g->edges, i));
	closefs(f);
	g->stale = 1;
	return 0;
}
/* the actual dictionary alone does not take much memory */
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
	g->infile = f;
	g->nnodes = get64(f);
	g->nedges = get64(f);
	g->nlevels = get64(f);
	g->doff = get64(f);	/* FIXME: names, fields */
	g->noff = get64(f);
	g->eoff = get64(f);
	g->moff = get64(f);
	fprint(2, "%zd %zd %zd\n", g->nnodes, g->nedges, g->nlevels);
	g->levels = vec(sizeof(Level), g->nlevels);
	for(lp=g->levels.buf, le=lp+g->nlevels; lp<le; lp++){
		lp->noff = g->noff + get64(f);
		lp->nnel = get64(f);
		lp->ntot = get64(f);
		lp->eoff = g->eoff + get64(f);
		lp->enel = get64(f);
		lp->etot = get64(f);
		fprint(2, "level %zd %zx %zx %zx %zx %zx %zx\n",
			lp - (Level*)g->levels.buf, lp->noff, lp->nnel, lp->ntot,
			lp->eoff, lp->enel, lp->etot);
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
	.chlev = loadlevel,
	.save = save,
};

Filefmt *
regindex(void)
{
	return &ff;
}
