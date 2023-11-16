#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	Minedges = 0,
	Lrecsz = 4 * sizeof(u64int),
	Nrecsz = 4 * sizeof(u64int),
	Erecsz = 2 * sizeof(u64int),
};
typedef struct Lbuf Lbuf;
struct Lbuf{
	int nnodes;
	int nedges;
	EM *nodes;
	EM *edges;
};
static int plaintext;
static usize nsuper;

// FIXME: fix fucking openfs semantics
static int
reverse(Graph *g, Lbuf *lvl)
{
	ssize noff, eoff;
	Lbuf *lp, *le;
	File *f;

	/* FIXME: for a user, failing at this point is... disappointing. */
	f = emalloc(sizeof *f);
	if(fdopenfs(f, 1, OWRITE) < 0)
		sysfatal("fdopenfs: %s", error());
	DPRINT(Debugcoarse, "reverse: %zx %zx %zx %zx",
		g->nnodes, g->nedges, nsuper, dylen(lvl));
	put64(f, g->nnodes);
	put64(f, g->nedges);
	put64(f, nsuper);
	put64(f, dylen(lvl));
	noff = tellfs(f) + dylen(lvl) * Lrecsz;
	eoff = noff + nsuper * Nrecsz;
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		put64(f, lp->nnodes);
		put64(f, lp->nedges);
		put64(f, noff);
		put64(f, eoff);
		noff += lp->nnodes * Nrecsz;
		eoff += lp->nedges * Erecsz;
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		DPRINT(Debugcoarse, "[%zx] %d nodes", lp-lvl, lp->nnodes);
		if(em2fs(lp->nodes, f, lp->nnodes * Nrecsz) < 0)
			sysfatal("reverse: %s", error());
		emclose(lp->nodes);
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		DPRINT(Debugcoarse, "[%zx] %d edges", lp-lvl, lp->nedges);
		if(em2fs(lp->edges, f, lp->nedges * Erecsz) < 0)
			sysfatal("reverse: %s", error());
		emclose(lp->edges);
	}
	closefs(f);
	free(f);
	return 0;
}

static void
printtab(EM *em, EM *map, ssize ne, ssize k, int nlvl)
{
	int i;
	ssize u, v, s, t;

	if(!plaintext && (debug & Debugcoarse) == 0)
		return;
	warn("level\t%d\t%zd\n", nlvl, ne);
	for(i=0; i<ne; i++){
		u = emr64(em, 2+i*2);
		v = emr64(em, 2+i*2+1);
		if((s = emr64(map, u) - 1) <= k){
			if(s >= 0)
				s = emr64(map, s) - 1;
			else
				s = u;
		}
		if((t = emr64(map, v) - 1) <= k){
			if(t >= 0)
				t = emr64(map, t) - 1;
			else
				t = v;
		}
		warn("%zx\t%zx\n", s, t);
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
	ssize off;
	EM *em;

	em = lp->edges;
	off = lp->nedges++;
	emw64(em, 2*off, u);
	emw64(em, 2*off+1, v);
	DPRINT(Debugcoarse, "drop edge %zx,%zx %d %zx,%zx", u, v, lp->nedges, 2*off, 2*off+1);
}

static void
outputnode(Lbuf *lp, ssize idx, ssize old, ssize new, ssize weight)
{
	ssize off;
	EM *em;

	DPRINT(Debugcoarse, "merge [%zx] %zx → %zx, weight %lld", idx, old, new, weight);
	em = lp->nodes;
	off = lp->nnodes++;
	emw64(em, 4*off, idx);
	emw64(em, 4*off+1, old);
	emw64(em, 4*off+2, new);
	emw64(em, 4*off+3, weight);
}

static Lbuf *
newlevel(Lbuf *lvl)
{
	Lbuf l = {0};

	if((l.nodes = emopen(nil, 0)) == nil)
		sysfatal("newlevel: %s", error());
	if((l.edges = emopen(nil, 0)) == nil)
		sysfatal("newlevel: %s", error());
	dypush(lvl, l);
	DPRINT(Debugcoarse, "-- newlevel %lld", dylen(lvl));
	return lvl;
}

KHASH_SET_INIT_INT64(meh)
#define EXISTS(h, s, t, width)	(kh_get(meh, (h), (s) * width + (t)) != kh_end(h))
#define SPAWN(h, s, t, width)	do{ \
	int ret; \
	kh_put(meh, (h), (s) * (width) + (t), &ret); \
	kh_put(meh, (h), (t) * (width) + (s), &ret); \
	}while(0)

