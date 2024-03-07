#include "strpg.h"
#include "fs.h"
#include "em.h"

static void
printtab(EM *em, EM *map, ssize ne, ssize k)
{
	int i;
	ssize u, v, s, t;

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
		warn("⇒ E[%d] %zx,%zx (%zx,%zx)\n", i, s, t, u, v);
	}
}

KHASH_SET_INIT_INT64(meh)
#define EXISTS(h, s, t, width)	(kh_get(meh, (h), (s) * width + (t)) != kh_end(h))
#define SPAWN(h, s, t, width)	do{ \
	int ret; \
	kh_put(meh, (h), (s) * (width) + (t), &ret); \
	kh_put(meh, (h), (t) * (width) + (s), &ret); \
	}while(0)

static int
coarsen(Graph *g, char *index)
{
	int topdog, lvl;
	ssize w, u, v, k, s, t, m, e, uw, vw, x, M, S;
	EM *fedge, *fnode, *fweight, *fshit;
	khash_t(meh) *h;

	if((fedge = emopen(index, 0)) == nil)
		sysfatal("coarsen: %s", error());
	g->nnodes = emr64(fedge, 0);
	g->nedges = emr64(fedge, 1);
	assert(g->nedges > 0);
	fnode = emopen(nil, 0);
	fweight = emopen(nil, 0);
	M = g->nedges;
	S = g->nnodes - 1;
	w = S;
	k = 0;
	uw = 0;	/* cannot happen */
	lvl = 0;
	while(M > 0){
		h = kh_init(meh);
		fshit = emopen(nil, 0);
		warn("===== level %d\n", lvl++);
		m = M;
		M = 0;
		topdog = 0;
		for(e=0; e<m; e++){
			u = emr64(fedge, 2+e*2+0);
			v = emr64(fedge, 2+e*2+1);
			warn(">> [%03zd] EDGE %zx,%zx\n", e, u, v);
			if((s = emr64(fnode, u) - 1) < 0)
				s = u;
			if(s <= w){
				s = ++S;
				if((uw = emr64(fweight, u) - 1) < 0)
					uw = 1;	/* default value */
				emw64(fnode, u, s+1);
				x = emr64(fshit, e*2+0);
				warn("NEWSUPER %zx → %zx [%lld]\n", x, s, uw);
				topdog = u;		// FIXME: only works if in order (and left always is?)
			}
			if((t = emr64(fnode, v) - 1) < 0)
				t = v;
			if(t <= w){
				if(u != topdog){
					warn("SKIPOUTER %zx,%zx (%zx,%zx)\n", s, t, u, v);
					continue;
				}
				if((vw = emr64(fweight, v) - 1) < 0)
					vw = 1;	/* default value */
				emw64(fnode, v, s+1);
				emw64(fweight, u, uw+vw+1);
				x = emr64(fshit, e*2+1);
				warn("MERGERIGHT %zx,%zx (%zx,%zx)\n", s, t, u, x);
			}else if(v == u){
				warn("SELF %zx,%zx (%zx,%zx)\n", s, t, u, v);
			}else if(t == s){
				warn("MIRROR %zx,%zx (%zx,%zx)\n", s, t, u, v);
			}else if(EXISTS(h, s, t, g->nnodes)){
				warn("REDUNDANT %zx,%zx (%zx,%zx)\n", s, t, u, v);
			}else{
				warn("EXT %zx,%zx (%zx,%zx)\n", s, t, u, v);
				emw64(fedge, 2+M*2+0, s);
				emw64(fedge, 2+M*2+1, t);
				emw64(fshit, M*2+0, u);
				emw64(fshit, M*2+1, v);
				SPAWN(h, s, t, g->nnodes);
				M++;
			}
		}
		printtab(fedge, fnode, M, k);
		if(k == 0)
			k = w;
		w = S;
		kh_destroy(meh, h);
		emclose(fshit);
		lvl++;
	}
	emclose(fnode);
	emclose(fedge);
	return 0;
}

static void
usage(void)
{
	warn("usage: %s [-m 16-63] WIDELIST\n", argv0);
	sysfatal("usage");
}

int
main(int argc, char **argv)
{
	int m;
	Graph g;

	ARGBEGIN{
	case 'm':
		multiplier = atoi(EARGF(usage()));
		break;
	}ARGEND
	if(argc < 1)
		usage();
	initem();
		sysfatal("invalid multiplier");
	if(coarsen(&g, argv[0]) < 0)
		sysfatal("coarsen: %s", error());
	return 0;
}
