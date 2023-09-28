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

static int
reverse(Graph *g, Lbuf *lvl)
{
	ssize noff, eoff;
	Lbuf *lp, *le;
	EM *em;

	/* FIXME: for a user, failing at this point is... disappointing. */
	if((em = emfdopen(1)) == nil)
		sysfatal("emfdopen: %s\n", error());
	emput64(em, g->nnodes);
	emput64(em, g->nedges);
	emput64(em, dylen(lvl));
	noff = em->off;
	noff += dylen(lvl) * Lrecsz;
	eoff = noff + Nrecsz;
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		emput64(em, lp->nnodes);
		emput64(em, lp->nedges);
		emput64(em, noff);
		emput64(em, eoff);
		noff += lp->nnodes * Nrecsz;
		eoff += lp->nedges * Erecsz;
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		emappend(em, lp->nodes);
		emclose(lp->nodes);
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		emappend(em, lp->edges);
		emclose(lp->edges);
	}
	emflush(em);
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
	emseek(em, 0, 0);
	for(i=0; i<ne; i++){
		u = emget64(em);
		v = emget64(em);
		dprint(Debugcoarse, "E[%d] %llx,%llx\n", i, u, v);
	}
}

static void
endlevel(Lbuf *lp, Graph *g)
{
	g->nnodes += lp->nnodes;
	g->nedges += lp->nedges;
}

static void
outputedge(Lbuf *lp, ssize u, ssize v)
{
	EM *em;

	dprint(Debugcoarse, "drop edge %llx,%llx\n", u, v);
	em = lp->edges;
	emput64(em, u);
	emput64(em, v);
	lp->nedges++;
}

static void
outputnode(Lbuf *lp, ssize old, ssize new, ssize weight)
{
	EM *em;

	dprint(Debugcoarse, "merge %llx → %llx, weight %lld\n", old, new, weight);
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
	dprint(Debugcoarse, "-- newlevel %lld\n", dylen(lvl));
	return lvl;
}

static Lbuf *
coarsen(Graph *g, char *index)
{
	ssize ls, lt, i, j, o, w, u, v, s, t, m, e, uw, vw, M, S;
	EM *fedge, *fnode, *fweight, *fweight2, *flastpoke;
	Lbuf *lvl, *lp;

	if((fedge = emopen(index)) == nil)
		sysfatal("emclone: %s", error());
	fweight = emnew(0);
	fweight2 = emnew(0);
	fnode = emnew(0);
	flastpoke = emnew(0);
	M = g->nedges;
	S = g->nnodes - 1;
	g->nnodes = 0;	/* vandalize it, don't care */
	g->nedges = 0;
	lvl = nil;
	w = S;
	i = 0;
	uw = -1;	/* cannot happen */
	s = -1;	/* cannot happen */
	while(M > Minedges){
		printtab(fedge, M);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		u = -1;
		m = M;
		M = 0;
		/* FIXME: could directly skip the second half of the list in the first
		 * iteration? that'd be a big speed up */
		for(e=0; e<m; e++){
			/* new left node */
			if((o = emget64(fedge)) != u){
				u = o;
				s = empget64(fnode, u);
				dprint(Debugcoarse, "[%04llx] new left node %llx → %llx\n", e, u, s);
				i = s - 1 - w;
				/* buffer contents are > w or undefined */
				if(s <= w){
					dprint(Debugcoarse, "[%04llx] unvisited left node: %llx ← %llx\n",
						e, u, S+1);
					s = ++S;
					i = s - 1 - w;
					uw = empget64(fweight, u);
					outputnode(lp, u, s, uw);
					empput64(fnode, u, s);
					empput64(fweight2, i, uw);
					empput64(flastpoke, i, i);
				}
			}
			v = emget64(fedge);
			t = empget64(fnode, v);
			dprint(Debugcoarse, "[%04llx] edge %llx→%llx,%llx→%llx\n", e, u, s, v, t);
			if(t <= w){
				dprint(Debugcoarse, "[%04llx] unvisited right node: %llx ← %llx\n",
					e, v, s);
				vw = empget64(fweight, v);
				outputnode(lp, v, u, vw);
				uw += vw;
				empput64(fweight2, i, uw);
				outputedge(lp, u, v);
			}else if(v == u){
				dprint(Debugcoarse, "[%04llx] self edge: %llx,%llx\n", e, u, s);
				outputedge(lp, u, u);
			}else if(t == s)
				dprint(Debugcoarse, "[%04llx] mirror of previous edge, ignored\n", e);
			else{
				j = t - 1 - w;
				lt = empget64(flastpoke, j);
				ls = empget64(flastpoke, i);
				assert(lt >= 0 && ls >= 0);
				/* to avoid multiple edges between the same two supernodes, only
				 * admit one iff super-u was marked mapped before super-v;
				 * multiple u,super-v edges and super-v,super-u are prohibited */
				if(ls < lt){
					dprint(Debugcoarse, "[%04llx] new external edge (%lld < %lld)\n",
						e, ls, lt);
					empput64(flastpoke, j, i);
					empput64(fedge, 2*M++, s);
					emput64(fedge, t);
				}else{
					dprint(Debugcoarse, "[%04llx] redundant external edge\n", e);
					outputedge(lp, u, v);
				}
			}
		}
		emshrink(fnode, 8*(S-w));
		emshrink(fedge, 8*2*M);
		emshrink(flastpoke, 0);
		emflip(fweight, fweight2);
		endlevel(lp, g);
		w = S;
	}
	if(M > 0){
		dprint(Debugcoarse, "stopping with %lld remaining edges\n", M);
		printtab(fedge, M);
	}
	/* add articial root node for all remaining */
	if(S > w + 1){
		s = ++S;
		dprint(Debugcoarse, "push artificial root node %llx\n", s);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		emseek(fedge, 0, 0);
		/* every node is an adjacency, every edge is internal */
		for(e=0, u=-1; e<M; e++){
			if((o = emget64(fedge)) != u){
				u = o;
				w = empget64(fweight, u);
				outputnode(lp, u, s, w);
			}
			v = emget64(fedge);
			outputedge(lp, u, v);
		}
	}
	emclose(fedge);
	emclose(fnode);
	emclose(fweight);
	emclose(fweight2);
	emclose(flastpoke);
	return lvl;
}

static int
readindex(Graph *g, char *path)
{
	File *f;

	f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		sysfatal("readindex: %s", error());
	g->nnodes = get64(f);
	g->nedges = get64(f);
	dprint(Debugcoarse, "graph %s nnodes %lld nedges %lld\n",
		f->path, g->nnodes, g->nedges);
	freefs(f);
	return 0;
}

static void
usage(void)
{
	dprint(Debugcoarse, "usage: %s [-D which] SORTED_EDGES\n", argv0);
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
			dprint(Debugcoarse, "unknown debug component %s\n", s);
			usage();
		}
		break;
	}ARGEND
	if(argc < 1)
		usage();
	if(readindex(&g, argv[0]) < 0)
		sysfatal("readindex: %s", error());
	if((lvl = coarsen(&g, argv[0])) == nil)
		sysfatal("coarsen: %s", error());
	if(reverse(&g, lvl) < 0)
		sysfatal("reverse: %s", error());
	return 0;
}
