#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	Minedges = 0,
	Lrecsz = 4 * sizeof(u64int),
	Nrecsz = 3 * sizeof(u64int),
	Erecsz = 2 * sizeof(u64int),
};
typedef struct Lbuf Lbuf;
struct Lbuf{
	int nnodes;
	int nedges;
	EM *nodes;
	EM *edges;
};
static usize nsuper;

static int
reverse(Graph *g, Lbuf *lvl)
{
	ssize noff, eoff;
	Lbuf *lp, *le;
	EM *em;

	/* FIXME: for a user, failing at this point is... disappointing. */
	if((em = emfdopen(1, 1)) == nil)
		sysfatal("emfdopen: %s\n", error());
	dprint(Debugcoarse, "reverse: %zd %zd %zd %zd",
		g->nnodes, g->nedges, nsuper, dylen(lvl));
	empput64(em, 0, g->nnodes);
	// FIXME: output number of superedges as well? useful? ← yes.
	emput64(em, g->nedges);
	emput64(em, nsuper);
	emput64(em, dylen(lvl));
	noff = 4*8;
	noff += dylen(lvl) * Lrecsz;
	eoff = noff + nsuper * Nrecsz;
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		emput64(em, lp->nnodes);
		emput64(em, lp->nedges);
		emput64(em, noff);
		emput64(em, eoff);
		noff += lp->nnodes * Nrecsz;
		eoff += lp->nedges * Erecsz;
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		dprint(Debugcoarse, "[%zd] %d nodes", lp-lvl, lp->nnodes);
		embraceextendextinguish(em, lp->nodes);
		emclose(lp->nodes);
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		dprint(Debugcoarse, "[%zd] %d edges", lp-lvl, lp->nedges);
		embraceextendextinguish(em, lp->edges);
		emclose(lp->edges);
	}
	emclose(em);
	return 0;
}

static void
printtab(EM *em, ssize ne)
{
	int i;
	ssize u, v;

	if((debug & Debugcoarse) == 0)
		return;
	for(i=0; i<ne; i++){
		u = empget64(em, 2+i*2);
		v = emget64(em);
		dprint(Debugcoarse, "E[%d] %zx,%zx", i, u, v);
	}
}

static void
endlevel(Lbuf *lp)
{
	nsuper += lp->nnodes;
}

static void
outputedge(Lbuf *lp, ssize u, ssize v)
{
	EM *em;

	dprint(Debugcoarse, "drop edge %llx,%llx", u, v);
	em = lp->edges;
	emput64(em, u);
	emput64(em, v);
	lp->nedges++;
}

static void
outputnode(Lbuf *lp, ssize old, ssize new, ssize weight)
{
	EM *em;

	dprint(Debugcoarse, "merge %llx → %llx, weight %lld", old, new, weight);
	em = lp->nodes;
	emput64(em, old);
	emput64(em, new);
	emput64(em, weight);
	lp->nnodes++;
}

static Lbuf *
newlevel(Lbuf *lvl)
{
	Lbuf l = {0};

	l.nodes = emnew(0);
	l.edges = emnew(0);
	dypush(lvl, l);
	dprint(Debugcoarse, "-- newlevel %lld", dylen(lvl));
	return lvl;
}

