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
	vlong parent;
	Node n;

	n = newnode();
	n.out = vec(sizeof(usize), get64(f));
	n.in = vec(sizeof(usize), get64(f));
	n.w = get64(f);
	parent = get64(f);
	if(parent >= 0)
		fulltrotsky(g->nodes+parent, g);
	dypush(g->nodes, n);
	return g->nodes + dylen(g->nodes) - 1;
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
	up = g->nodes + (e.from >> 1);
	vp = g->nodes + (e.to >> 1);
	veccopy(&up->out, &i, nil);
	veccopy(&vp->in, &i, nil);
	return (Edge *)g->edges.buf + i;
}

static int
loadlevel(Graph *g, int lvl)
{
	int i;
	File *f;
	Edge *e;
	Level *l, *le;

	if(lvl < 0 || lvl >= g->levels.len){
		werrstr("no such level %d", lvl);
		return -1;
	}
	if(g->level > 0 && g->level == lvl){
		werrstr("loadlevel: not reloading same zoom level");
		return -1;
	}
	f = g->infile;

	l = vecp(&g->levels, g->level);
	le = vecp(&g->levels, lvl);
	if(l <= le)
		le++;
	else
		le--;

	/* zoom out: unwind descent */
	for(; l>le; l--){

		// FIXME

	}
	/* zoom in: descend tree
	 * current implementation probably doesn't make it safe to process all
	 * nodes then all edges in one go */
	for(; l<le; l++){
		seekfs(f, l->noff);
		for(i=0; i<l->nnel; i++)
			readnode(f, g);
		seekfs(f, l->eoff);
		for(i=0; i<l->enel; i++)
			readedge(f, g);
	}
	for(i=0; i<dylen(g->nodes); i++){
		Node *n = g->nodes + i;
		dprint("n %p in %zd out %zd w %.1f par %d erased %d\n", n, n->in.len, n->out.len, n->w, n->parent, n->erased);
	}
	for(i=0; i<g->edges.len; i++){
		e = vecp(&g->edges, i);
		dprint("e %p w %.1f t %zd (%zd) h %zd (%zd)\n", e, e->w,
			e->from >> 1, e->from & 1, e->to >>1, e->to & 1);
	}
	g->level = lvl;
	return updatelayout(g);
}
/* the actual dictionary alone does not take much memory */
static Graph *
loaddicts(char *path)
{
	int i;
	File *f;
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
	g->levels = vec(sizeof(Level), g->nlevels);
	for(i=0; i<g->nlevels; i++){
		Level l = {0};
		l.noff = g->noff + get64(f);
		l.nnel = get64(f);
		l.ntot = get64(f);
		l.eoff = g->eoff + get64(f);
		l.enel = get64(f);
		l.etot = get64(f);
		veccopy(&g->levels, &l, nil);
	}
	/* file remains open */
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
