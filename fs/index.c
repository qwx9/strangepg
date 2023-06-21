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

	dprint(Debugcoarse, "fulltrotsky node %#p %zd\n", lev, lev-g->nodes);
	/* castrate */
	for(ip=lev->out, ie=ip+dylen(lev->out); ip<ie; ip++){
		dprint(Debugcoarse, "fulltrotsky edge %zd\n", *ip);
		removeedge(g, *ip >> 1);	// FIXME: why shifted?
	}
	/* excommunicate */
	for(ip=lev->in, ie=ip+dylen(lev->in); ip<ie; ip++){
		dprint(Debugcoarse, "fulltrotsky edge %zd\n", *ip);
		removeedge(g, *ip >> 1);
	}
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
	get64(f);	// invec
	get64(f);	// outvec
	n.w = get64(f);
	parent = get64(f);
	if(parent >= 0){
		assert(g->nodes != nil);
		fulltrotsky(g->nodes+parent, g);
	}
	dypush(g->nodes, n);
	g->len++;
	return g->nodes + dylen(g->nodes) - 1;
}
/* don't try to be clever about reclaiming edges for now,
 * just delete old, add new */
static Edge *
readedge(File *f, Graph *g)
{
	usize i;
	Edge e;
	Node *up, *vp, *ne;

	e = newedge();
	e.from = get64(f);
	e.to = get64(f);
	e.w = get64(f);
	e.parent = get64(f);
	dypush(g->edges, e);
	up = g->nodes + (e.from >> 1);
	vp = g->nodes + (e.to >> 1);
	ne = g->nodes + dylen(g->nodes);
	assert(up < ne && vp < ne);
	i = dylen(g->edges) - 1;
	dypush(up->out, i);
	dypush(vp->in, i);
	return g->edges + i;
}

static int
loadlevel(Graph *g, int lvl)
{
	int i;
	File *f;
	Edge *e;
	Level *l, *le;

	dprint(Debugcoarse, "loading level %d from %s\n", lvl, g->infile->path);
	if(lvl < 0 || lvl >= dylen(g->levels)){
		werrstr("no such level %d", lvl);
		return -1;
	}
	if(g->level > 0 && g->level == lvl){
		werrstr("loadlevel: not reloading same zoom level");
		return -1;
	}
	f = g->infile;

	l = g->levels + g->level;
	le = g->levels + lvl;
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
		if(g->nodes[i].erased)
			continue;
		Node *n = g->nodes + i;
		dprint(Debugcoarse, "n %p in %zd out %zd w %.1f par %d erased %d\n", n, dylen(n->in), dylen(n->out), n->w, n->parent, n->erased);
	}
	for(i=0; i<dylen(g->edges); i++){
		if(g->edges[i].erased)
			continue;
		e = g->edges + i;
		dprint(Debugcoarse, "e %p w %.1f t %zd (%zd) h %zd (%zd)\n", e, e->w,
			e->from >> 1, e->from & 1, e->to >>1, e->to & 1);
	}
	g->level = lvl;
	if(g->layout.ll != nil)
		return updatelayout(g);
	return 0;
}
/* the actual dictionary alone does not take much memory */
static Graph *
loaddicts(char *path)
{
	int i;
	File *f;
	Graph *g;

	dprint(Debugcoarse, "loadindex %s\n", path);
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
	for(i=0; i<g->nlevels; i++){
		Level l = {0};
		l.noff = g->noff + get64(f);
		l.nnel = get64(f);
		l.ntot = get64(f);
		l.eoff = g->eoff + get64(f);
		l.enel = get64(f);
		l.etot = get64(f);
		dypush(g->levels, l);
	}
	/* file remains open */
	dprint(Debugcoarse, "done loading\n");
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
