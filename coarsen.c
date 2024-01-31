#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	// FIXME: configurable
	Minnodes = 1,
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
endlevel(Lbuf *lp)
{
	nsuper += lp->nnodes;
}

static void
outputedge(Lbuf *lp, ssize u, ssize v)
{
	ssize off;
	EM *em;

	DPRINT(Debugcoarse, "outputedge %zx,%zx", u, v);
	em = lp->edges;
	off = lp->nedges++;
	emw64(em, 2*off, u);
	emw64(em, 2*off+1, v);
}

static void
outputnode(Lbuf *lp, ssize idx, ssize old, ssize new, ssize weight)
{
	ssize off;
	EM *em;

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
	return lvl;
}

KHASH_SET_INIT_INT64(meh)
#define EXISTS(h, s, t, width)	\
	(kh_get(meh, (h), (s) * width + (t)) != kh_end(h) \
	|| kh_get(meh, (h), (t) * width + (s)) != kh_end(h))
#define SPAWN(h, s, t, width)	do{ \
	int ret; \
	kh_put(meh, (h), (t) * (width) + (s), &ret); \
	}while(0)
KHASH_MAP_INIT_INT64(ugh, ssize)
#define NOTAMILKMAN(h, u, s)	do{ \
	int ret; \
	khiter_t __k; \
	__k = kh_put(ugh, (h), (u), &ret); \
	kh_value((h), __k) = (s); \
	}while(0)
#define HESDEADJIM(h, u)	(kh_get(ugh, (h), (u)))	/* put an torpe-do-oh-oh */
#define SULUGOTOWARP(h, k)	(kh_value((h), (k)))	/* warp 3, sir? */

int
exists(ssize s, ssize t, EM *edges, int width)
{
	ssize x, v;

	x = s * width + t;
	v = emr64(edges, x >> 6);
	return (v & 1ULL << (x & 63)) != 0;
}

void
mark(ssize s, ssize t, EM *edges, int width)
{
	ssize x, v;

	x = t * width + s;
	v = emr64(edges, x >> 6);
	emw64(edges, x >> 6, v | 1ULL << (x & 63));
	x = s * width + t;
	v = emr64(edges, x >> 6);
	emw64(edges, x >> 6, v | 1ULL << (x & 63));
}

