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
	File *f;

	/* FIXME: for a user, failing at this point is... disappointing. */
	f = emalloc(sizeof *f);
	if(fdopenfs(f, 1, OWRITE) < 0)
		sysfatal("fdopenfs: %s", error());
	DPRINT(Debugcoarse, "reverse: %zd %zd %zd %zd",
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
		DPRINT(Debugcoarse, "[%zd] %d nodes", lp-lvl, lp->nnodes);
		emflushtofs(lp->nodes, f);
		emclose(lp->nodes);
	}
	for(le=lvl+dylen(lvl)-1, lp=le; lp>=lvl; lp--){
		DPRINT(Debugcoarse, "[%zd] %d edges", lp-lvl, lp->nedges);
		emflushtofs(lp->edges, f);
		emclose(lp->edges);
	}
	closefs(f);
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
		u = emr64(em, 2+i*2);
		v = emr64(em, 2+i*2+1);
		DPRINT(Debugcoarse, "E[%d] %zx,%zx", i, u, v);
	}
}

static void
endlevel(Lbuf *lp)
{
	nsuper += lp->nnodes;
	close(lp->nodes->fd);
	lp->nodes->fd = -1;
	close(lp->edges->fd);
	lp->edges->fd = -1;
}

static void
outputedge(Lbuf *lp, ssize u, ssize v)
{
	ssize off;
	EM *em;

	DPRINT(Debugcoarse, "drop edge %llx,%llx", u, v);
	em = lp->edges;
	off = lp->nedges++;
	emw64(em, 2*off, u);
	emw64(em, 2*off+1, v);
	lp->nedges++;
}

static void
outputnode(Lbuf *lp, ssize old, ssize new, ssize weight)
{
	ssize off;
	EM *em;

	DPRINT(Debugcoarse, "merge %llx → %llx, weight %lld", old, new, weight);
	em = lp->nodes;
	off = lp->nnodes++;
	emw64(em, 3*off, old);
	emw64(em, 3*off+1, new);
	emw64(em, 3*off+2, weight);
	lp->nnodes++;
}

static Lbuf *
newlevel(Lbuf *lvl)
{
	Lbuf l = {0};

	if((l.nodes = emnew(0)) == nil)
		sysfatal("emnew: %s", error());
	if((l.edges = emnew(0)) == nil)
		sysfatal("emnew: %s", error());
	dypush(lvl, l);
	DPRINT(Debugcoarse, "-- newlevel %lld", dylen(lvl));
	return lvl;
}

static int
exists(usize s, usize t, EM *fs2j, EM *fjump, EM *fedge, usize nedges, usize w)
{
	usize p;

	if((p = emr64(fs2j, s-1-w)) == EMbupkis){
		DPRINT(Debugcoarse, "s2j[%zx] not found", s-1-w);
		return 0;
	}
	while(p < nedges){
		DPRINT(Debugcoarse, "check s2j[%zx] → jump[%zx], looking for %zx,%zx",
			s-1-w, p, s, t);
		if(emr64(fedge, 2+2*p) != s)
			break;
		if(emr64(fedge, 2+2*p+1) == t)
			return 1;
		p = emr64(fjump, p);
	}
	return 0;
}
/* edgelist is not reordered; the jump table serves as
 * a linked list instead
 * s2j[u] → i, first occurrence of u in jump table
 * edge[i] → u,_ corresponding edge
 * jump[i] → j, next edge in order
 */
static void
insert(usize s, EM *fs2j, EM *fjump, EM *fedge, EM *fdeg, usize cur)
{
	int i;
	usize x, p;

	// FIXME: looping through everything; smarter, cleaner way
	if((p = emr64(fs2j, s)) != EMbupkis){
		p += emr64(fdeg, s) - 1;
		if((x = emr64(fjump, p)) != cur)
			emw64(fjump, p, cur);
		else
			x = cur + 1;
	}else{
		x = cur + 1;
		emw64(fs2j, s, cur);
	}
	emw64(fjump, cur, x);
	if((debug & Debugcoarse) == 0)
		return;
	warn(">> index\t");
	for(i=0; i<cur; i++)
		warn("%02x ", i);
	warn("\n>> edgeu\t");
	for(i=0; i<cur; i++)
		warn("%02zx ", emr64(fedge, 2+2*i));
	warn("\n>> jump \t");
	for(i=0; i<cur; i++)
		warn("%02zx ", emr64(fjump, i));
	warn("\n");
}

