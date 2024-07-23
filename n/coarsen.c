#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	// FIXME: configurable
	Minnodes = 1,
	Lrecsz = 4 * sizeof(u64int),
	Nrecsz = 4 * sizeof(u64int),
	Erecsz = 3 * sizeof(u64int),
};
typedef struct Lbuf Lbuf;
struct Lbuf{
	int nnodes;
	int nedges;
	EM *nodes;
	EM *edges;
};
static int plaintext;
static usize lastsuper, nsuper;

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
		dylen(g->nodes), dylen(g->edges), nsuper, dylen(lvl));
	put64(f, dylen(g->nodes));
	put64(f, dylen(g->edges));
	put64(f, nsuper);
	put64(f, dylen(lvl));
	noff = tellfs(f) + dylen(lvl) * Lrecsz;
	eoff = noff + lastsuper * Nrecsz;
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
outputedge(Lbuf *lp, ssize u, ssize v, int o, ssize ei, int nlvl)
{
	ssize off;
	EM *em;

	if(plaintext){
		print("%d e[%zx] %c%zx,%c%zx\n", nlvl, ei, (o&1)!=0?'-':'+', u, (o>>1&1)!=0?'-':'+', v);
		return;
	}
	DPRINT(Debugcoarse, "outputedge %zx:%d,%zx:%d", u, o&1, v, o>>1&1);
	em = lp->edges;
	off = lp->nedges++;
	emw64(em, 3*off, u << 1 | o & 1);
	emw64(em, 3*off+1, v << 1 | o >> 1 & 1);
	emw64(em, 3*off+2, ei);
}

static void
outputnode(Lbuf *lp, ssize idx, ssize old, ssize new, ssize len, int nlvl)
{
	ssize off;
	EM *em;

	if(plaintext){
		print("%d n[%zx] %zx→%zx w:%zd\n", nlvl, old, idx, new, len);
		return;
	}
	em = lp->nodes;
	off = lp->nnodes++;
	emw64(em, 4*off, idx);
	emw64(em, 4*off+1, old);
	emw64(em, 4*off+2, new);
	emw64(em, 4*off+3, len);
}

static Lbuf *
newlevel(Lbuf *lvl)
{
	Lbuf l = {0};

	if(plaintext){
		print("========\n");
		return nil;
	}
	if((l.nodes = emopen(nil)) == nil)
		sysfatal("newlevel: %s", error());
	if((l.edges = emopen(nil)) == nil)
		sysfatal("newlevel: %s", error());
	dypush(lvl, l);
	return lvl;
}

KHASH_SET_INIT_INT64(meh)
#define EXISTSONE(h, s, t, width)	\
	(kh_get(meh, (h), (s) * width + (t)) != kh_end(h))
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