static Lbuf *
coarsen(Graph *g, char *index)
{
	int topdog, nlvl;
	ssize o, w, u, v, z, k, s, t, m, e, uw, vw, x, M, S;
	EM *fedge, *fnode, *fweight;
	Lbuf *lvl, *lp;
	khash_t(meh) *h;

	if((fedge = emopen(index, 0)) == nil)
		sysfatal("coarsen: %s", error());
	g->nnodes = emr64(fedge, 0);
	g->nedges = emr64(fedge, 1);
	if(g->nedges <= Minedges){
		werrstr("number of edges below set threshold, nothing to do");
		return nil;
	}
	DPRINT(Debugcoarse, "graph %s nnodes %lld nedges %lld",
		index, g->nnodes, g->nedges);
	fnode = emopen(nil, 0);
	fweight = emopen(nil, 0);
	M = g->nedges;
	S = g->nnodes - 1;
	lp = lvl = nil;
	nlvl = 0;
	w = S;
	k = 0;
	uw = 0;	/* cannot happen */
	while(M > Minedges){
		h = kh_init(meh);
		if(!plaintext){
			lvl = newlevel(lvl);
			lp = lvl + dylen(lvl) - 1;
		}
		m = M;
		M = 0;
		topdog = 0;
		for(e=0; e<m; e++){
			if(e > 0 && e % 1000 == 0)
				DPRINT(Debugcoarse, "L%02d edge %zx/%zx\n", nlvl, e, g->nedges);
			u = emr64(fedge, 2+e*2+0);
			v = emr64(fedge, 2+e*2+1);
			if((s = emr64(fnode, u) - 1) < 0)
				s = u;
			else if((x = emr64(fnode, s) - 1) >= 0)
				s = x;
			if((t = emr64(fnode, v) - 1) < 0)
				t = v;
			else if((x = emr64(fnode, t) - 1) >= 0)
				t = x;
			DPRINT(Debugcoarse, "processing %#p %zx,%zx, %zx,%zx [w %zx S %zx]",
				fedge, 2+e*2, 2+e*2+1, u, v, w, S);
			/* FIXME: fuzz against awk */
			if(s <= w){
				z = s;
				s = ++S;
				DPRINT(Debugcoarse, "[%04zx] unvisited left node[%zx] ← %zx", e, u, s);
				if((uw = emr64(fweight, u) - 1) < 0)
					uw = 1;	/* default value */
				emw64(fnode, z, s+1);
				emw64(fnode, u, s+1);
				if(!plaintext)
					outputnode(lp, u, z, s, uw);
				topdog = u;		// FIXME: only works if in order (is it always more or less preserved on the left hand side at least?)
			}
			DPRINT(Debugcoarse, "\t>> %zx,%zx → %zx,%zx", u, v, s, t);
			if(t <= w){
				z = t;
				DPRINT(Debugcoarse, "[%04zx] unvisited right node: %zx w %zd", e, v, w);
				if(u != topdog){
					DPRINT(Debugcoarse, "vassal may not annex nodes on its own");
					continue;
				}
				if((vw = emr64(fweight, v) - 1) < 0)
					vw = 1;	/* default value */
				emw64(fnode, t, s+1);
				emw64(fnode, v, s+1);
				emw64(fweight, u, uw+vw+1);
				if(!plaintext){
					outputnode(lp, v, z, s, vw);
					outputedge(lp, u, v);
				}
			}else if(v == u){
				DPRINT(Debugcoarse, "[%04zx] self edge: %zx,%zx", e, u, s);
				if(!plaintext)
					outputedge(lp, u, u);
			}else if(t == s){
				DPRINT(Debugcoarse, "[%04zx] mirror of previous edge, ignored", e);
			}else if(EXISTS(h, s, t, g->nnodes)){
				DPRINT(Debugcoarse, "[%04zx] redundant external edge", e);
				if(!plaintext)
					outputedge(lp, u, v);
			}else{
				DPRINT(Debugcoarse, "[%04zx] new external edge %zx,%zx", e, s, t);
				emw64(fedge, 2+M*2+0, u);
				emw64(fedge, 2+M*2+1, v);
				SPAWN(h, s, t, g->nnodes);
				M++;
			}
		}
		if(!plaintext)
			endlevel(lp);
		printtab(fedge, fnode, M, k, nlvl++);
		if(k == 0)
			k = w;
		w = S;
		kh_destroy(meh, h);
	}
	DPRINT(Debugcoarse, "coarsen: ended at level %d", nlvl);
	if(M > 0)
		DPRINT(Debugcoarse, "stopping with %lld remaining edges", M);
	/* add articial root node for all remaining */
	if(S > w + 1){
		s = ++S;
		DPRINT(Debugcoarse, "push artificial root node %zx", s);
		if(!plaintext){
			lvl = newlevel(lvl);
			lp = lvl + dylen(lvl) - 1;
		}
		/* every node is an adjacency, every edge is internal */
		for(e=0, u=-1; e<M; e++){
			if((o = emr64(fedge, 2+e*2)) != u){
				u = o;
				w = emr64(fweight, u);
				z = emr64(fnode, u) - 1;
				outputnode(lp, u, z, s, w);
			}
			v = emr64(fedge, 2+e*2+1);
			outputedge(lp, u, v);
		}
		printtab(fedge, fnode, M, k, nlvl);
	}
	emclose(fnode);
	emclose(fedge);
	return lvl;
}

static void
usage(void)
{
	warn("usage: %s [-n] [-m 16-63] WIDELIST\n", argv0);
	sysfatal("usage");
}

int
main(int argc, char **argv)
{
	int m;
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
			DPRINT(Debugcoarse, "unknown debug component %s", s);
			usage();
		}
		break;
	case 'm':
		m = atoi(EARGF(usage()));
		if(eminit(m) < 0)
			sysfatal("invalid multiplier");
		break;
	case 'n':
		plaintext = 1;
		break;
	}ARGEND
	if(argc < 1)
		usage();
	lvl = coarsen(&g, argv[0]);
	if(plaintext)
		return 0;
	else if(lvl == nil)
		sysfatal("coarsen: %s", error());
	if(reverse(&g, lvl) < 0)
		sysfatal("reverse: %s", error());
	return 0;
}