static Lbuf *
coarsen(Graph *g, char *index)
{
	int topdog;
	ssize i, o, w, u, v, s, t, y, m, e, uw, vw, d, M, S;
	EM *fedge, *fedge2, *fnode, *fweight, *fweight2, *fs2j, *fjump, *fdeg;
	Lbuf *lvl, *lp;

	if((fedge = emclone(index)) == nil)
		sysfatal("emclone: %s", error());
	g->nnodes = emr64(fedge, 0);
	g->nedges = emr64(fedge, 1);
	if(g->nedges <= Minedges){
		werrstr("number of edges below set threshold, nothing to do");
		return nil;
	}
	DPRINT(Debugcoarse, "graph %s nnodes %lld nedges %lld",
		index, g->nnodes, g->nedges);
	fweight = emnew(0);
	M = g->nedges;
	S = g->nnodes - 1;
	lvl = nil;
	w = S;
	y = 0;
	i = 0;
	uw = -1;	/* cannot happen */
	while(M > Minedges){
		fs2j = emnew(0);
		fjump = emnew(0);
		fnode = emnew(0);
		fweight2 = emnew(0);
		fedge2 = emnew(0);
		fdeg = emnew(0);
		printtab(fedge, M);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		m = M;
		M = 0;
		topdog = 0;
		for(e=0; e<m; e++){
			if(e > 0 && e % 1000 == 0)
				DPRINT(Debugcoarse, "L%02zd edge %lld/%lld\n", dylen(lvl), e, g->nedges);
			// FIXME: layer violation
			u = emr64(fedge, 2+e*2);
			v = emr64(fedge, 2+e*2+1);
			DPRINT(Debugcoarse, "processing %zx,%zx (%zx,%zx) [%zx,%zx]", u, v, u-y, v-y, emr64(fnode,u-y), emr64(fnode,v-y));
			if((s = emr64(fnode, u-y)) == EMbupkis || s <= w){
				s = ++S;
				DPRINT(Debugcoarse, "[%04llx] unvisited left node %llx ← %llx", e, u, s);
				i = s - 1 - w;
				if((uw = emr64(fweight, i)) == EMbupkis)
					uw = 1;	/* default value */
				emw64(fnode, u-y, s);
				emw64(fweight2, i, uw);
				outputnode(lp, u, s, uw);
				topdog = u;
			}
			/* new right node */
			if((t = emr64(fnode, v-y)) == EMbupkis || t <= w){
				DPRINT(Debugcoarse, "[%04llx] unvisited right node: %llx", e, v);
				if(u != topdog){
					DPRINT(Debugcoarse, "vassal may not annex nodes on its own");
					continue;
				}
				if((vw = emr64(fweight, i)) == EMbupkis)
					vw = 1;	/* default value */
				outputnode(lp, v, s, vw);
				uw += vw;
				emw64(fnode, v-y, s);
				emw64(fweight2, i, uw);
				outputedge(lp, u, v);
			}else if(v == u){
				DPRINT(Debugcoarse, "[%04llx] self edge: %llx,%llx", e, u, s);
				outputedge(lp, u, u);
			}else if(t == s)
				DPRINT(Debugcoarse, "[%04llx] mirror of previous edge, ignored", e);
			else if(exists(s, t, fs2j, fjump, fedge2, M, w)
			|| exists(t, s, fs2j, fjump, fedge2, M, w)){
				DPRINT(Debugcoarse, "[%04llx] redundant external edge", e);
				outputedge(lp, u, v);
			}else{
				DPRINT(Debugcoarse, "[%04llx] new external edge %zx,%zx", e, s, t);
				emw64(fedge2, 2+2*M, s);
				emw64(fedge2, 2+2*M+1, t);
				if((d = emr64(fdeg, s-1-w)) == EMbupkis)
					d = 0;
				emw64(fdeg, s-1-w, ++d);
				if((d = emr64(fdeg, t-1-w)) == EMbupkis)
					d = 0;
				emw64(fdeg, t-1-w, ++d);
				insert(s-1-w, fs2j, fjump, fedge2, fdeg, M);
				M++;
			}
		}
		emclose(fs2j);
		emclose(fjump);
		emclose(fweight);
		emclose(fnode);
		emclose(fdeg);
		fweight = fweight2;
		emclose(fedge);
		fedge = fedge2;
		endlevel(lp);
		y = w + 1;
		w = S;
	}
	DPRINT(Debugcoarse, "coarsen: ended at level %lld", dylen(lvl));
	if(M > 0){
		DPRINT(Debugcoarse, "stopping with %lld remaining edges", M);
		printtab(fedge, M);
	}
	/* add articial root node for all remaining */
	if(S > w + 1){
		s = ++S;
		DPRINT(Debugcoarse, "push artificial root node %llx", s);
		lvl = newlevel(lvl);
		lp = lvl + dylen(lvl) - 1;
		/* every node is an adjacency, every edge is internal */
		for(e=0, u=-1; e<M; e++){
			if((o = emr64(fedge, 2 + e*2)) != u){
				u = o;
				w = emr64(fweight, u);
				assert(o != EMbupkis && w != EMbupkis);
				outputnode(lp, u, s, w);
			}
			v = emr64(fedge, 2+e*2+1);
			assert(v != EMbupkis);
			outputedge(lp, u, v);
		}
	}
	emclose(fedge);
	emclose(fweight);
	return lvl;
}

static void
usage(void)
{
	DPRINT(Debugcoarse, "usage: %s [-D which] SORTED_EDGES", argv0);
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
			DPRINT(Debugcoarse, "unknown debug component %s", s);
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