static Lbuf *
coarsen(Graph *g, char *index)
{
	ssize i, o, w, u, v, s, t, y, m, e, uw, vw, M, S;
	EM *fedge, *fnode, *fweight, *fweight2;
	Lbuf *lvl, *lp;

	if((fedge = emclone(index)) == nil)
		sysfatal("emclone: %s", error());
	g->nnodes = empget64(fedge, 0);
	g->nedges = emget64(fedge);
	dprint(Debugcoarse, "graph %s nnodes %lld nedges %lld",
		index, g->nnodes, g->nedges);
	fweight = emnew(0);
	fweight2 = emnew(0);
	fnode = emnew(0);
	M = g->nedges;
	S = g->nnodes - 1;
	lvl = nil;
	w = S;
	y = 0;
	i = 0;
	uw = -1;	/* cannot happen */
	s = -1;	/* cannot happen */
	// FIXME: immediate fatal exit with no error if below threshold at start
	while(M > Minedges){
		printtab(fedge, M);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		u = -1;
		m = M;
		M = 0;
		int nein = 0;
		for(e=0; e<m; e++){
			if(e > 0 && e % 1000 == 0)
				dprint(Debugcoarse, "L%02zd edge %lld/%lld\n", dylen(lvl), e, g->nedges);
			/* new left node */
			if((o = empget64(fedge, 2+e*2)) != u){
				assert(o != EMbupkis);
				u = o;
				if((s = empget64(fnode, u-y)) == EMbupkis || s == 0 && u != 0)
					s = u;	/* default value */
				dprint(Debugcoarse, "[%04llx] new left node %llx → %llx", e, u, s);
				i = s - 1 - w;
				/* buffer contents are > w or undefined */
				if(s <= w){
					dprint(Debugcoarse, "[%04llx] unvisited left node: %llx ← %llx",
						e, u, S+1);
					s = ++S;
					i = s - 1 - w;
					if((uw = empget64(fweight, i)) == EMbupkis)
						uw = 1;	/* default value */
					outputnode(lp, u, s, uw);
					empput64(fnode, u-y, s);
					empput64(fweight2, i, uw);
					nein = 0;
				}else
					nein = 1;
			}
			v = empget64(fedge, 2+e*2+1);
			assert(v != EMbupkis);
			if((t = empget64(fnode, v-y)) == EMbupkis || t == 0 && v != 0)
				t = v;	/* default value */
			dprint(Debugcoarse, "[%04llx] edge %llx→%llx,%llx→%llx", e, u, s, v, t);
			if(t <= w){
				dprint(Debugcoarse, "[%04llx] unvisited right node: %llx ← %llx",
					e, v, s);
				if(nein){
					dprint(Debugcoarse, "not connecting unmerged right node for now");
					continue;
				}
				if((vw = empget64(fweight, i)) == EMbupkis)
					vw = 1;	/* default value */
				outputnode(lp, v, u, vw);
				uw += vw;
				empput64(fnode, v-y, s);
				empput64(fweight2, i, uw);
				outputedge(lp, u, v);
			}else if(v == u){
				dprint(Debugcoarse, "[%04llx] self edge: %llx,%llx", e, u, s);
				outputedge(lp, u, u);
			}else if(t == s)
				dprint(Debugcoarse, "[%04llx] mirror of previous edge, ignored", e);
			else{
				/* to avoid multiple edges between the same two supernodes, only
				 * admit one iff super-u was marked mapped before super-v;
				 * multiple u,super-v edges and super-v,super-u are prohibited */
				// FIXME: this allows consecutive redundant edges to be
				// seen as external:
				// v,a... v,b... u,a... u,b..: u,a and u,b both added
				if(s >= S && t < s){
					dprint(Debugcoarse, "[%04llx] new external edge", e);
					empput64(fedge, 2+2*M++, s);
					emput64(fedge, t);
				}else{
					dprint(Debugcoarse, "[%04llx] redundant external edge", e);
					outputedge(lp, u, v);
				}
			}
		}
		emshrink(fnode, 0);
		emshrink(fedge, 2*8 + 8*2*M);
		emsteal(fweight, fweight2);
		endlevel(lp);
		y = w;
		w = S;
	}
	dprint(Debugcoarse, "coarsen: ended at level %lld", dylen(lvl));
	if(M > 0){
		dprint(Debugcoarse, "stopping with %lld remaining edges", M);
		printtab(fedge, M);
	}
	/* add articial root node for all remaining */
	if(S > w + 1){
		s = ++S;
		dprint(Debugcoarse, "push artificial root node %llx", s);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		/* every node is an adjacency, every edge is internal */
		for(e=0, u=-1; e<M; e++){
			if((o = empget64(fedge, 2 + e*2)) != u){
				u = o;
				w = empget64(fweight, u);
				assert(o != EMbupkis && w != EMbupkis);
				outputnode(lp, u, s, w);
			}
			v = emget64(fedge);
			assert(v != EMbupkis);
			outputedge(lp, u, v);
		}
	}
	emclose(fedge);
	emclose(fnode);
	emclose(fweight);
	emclose(fweight2);
	return lvl;
}

static void
usage(void)
{
	dprint(Debugcoarse, "usage: %s [-D which] SORTED_EDGES", argv0);
	sysfatal("usage");
}

int
main(int argc, char **argv)
{
	char *s;
	Graph g;
	Lbuf *lvl;

	ARGBEGIN{
	case 'D':
		s = EARGF(usage());
		if(strcmp(s, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(s, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(s, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(s, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(s, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(s, "extmem") == 0)
			debug |= Debugextmem;
		else if(strcmp(s, "all") == 0)
			debug |= Debugtheworld;
		else{
			dprint(Debugcoarse, "unknown debug component %s", s);
			usage();
		}
		break;
	}ARGEND
	if(argc < 1)
		usage();
	if((lvl = coarsen(&g, argv[0])) == nil)
		sysfatal("coarsen: %s", error());
	if(reverse(&g, lvl) < 0)
		sysfatal("reverse: %s", error());
	return 0;
}