Lbuf *
coarsen(Graph *g, char *uindex, char *eindex)
{
	int nlvl, d, di, eoff, o;
	ssize top, w, U, V, u, v, u0, v0, k, s, t, m, e, x, M, S, S0, ei;
	EM *edges, *nodes, *edges0;
	Lbuf *lvl, *lp;
	khash_t(meh) *h;
	khash_t(ugh) *visited;

	if((edges0 = emopen(eindex)) == nil)
		sysfatal("coarsen: %s", error());
	if((nodes = emopen(uindex)) == nil)
		sysfatal("coarsen: %s", error());
	dylen(g->nodes) = emr64(nodes, 0);
	dylen(g->edges) = emr64(edges0, 0);
	M = emr64(edges0, 1);
	S0 = dylen(g->nodes);
	if(S0 <= Minnodes){
		werrstr("number of nodes below set threshold, nothing to do");
		return nil;
	}
	S = S0 - 1;
	DPRINT(Debugcoarse, "graph %s:%s nnodes %lld nedges %lld tot edges %lld", uindex, eindex, dylen(g->nodes), dylen(g->edges), M);
	lp = lvl = nil;
	nlvl = 0;
	w = S;
	k = 0;
	d = di = 0;
	u0 = 0;
	u = -1;
	eoff = -1;
	if((edges = emopen(nil)) == nil)
		sysfatal("coarsen: %s", error());
	while(S0 > Minnodes){
		h = kh_init(meh);
		visited = kh_init(ugh);
		// FIXME: redundant with lp edge array
		if((lvl = newlevel(lvl)) != nil)
			lp = lvl + dylen(lvl) - 1;
		// FIXME: use index for ids rather than y*i+x which will blow up
		DPRINT(Debugcoarse, "===== level %zx nn %zx ne %zx", lp-lvl+1, S0, M);
		m = M;
		M = 0;
		top = -1;
		for(e=0; e<m; e++){
			if(nlvl > 0){
				x = emr64(edges, 3*e);
				U = x % (2*dylen(g->nodes));
				V = x / (2*dylen(g->nodes));
				o = U&1 | (V&1 << 1);
				u = (U >> 1) + k;
				v = (V >> 1) + k;
				x = emr64(edges, 3*e+1);
				u0 = x % dylen(g->nodes);
				v0 = x / dylen(g->nodes);
				ei = emr64(edges, 3*e+2);
			}else{
				if(di >= d){
					di = 0;
					d = emr64(nodes, 1+5*u0);
					u = emr64(nodes, 1+5*u0+1);
					eoff = emr64(nodes, 1+5*u0+2);
					u0++;
				}
				ei = eoff + di;
				// FIXME: could use len field?
				v0 = v = emr64(edges0, 2+5*ei+1);
				o = emr64(edges0, 2+5*ei+2);
				U = u << 1 | o & 1;
				V = v << 1 | o >> 1 & 1;
				assert((o & ~3) == 0);
				di++;
			}
			DPRINT(Debugcoarse, "edge [%03zd] [%zx,%zx] %zx,%zx UV %zx,%zx o %x: ",
				e, u0, v0, u, v, U, V, o);
			if((s = HESDEADJIM(visited, u)) == kh_end(visited) || (s = SULUGOTOWARP(visited, s)) <= w){
				s = ++S;
				NOTAMILKMAN(visited, u, s);
				top = u;
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\bnew super [%zx]%zx→%zx", u0, u, s);
				outputnode(lp, u, u0, s, 1, nlvl);
			}
			if((t = HESDEADJIM(visited, v)) == kh_end(visited) || (t = SULUGOTOWARP(visited, t)) <= w){
				if(u == top){
					NOTAMILKMAN(visited, v, s);
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; merge [%zx]%zx", v0, v);
					outputnode(lp, v, v0, s, 1, nlvl);
					outputedge(lp, u, v, o, ei, nlvl);
				}else
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; skip [%zx]%zx", v0, v);
			}else if(EXISTS(h, u, v, 2*dylen(g->nodes))){
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; redundant %zx:%d,%zx:%d", u, o&1, v, o>>1&1);
				if(!EXISTS(h, U, V, 2*dylen(g->nodes)) && !EXISTSONE(h, U^1, V^1, 2*dylen(g->nodes))){
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b but with unique orientation %d,%d", o&1, o>>1&1);
					SPAWN(h, U, V, 2*dylen(g->nodes));
					outputedge(lp, u, v, o, ei, nlvl);
				}
			}else if(u == v){
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; self edge");
				if(!EXISTS(h, U, V, 2*dylen(g->nodes))){
					outputedge(lp, u, v, o, ei, nlvl);
					SPAWN(h, u, v, 2*dylen(g->nodes));
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; added %zx:%zd,%zx:%zd", u, u&1, v, v&1);
					if(nlvl == 0){
						SPAWN(h, U, V, 2*dylen(g->nodes));
						DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; added %zx:%zd,%zx:%zd", U, U&1, V, V&1);
					}
				}
			}else if(t == s)
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; mirror %zx,%zx", s, t);
			else{
				DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; external[%zx] %zx,%zx", M, s, t);
				s = (s-w) << 1 | o & 1;
				t = (t-w) << 1 | o >> 1 & 1;
				SPAWN(h, u, v, 2*dylen(g->nodes));
				if(nlvl == 0){
					DPRINT(Debugcoarse, "\b\b\b\b\b\b\b\b\b\b; spawn %zx:%zd,%zx:%zd", U, (U&1), V, (V)>>1&1);
					SPAWN(h, U, V, 2*dylen(g->nodes));
				}
				emw64(edges, 3*M, t * 2*dylen(g->nodes) + s);
				emw64(edges, 3*M+1, v0 * dylen(g->nodes) + u0);
				emw64(edges, 3*M+2, ei);
				outputedge(lp, u, v, o, ei, nlvl);
				M++;
			}
		}
		kh_destroy(meh, h);
		kh_destroy(ugh, visited);
		if(nodes != nil){
			emclose(nodes);
			nodes = nil;
		}
		if(edges0 != nil){
			emclose(edges0);
			edges0 = nil;
		}
		if(!plaintext)
			endlevel(lp);
		nlvl++;
		k = w;
		S0 = S - w;
		w = S;
	}
	DPRINT(Debugcoarse, "coarsen: ended at level %d, %zd remaining nodes, %zd edges", nlvl, S0, M);
	lastsuper = nsuper;
	nsuper += S0 + 1;	// FIXME: not in the actual file
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
	char *s;
	Graph g;
	Lbuf *lvl;

	ARGBEGIN{
	case 'D':
		s = EARGF(usage());
		if(strcmp(s, "fs") == 0)
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
		multiplier = atoi(EARGF(usage()));
		break;
	case 'n':
		plaintext = 1;
		break;
	}ARGEND
	if(argc < 2)
		usage();
	initem();
	lvl = coarsen(&g, argv[0], argv[1]);
	if(plaintext)
		return 0;
	else if(lvl == nil)
		sysfatal("coarsen: %s", error());
	if(reverse(&g, lvl) < 0)
		sysfatal("reverse: %s", error());
	return 0;
}
