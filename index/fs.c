#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

Node *
getithnode(Graph *g, usize u)
{
	usize v;

	if((v = emget64(g->c->inodes, 8*u)) == 0)
		return nil;
	return g->nodes + v-1;
}

Edge *
getithedge(Graph *g, usize e)	/* edges[iedge[e]] */
{
	usize v;

	if((v = emget64(g->c->iedges, 8*e)) == 0)
		return nil;
	return g->edges + v-1;
}

Edge
getedgedef(Graph *g, usize e)	/* edgelist[i] */
{
	File *f;
	Edge ed;

	f = g->c->i.f;
	seekfs(f, 2*8 + e*2*8);
	// FIXME: no orientation + layer violation?
	ed.u = get64(f) << 1 | 0;
	ed.v = get64(f) << 1 | 0;
	dprint(Debugcoarse, "getedgedef %zx: read %zx,%zx\n", e, ed.u>>1, ed.v>>1);
	return ed;
}

static void
remaporpush(Graph *g, usize u, usize new, int w)
{
	usize v;
	Node *n;

	n = getithnode(g, u);
	if(n != nil){
		dprint(Debugcoarse, "remap u=%zux: %zux → %zux\n", u, n-g->nodes, new);
		n->id = new;
		n->weight = w;
		return;
	}
	v = pushnode(g, new, w);
	emput64(g->c->inodes, 8*u, v+1);
}

int
unloadbranch(Graph *, Level *, usize)
{
	// FIXME
	return -1;
}

static int
loadbranch(Graph *, Level *, usize)
{
	// FIXME
	return -1;
}

void
unloadlevels(Graph *, int , int)
{
	// FIXME

	/*
	...
	remaporpush(g, u, s, w);
	if(s´ == s)
		popnode(g, g->nodes[inode[u]]);
	// FIXME: for now, just keep a inode[]→Node array, fuck it
	*/
}

void
loadlevels(Graph *g, int z, int Δ)
{
	int nn, ne, w;
	usize u, u0, s, e, i;
	Ctree *ct;
	Level *l;
	File *ft;
	Edge ed;

	ft = g->f;
	ct = &g->c->t;
	nn = ne = 0;
	for(l=ct->levels+z; Δ>0; Δ--, l++){
		nn += l->nnodes;
		ne += l->nedges;
		dprint(Debugcoarse, "loadlevels: add level %zd: nn %d ne %d\n",
			l - ct->levels, nn, ne);
	}
	seekfs(ft, ct->levels[z].noff);
	while(nn-- > 0){
		u = get64(ft);
		u0 = get64(ft);
		s = get64(ft);
		w = get64(ft);
		dprint(Debugcoarse, "loadnode u=%zux old=%zux new=%zux w=%d\n",
			u, u0, s, w);
		remaporpush(g, u, u0, w);
	}
	seekfs(ft, ct->levels[z].eoff);
	while(ne-- > 0){
		e = get64(ft);
		ed = getedgedef(g, e);
		dprint(Debugcoarse, "loadedge e %zux u=%zux v=%zux\n",
			e, ed.u>>1, ed.v>>1);
		i = pushpackededge(g, ed.u, ed.v, e);
		emput64(g->c->iedges, 8*e, i+1);
	}
	g->c->level = z + Δ;
	printgraph(g);
}