Lbuf *
coarsen(Graph *g, char *uindex, char *eindex)
{
	int nlvl, d, di, eoff;
	ssize top, w, u, v, u0, v0, k, s, t, m, e, x, M, S, S0;
	EM *edges, *nodes, *edgeuv, *edgeset;
	Lbuf *lvl, *lp;
	khash_t(meh) *h;
	khash_t(ugh) *visited;

	if((edges = emopen(eindex, 0)) == nil)
		sysfatal("coarsen: %s", error());
	if((nodes = emopen(uindex, 0)) == nil)
		sysfatal("coarsen: %s", error());
	g->nnodes = emr64(nodes, 0);
	g->nedges = emr64(edges, 0);
	M = emr64(edges, 1);
	S0 = g->nnodes;
	if(S0 <= Minnodes){
		werrstr("number of nodes below set threshold, nothing to do");
		return nil;
	}
	S = S0 - 1;
	DPRINT(Debugcoarse, "graph %s:%s nnodes %lld nedges %lld tot edges %lld", uindex, eindex, g->nnodes, g->nedges, M);
	edgeuv = emopen(nil, 0);
	lp = lvl = nil;
	nlvl = 0;
	w = S;
	k = 0;
	d = di = 0;
	u0 = 0;
	u = -1;
	//ismirror = 0;
	eoff = -1;
	while(S0 > Minnodes){
		h = kh_init(meh);
		visited = kh_init(ugh);
		edgeset = emopen(nil, 0);
		if(!plaintext){
			lvl = newlevel(lvl);
			lp = lvl + dylen(lvl) - 1;
		}
		// FIXME: use index for ids rather than y*i+x which will blow up
		DPRINT(Debugcoarse, "===== level %zx", lp-lvl+1);
		m = M;
		M = 0;
		top = -1;
		for(e=0; e<m; e++){
			if(nlvl > 0){
				x = emr64(edges, e);
				u = x % g->nnodes + k;
				v = x / g->nnodes + k;
				x = emr64(edgeuv, e);
				u0 = x % g->nnodes;
				v0 = x / g->nnodes;
			}else{
				if(di >= d){
					di = 0;
					d = emr64(nodes, 1+5*u0);
					u = emr64(nodes, 1+5*u0+1);
					eoff = emr64(nodes, 1+5*u0+2);
					u0++;
				}
				// FIXME: could use len field?
				v0 = v = emr64(edges, 2+5*(eoff+di)+1);
				//o = emr64(edges, 2+5*(eoff+di)+2);
				//ismirror = (ssize)emr64(edges, 2+5*(eoff+di)+3) < 0;
				di++;
			}
			DPRINT(Debugcoarse, "edge [%03zd] [%zx,%zx] %zx,%zx: ", e, u0, v0, u, v);
			/* below memory limit, this over EM *visited has barely any effect */
			if((s = HESDEADJIM(visited, u)) == kh_end(visited) || (s = SULUGOTOWARP(visited, s)) <= w){
				s = ++S;
				NOTAMILKMAN(visited, u, s);
				top = u;
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\bnew super [%zx]%zx→%zx", u0, u, s);
				outputnode(lp, u, u0, s, 1);
			}
			if((t = HESDEADJIM(visited, v)) == kh_end(visited) || (t = SULUGOTOWARP(visited, t)) <= w){
				if(u == top){
					NOTAMILKMAN(visited, v, s);
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; merge [%zx]%zx", v0, v);
					outputnode(lp, v, v0, s, 1);
					S0--;
					outputedge(lp, u, v);
				}else
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; skip [%zx]%zx", v0, v);
			}else if(EXISTS(h, u, v, g->nnodes)){
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; redundant %zx,%zx", s, t);
				// FIXME: handle orientation correctly; we just output ++ always,
				//	so at lvl0, output all with unique orientation
				//	at lvl > 0, just output with orientation
				//	just use lower bit
				/*
				if(!ismirror){
					outputedge(lp, u, v);
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b but unique orientation", s, t);
				}
				*/
			}else if(u == v){
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; self edge");
				outputedge(lp, u, v);
				SPAWN(h, u, v, g->nnodes);
			}else if(t == s)
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; mirror %zx,%zx", s, t);
			else{
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; external %zx,%zx", s, t);
				s -= w;
				t -= w;
				SPAWN(h, u, v, g->nnodes);
				emw64(edges, M, t * g->nnodes + s);
				emw64(edgeuv, M, v0 * g->nnodes + u0);
				outputedge(lp, u, v);
				M++;
			}
		}
		kh_destroy(meh, h);
		kh_destroy(ugh, visited);
		emclose(edgeset);
		if(nodes != nil){
			emclose(nodes);
			nodes = nil;
		}
		if(!plaintext)
			endlevel(lp);
		nlvl++;
		k = w;
		w = S;
		//ismirror = 0;
	}
	DPRINT(Debugcoarse, "coarsen: ended at level %d, %zd remaining nodes, %zd edges", nlvl, S0, M);
	/* add articial root node for all remaining */
	// FIXME: out of date, untested
#ifdef fuck
	if(S > w + 1){
		s = ++S;
		DPRINT(Debugcoarse, "push artificial root node %zx", s);
		if(!plaintext){
			lvl = newlevel(lvl);
			lp = lvl + dylen(lvl) - 1;
		}
		/* every node is an adjacency, every edge is internal */
		for(e=0, u=-1; e<M; e++){
			if((o = emr64(edges, e*2)) != u){
				u = o;
				z = emr64(visited, u) - 1;
				outputnode(lp, u, z, s, w);
			}
		}
	}
#endif
	emclose(edgeuv);
	emclose(edges);
	return lvl;
}

static void
usage(void)
{
	warn("usage: %s [-n] [-m 16-63] BINNODES BINEDGES\n", argv0);
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
	if(argc < 2)
		usage();
	lvl = coarsen(&g, argv[0], argv[1]);
	if(plaintext)
		return 0;
	else if(lvl == nil)
		sysfatal("coarsen: %s", error());
	if(reverse(&g, lvl) < 0)
		sysfatal("reverse: %s", error());
	return 0;
}
