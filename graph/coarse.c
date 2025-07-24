#include "strpg.h"
#include "lib/khashl.h"
#include "threads.h"
#include "cmd.h"
#include "graph.h"
#include "layout.h"
#include "drw.h"

KHASHL_SET_INIT(KH_LOCAL, edgeset, es, u64int, kh_hash_uint64, kh_eq_generic)

enum{
	FCNvisited = 1<<0,
};
/* FIXME: some of the properties of the nodes would be nice to keep as read-
 * only for recovery or undoing changes; but a lot of those could just be
 * kept in the gfa and re-read as well, that's relatively cheap and can be
 * done asynchronously already */
typedef struct CNode CNode;
struct CNode{
	ioff idx;		/* correspondence in visible nodes[] */
	ioff eoff;
	ioff nedges;
	ioff parent;
	ioff child;
	ioff sibling;
	uchar flags;	/* FIXME */
	int length;		/* LN[id] value before coarsening */	/* FIXME: uint? */
};
static CNode *cnodes;	/* immutable */
static ioff *cedges;	/* immutable */
static ioff nnodes, nedges, ncoarsed;
static ioff *expnodes;
static u64int *crapges;

enum{
	Srandom,
	Sascdeg,
	Sdscdeg,
};

/* FIXME: length, color (still in awk), number of nodes, etc. */
/* FIXME: have to clear and/or fetch attributes and other struct fields,
 * and simultaneously update rnodes (also, parents: length, etc.);
 * draw lengths probably have to be recomputed;
 * color, length, number of nodes, etc. (but should still be able to dig up
 * a collapsed node) */

/* FIXME */
/* not thread-safe */
ioff
getnodeidx(ioff id)
{
	ioff idx;

	if(cnodes == nil)
		return id;
	if(id < 0 || id >= nnodes){
		werrstr("out of bounds cnode id: %d > %d", id, nnodes-1);
		return -1;
	}
	idx = cnodes[id].idx;
	if(idx >= dylen(nodes)){
		warn("out of bounds node idx: %d > %d", idx, dylen(nodes)-1);
		abort();
	}
	return idx;
}

static void
printcbranch(ioff i, ioff p)
{
	ioff j;
	CNode *U;

	for(j=i; j!=-1; j=U->sibling){
		U = cnodes + j;
		if(j == i)
			warn("%d→%d", p, j);
		else
			warn(":%d", j);
	}
	if(j == i)
		return;
	warn(" ");
	for(j=i; j!=-1; j=U->sibling){
		U = cnodes + j;
		printcbranch(U->child, j);
	}
}

static void
printctree(void)
{
	ioff id;
	Node *u, *ue;
	CNode *U;

	if((debug & Debugcoarse) == 0)
		return;
	warn("corresponding tree:\n");
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++){
		id = u->id;
		if(u->flags & FNalias)
			continue;
		U = cnodes + id;
		if(U->parent == -1 && U->idx != -1){
			warn("[%d> ", id);
			printcbranch(U->child, id);
			warn("\n");
		}
	}
}

static inline void
checkcnode(CNode *U)
{
	DPRINT(Debugcoarse, "checkcnode: id=%zd %d %d %d idx=%d",
		U - cnodes, U->parent, U->child, U->sibling, U->idx);
	if(U->flags & FCNvisited){
		DPRINT(Debugcoarse, "check: cycle detected or flags dirty id=%zd idx=%d",
			U - cnodes, U->idx);
		abort();
	}
	U->flags |= FCNvisited;
}

/* FIXME: only useful when the ct is modified */
static void
checkbranch(CNode *U)
{
	int j;
	CNode *V;

	checkcnode(U);
	if((j = U->child) != -1)
		checkbranch(cnodes + j);
	for(j=U->sibling; j!=-1; j=V->sibling){
		V = cnodes + j;
		if(V->parent != U->parent){
			warn("check: broken parent link of %d to %d not %zd\n",
				j, V->parent, U->parent);
			abort();
		}
		checkcnode(V);
		if((j = V->child) != -1)
			checkbranch(cnodes + j);
	}
}

static void
checkctree(void)
{
	ioff i;
	CNode *U, *UE, *V;
	Node *u;

	for(U=cnodes, UE=U+nnodes; U<UE; U++){
		if(U->idx == -1 || U->parent != -1)
			continue;
		checkbranch(U);
	}
	for(i=0, U=cnodes; U<UE; U++, i++){
		if((U->flags & FCNvisited) == 0){
			/* FIXME: warn + abort wrapper (not fatal? debug!=0 only?) */
			warn("check: unvisited cnode %zd %d %d %d idx=%d\n",
				i, U->parent, U->child, U->sibling, U->idx);
			abort();
		}
		U->flags &= ~FCNvisited;
		if(U->parent != -1){
			V = cnodes + U->parent;
			if(V->child == -1){
				warn("check: broken parent %zd of %zd, no children\n",
					U->parent, i);
				abort();
			}
			V = cnodes + V->child;
			while(V != U){
				V = cnodes + V->sibling;
				if(V->sibling == -1)
					break;
			}
			if(V != U){
				warn("check: broken child list of %d missing child %d\n",
					U->parent, i);
				abort();
			}
		}
		if(U->idx != -1){
			if(U->idx < 0 || U->idx >= dylen(nodes)){
				warn("check: %d node index %d out of range (0-%d)\n",
					i, U->idx, dylen(nodes));
				abort();
			}
			u = nodes + U->idx;
			if(u->id != i){
				warn("check: broken node link %d not %d idx=%d\n",
					u->id, i, U->idx);
				abort();
			}
		}
	}
}

static void
checktree(void)
{
	Node *u, *ue;
	CNode *U;

	/* FIXME
	if((debug & Debugcoarse) == 0)
		return;
	*/
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++){
		U = cnodes + u->id;
		if(u - nodes != U->idx){
			warn("check: [0] node %zd ≠ idx %d of cnode %d\n",
				u-nodes, U->idx, U-cnodes);
			abort();
		}
	}
	checkctree();
}

static inline CNode *
activetop(CNode *U)
{
	while(U->parent != -1){
		if(U->idx != -1)
			break;
		U = cnodes + U->parent;
	}
	assert(U->parent != -1 || U->idx != -1);
	return U;
}

static inline int
newedge(ioff i, ioff j, ioff e, edgeset *eset)
{
	int abs;
	u64int id;

	/* FIXME: special case: reject new -+ or +- self-edges, which are
	 * self-loops at either end of the node; at the top we won't be
	 * restoring all edges though, maybe have a flag or check if it's
	 * fully expanded now */
	if(i == j && (e == 2 || e == 1))
		return 0;
	if(i > j || i == j && e & 1)	/* see fs/gfa.c:/^readedge */
		id = (u64int)j << 32 | i << 2 | (e >> 1 | e << 1 & 2) ^ 3;
	else
		id = (u64int)i << 32 | j << 2 | e;
	es_put(eset, id, &abs);
	return abs != 0;
}

static inline void
spawn(ioff p)
{
	RNode r;

	/* FIXME: might need more jitter */
	r = rnodes[p];
	r.pos[0] += 0.5 - xfrand();
	r.pos[1] += 0.5 - xfrand();
	if(drawing.flags & DF3d)
		r.pos[2] += 0.5 - xfrand();	/* FIXME: ? */
	dypush(rnodes, r);
	DPRINT(Debugcoarse, "spawn rnode at %f,%f,%f from parent idx=%d",
		r.pos[0], r.pos[1], r.pos[2], p);
}

static void
regenedges(edgeset *eset, ioff nedge, ioff nadj)
{
	ioff x, n, m;
	Node *u, *v;
	khint_t k;
	u64int uv;

	DPRINT(Debugcoarse, "regenerating edges...");
	n = m = 0;
	kh_foreach(eset, k){
		n++;
		m++;
		uv = kh_key(eset, k);
		x = uv & 0xffffffff;
		u = nodes + (uv >> 32 & 0x7fffffff);
		v = nodes + (uint)(uv >> 2 & 0x3fffffff);
		DPRINT(Debugcoarse, "> edge[%d/%d]: %d%c,%d%c → %zd,%zd [%#08x]", n, m, u->id, x&1?'-':'+', v->id, x&2?'-':'+', u-nodes, v-nodes, uv);
		if(u < nodes || u >= nodes+dylen(nodes)){
			warn("oob: u %#p %d/%d\n", u, u-nodes, dylen(nodes));
			abort();
		}
		assert((u->flags & FNalias) == 0);
		u->flags &= ~FNalias;
		edges[u->eoff+u->nedges++] = x;
		if(u == v)	/* the mirror is what we just added */
			continue;
		n++;
		if(v < nodes || v >= nodes+dylen(nodes)){
			warn("oob: v %#p %d/%d\n", v, v-nodes, dylen(nodes));
			abort();
		}
		assert((v->flags & FNalias) == 0);
		v->flags &= ~FNalias;
		x = uv >> 30 & 0xfffffffcULL | (uv >> 1 & 1 | uv << 1 & 2) ^ 3;
		edges[v->eoff+v->nedges++] = x;
	}
	DPRINT(Debugcoarse, "generated %d unique edges, %d adjacencies; expected %d and %d", m, n, nedge, nadj);
	printgraph();
	assert(n == nadj && m == nedge);
}

#define	TIME(lab)	if(debug & Debugperf) do{ \
	t = μsec(); \
	warn("uncoarsen: " lab ": %.2f ms\n", (t - t0)/1000); \
	t0 = t; \
}while(0)

/* FIXME: we should just change how this is handled to begin with,
 * eg. cnode length or w/e should be the reference we use here */
/* FIXME: maybe we just need to re-fetch this data in the gfa...? */
int
setclength(Node *u, int len)
{
	CNode *U;

	if(len <= 0){
		werrstr("setclength: invalid length");
		return -1;
	}
	u->length = len;	/* FIXME: what if coarsened? only if equal to 0? */
	if(cnodes == nil)
		return 0;
	U = cnodes + u->id;
	assert(U->idx != -1 && U->idx == u - nodes);
	U->length = len;
	return 0;
}

/* FIXME: too much recursive shit */
static inline int
sublength(CNode *U)
{
	int n;
	ioff i;

	n = U->length;
	//if(U->idx != -1)
	//	return 0;

	for(i=U->child; i!=-1; i=U->sibling){
		U = cnodes + i;
		if(U->idx == -1)
			n += sublength(U);
	}
	return n;

	/*
	if(U->child != -1)
		n -= sublength(cnodes + U->child);
	for(i=U->sibling; i!=-1; i=U->sibling){
		U = cnodes + i;
		if(U->idx != -1)
			continue;
		n -= U->length;
		if(U->child != -1){
			V = cnodes + U->child;
			if(V->idx == -1)
				n -= sublength(V);
		}
	}
	*/
}

/* note: only resize global arrays at the very end to avoid racing given
 * our very loose safeguards */
/* FIXME: split into smaller functions (separate commit?) */
/* FIXME: use of crapges sucks, and we're still missing (or inventing) some edges;
 * makes sense given the assymetry between coarsen and uncoarsen; also
 * extra nodes? check */
int
uncoarsen(void)
{
	short *deg, *d;
	u64int *vp, *pp, *ve;
	ioff off, i, j, x, nn, nnew, *e, *ee, *ip, *ie;
	double t, t0;
	Node *u, *ue, *v, *unew;
	CNode *U, *V;
	edgeset *eset;

	if(expnodes == nil){
		werrstr("nothing to do");
		return -2;
	}
	/* FIXME: count is right, but there are duplicates and missing redges
	 * in render; also labels seem wrong */
	if(cnodes == nil){
		werrstr("no coarsening tree");
		return -1;
	}
	DPRINT(Debugcoarse, "uncoarsen: freezing draw and render...");
	freezeworld();
	DPRINT(Debugcoarse, "current graph: %d (%d) nodes, %d (%d) edges",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	t0 = μsec();
	nnew = dylen(expnodes);
	nn = dylen(nodes);
	unew = emalloc(nnew * sizeof *unew);	/* FIXME: bleh */
	TIME("alloc");
	DPRINT(Debugcoarse, "resetting node lengths...");
	drawing.length = (Range){9999.0f, 0.0f};	/* FIXME: necessary else skipped */
	/* new nodes only */
	DPRINT(Debugcoarse, "colleting new inner node and frontier edges...");
	for(u=unew, ip=expnodes, ie=ip+nnew; ip<ie; ip++, u++){
		i = *ip;
		U = cnodes + i;
		u->id = i;
		assert(U->idx == nn + ip - expnodes);
		V = cnodes + U->parent;
		assert(V->idx != -1);
		//spawn(V->idx);	/* FIXME: rnode realloc racing against layout */
	}
	dyfree(expnodes);
	TIME("recompute edge set");
	eset = es_init();
	deg = emalloc((nn + nnew) * sizeof *deg);
	for(d=deg, u=nodes, i=0; i<nn+nnew; i++, u++, d++){
		if(i == nn)
			u = unew;
		U = cnodes + u->id;
		assert(U->idx == i);
		for(e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			V = cnodes + j;
			/* FIXME: problem is here, we just put everything to hidden
			 * nodes as self-edges */
			if(V->idx == -1){
				for(j=V->parent; j!=-1; j=V->parent){
					V = cnodes + j;
					if(V->idx != -1)
						break;
				}
				//if(V == U || V->idx == -1)
				// FIXME: hidden top node? does that even make sense?
				if(V->idx == -1){
					abort();
					continue;
				}
			}
			if(V == U && i != j)	/* skip artificial self-edges in supernodes */
				continue;
			/* can't rely on order of u and v since we're looking at cedges */
			if(newedge(i, V->idx, x & 3, eset)){
				(*d)++;
				v = V->idx < nn ? nodes + V->idx : unew + V->idx - nn;
				DPRINT(Debugcoarse, "> save edge %d%c,%d%c →%d,%d (%d,%d)",
					i, x&1?'-':'+', V->idx, x&2?'-':'+',
					u->id, v->id, i, j);
				/* if it's a new edge, we must be the first to have seen it */
				if(U == V)
					continue;
				deg[V->idx]++;
			}else
				DPRINT(Debugcoarse, "> reject edge %d%c,%d%c →%d,%d (%d,%d)",
					i, x&1?'-':'+', V->idx, x&2?'-':'+',
					u->id, V->idx < nn ? nodes[V->idx].id : unew[V->idx-nn].id, U->idx, V->idx);
		}
	}
	TIME("collect frontier edges");
	/* FIXME: doesn't work across multiple rounds of collapse or expand;
	 * we're missing edges that were cut for redundancy and we don't have
	 * anything to find out what they were afterwards; we don't want to
	 * work backwards here because there isn't a direct correspondence
	 * between collapse and expand; instead we need a way to find all edges
	 *	keys:
	 *	- new edges will only be because of newly expanded nodes and their
	 *		parents
	 *	- direct adjacencies to new nodes are not a problem; intermediate
	 *		ones that would be renamed are
	 */
	warn("uncoarsen: FIXME ignoring crapges or missing edges\n");
	for(vp=pp=crapges, ve=vp+dylen(vp); vp<ve; vp++){
		break;	// FIXME
		U = cnodes + (*vp >> 32 & 0x3fffffffULL);
		V = cnodes + ((*vp & 0xfffffffcUL) >> 2);
		x = *vp & 3;
		DPRINT(Debugcoarse, "crapedge %zd(%d),%zd(%d)", U-cnodes, U->idx, V-cnodes, V->idx);
		do{
			//assert(U->parent != -1);
			if(U->parent == -1)
				break;
			U = cnodes + U->parent;
		}while(U->idx == -1);
		do{
			//assert(V->parent != -1);
			if(V->parent == -1)
				break;
			V = cnodes + V->parent;
		}while(V->idx == -1);
		if(U->idx == -1 || V->idx == -1)
			continue;
		DPRINT(Debugcoarse, " → %d%c,%d%c", U->idx, x&1?'-':'+', V->idx, x&2?'-':'+');
		if(newedge(U->idx, V->idx, x, eset)){
			DPRINT(Debugcoarse, "added.");
			deg[U->idx]++;
			if(U != V)
				deg[V->idx]++;
			*pp++ = *vp;
		}else
			DPRINT(Debugcoarse, "skipped.");
	}
	x = pp - crapges;
	dyresize(crapges, x);
	DPRINT(Debugcoarse, "kept %d/%d crap edges", x, dylen(crapges));
	TIME("check crap edges");
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	TIME("wait for green light");
	dyresize(nodes, nn + nnew);
	memcpy(nodes + nn, unew, nnew * sizeof *nodes);
	free(unew);
	for(off=0, d=deg, u=nodes, ue=u+nn+nnew; u<ue; u++){
		u->eoff = off;
		off += *d++;
		u->nedges = 0;	/* for regenedges */
		u->length = sublength(cnodes + u->id);	/* FIXME */
		if(u->length <= 0){
			warn("uncoarsen: %d idx=%zd length %d <= 0\n", u->id, u-nodes, u->length);
			abort();
		}
		if(u >= nodes + nn)
			spawn(cnodes[cnodes[u->id].parent].idx);
		if(setnodelength(u - nodes) < 0)
			warn("uncoarsen: setnodelength %zd: %s\n", u - nodes, error());
	}
	assert(dylen(nodes) == dylen(rnodes));
	free(deg);
	TIME("regenerate offsets and rnodes");
	dyresize(edges, off);
	x = kh_size(eset);
	dyresize(redges, x);
	TIME("resizing arrays");
	regenedges(eset, x, off);
	es_destroy(eset);
	TIME("regenerating edges");
	drawing.fcoarse = (float)nnodes / dylen(nodes);
	resizenodes();
	TIME("reset node lengths");
	thawworld();
	/* FIXME: logmsg */
	DPRINT(Debugcoarse, "uncoarsen: done.");
	//DPRINT(Debugcoarse, "current graph: %d (%d) nodes, %d (%d) edges",
	//	dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	warn("graph after expansion: %d (%d) nodes, %d (%d) edges\n",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	printgraph();
	checktree();
	return 0;
}

#undef TIME

int
expand(ioff i)
{
	ioff j, n, nid;
	CNode *U, *V;

	if(cnodes == nil){
		werrstr("no coarsening tree");
		return -1;
	}else if(i < 0 || i >= nnodes){
		werrstr("out of bounds %d > %d", i, nnodes);
		return -1;
	}else if((U = cnodes + i)->child == -1){
		DPRINT(Debugcoarse, "expand: %d not a parent", i);
		return 0;
	}
	nid = dylen(nodes) + dylen(expnodes);
	if(U->idx == -1){
		for(V=nil, j=V->parent; j!=-1; j=V->parent){	/* check first */
			V = cnodes + j;
			if(V->idx != -1)
				break;
		}
		if(V == nil || V->idx == -1){
			werrstr("%d has no visible parent??", i);
			return -1;
		}
		for(j=i;; j=V->parent){		/* push U and hidden parents */
			V = cnodes + j;
			if(V->idx != -1)
				break;
			V->idx = nid++;
			DPRINT(Debugcoarse, "expand: %d: push self and parent: %d idx=%d", i, j, V->idx);
			dypush(expnodes, j);
		}
	}
	for(n=0, j=U->child; j!=-1; j=V->sibling){
		V = cnodes + j;
		if(V->idx != -1)
			continue;
		V->idx = nid + n++;	/* FIXME: thread safety? (for which other thread?) */
		DPRINT(Debugcoarse, "expand: %d: push child %d idx=%d", i, j, V->idx);
		dypush(expnodes, j);
	}
	if(n == 0){
		DPRINT(Debugcoarse, "expand: %d: no inactive children", i);
		return 0;
	}
	return 0;
}

void
expandall(void)
{
	ioff m, l, i, e;
	Node *u, *ue;

	if(cnodes == nil){
		DPRINT(Debugcoarse, "expandall: nothing to expand");
		return;
	}
	m = dylen(nodes);
	l = 1.5 * m;
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++)
		expand(u->id);
	e = dylen(expnodes);
	m += e;
	i = 0;
	while(m < l){
		for(; i<e; i++, m++)
			expand(expnodes[i]);
		e = dylen(expnodes);
		if(i == e)
			break;
	}
	DPRINT(Debugcoarse, "expandall: marked %d nodes for expansion", e);
}

#define	TIME(lab)	if(debug & Debugperf) do{ \
	t = μsec(); \
	warn("coarsen: " lab ": %.2f ms\n", (t - t0)/1000); \
	t0 = t; \
}while(0)

/* FIXME: same approach as above, just memcpy edges for nodes that aren't
 * affected and only store the diff of edges; then there's stuff we don't
 * need to keep track of anymore and can modify edges[] in place */

/* FIXME: perhaps we could merge coarsen and uncoarsen?
 * - we have a targets[] or whatever array
 * - each target can either be a node to add, or an aliased node
 */
/* this should be generic enough to allow the use of any coarsening method but just
 * update the internal data structures */
int
coarsen(void)
{
	ioff x, c, j, off, ne, ne2, *e, *ee, *deg, *d;
	double t, t0;
	edgeset *eset;
	Node *u, *ue, *v, *up;
	RNode *r, *rp;
	CNode *U, *V;

	if(ncoarsed == 0){
		werrstr("nothing to do");
		return -2;
	}
	if(cnodes == nil){
		werrstr("no coarsening tree");
		return -1;
	}
	DPRINT(Debugcoarse, "coarsen: freezing draw and render...");
	freezeworld();
	if(dylen(nodes) == nnodes && dylen(edges) == nedges){
		warn("current graph: %d/%d nodes, %d (%d/%d) edges\n",
			dylen(nodes), nnodes, dylen(redges), dylen(edges), nedges);
	}else{
		DPRINT(Debugcoarse, "current graph: %d/%d (%d) nodes, %d (%d/%d) edges",
			dylen(rnodes), nnodes, dylen(nodes), dylen(redges), dylen(edges), nedges);
	}
	t0 = μsec();
	eset = es_init();
	DPRINT(Debugcoarse, "resetting node lengths...");
	drawing.length = (Range){9999.0f, 0.0f};	/* FIXME: necessary else skipped */
	DPRINT(Debugcoarse, "assigning new top node slots...");
	for(off=0, r=rp=rnodes, u=nodes, ue=u+dylen(u); u<ue; u++, r++){
		if(u->flags & FNalias){
			DPRINT(Debugcoarse, "> skipping aliased rnode %zd (→%d)", u-nodes, u->id);
			continue;
		}
		DPRINT(Debugcoarse, "> store node[%03d] = %d", off, u->id);
		U = cnodes + u->id;
		if(setnodelength(U->idx) < 0)	/* order is important */
			warn("coarsen: setnodelength %d: %s\n", U->idx, error());
		U->idx = off++;
		*rp = *r;
		rp++;
	}
	TIME("assign slot");
	//deg = emalloc((dylen(nodes) - ncoarsed) * sizeof *deg);
	deg = emalloc(dylen(nodes) * sizeof *deg);
	DPRINT(Debugcoarse, "storing frontier edgeset...");
	for(ne=ne2=0, u=nodes; u<ue; u++){
		U = cnodes + u->id;
		c = 0;
		if(U->idx == -1){
			DPRINT(Debugcoarse, "inactive node %d: try to get parent", u->id);
			if((V = activetop(U)) == U || V->idx == -1)
				continue;
			DPRINT(Debugcoarse, "looking at %zd instead", V - cnodes);
			U = V;
			c |= 1;
		}
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			c &= ~2;
			x = *e;
			j = x >> 2;
			v = nodes + j;
			V = cnodes + v->id;
			if(U == V && (v->flags | u->flags) & FNalias){
				DPRINT(Debugcoarse, "> prune internal edge %d%c,%d%c →%d,%d",
					u-nodes, x&1?'-':'+', j, x&2?'-':'+', u->id, v->id);
				continue;
			}
			if(V->idx == -1){
				c |= 2;
				V = activetop(V);
				if(V != U && V->idx == -1){
					warn("edge %d%c,%d%c (%zd,%zd): V %d %d/%d/%d can't be inactive and an orphan\n",
						u-nodes, x&1?'-':'+', j, x&2?'-':'+',
						U-cnodes, V-cnodes, V->idx, V->parent, V->child, V->sibling);
					abort();
				}
				DPRINT(Debugcoarse, "> external edge to collapsed node %d%c,%d%c (%zd,%zd)",
					u-nodes, x&1?'-':'+', j, x&2?'-':'+', u->id, v->id);
			}
			/* FIXME */
			if(c == 3)
				dypush(crapges, (uvlong)u->id << 32 | v->id << 2 | x & 3);
			if(newedge(U->idx, V->idx, x & 3, eset)){
				deg[U->idx]++;
				ne2++;
				DPRINT(Debugcoarse, "> keep edge %d%c,%d%c; du=%d, dv=%d",
					U->idx, x&1?'-':'+', V->idx, x&2?'-':'+', deg[U->idx], deg[V->idx]);
				if(U != V){
					assert(V->idx >= 0 && V->idx < dylen(nodes));
					deg[V->idx]++;
					ne2++;
				}
				//dypush(crapges, (uvlong)u->id << 32 | v->id << 2 | x & 3);
				ne++;
			}else
				DPRINT(Debugcoarse, "> discard redundant edge %d%c,%d%c (%zd,%zd)",
					u-nodes, x&1?'-':'+', j, x&2?'-':'+', u->id, v->id);
		}
	}
	DPRINT(Debugcoarse, "kept %d/%d unique edges, /%d adj", ne, dylen(edges), ne2);
	TIME("save unique edges");
	DPRINT(Debugcoarse, "new nodes[]:");
	for(d=deg, off=0, u=up=nodes; u<ue; u++){
		if(u->flags & FNalias)
			continue;
		*up = *u;
		up->nedges = 0;
		up->eoff = off;
		DPRINT(Debugcoarse, "> node[%zd] off=%d deg=%d",
			up-nodes, up->eoff, *d);
		off += *d++;
		up++;
	}
	assert(off == ne2);
	free(deg);
	x = up - nodes;
	assert(x == dylen(nodes) - ncoarsed);
	t0 = μsec();
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	/* FIXME: still racing, can still get stuck */
	TIME("wait for green light");
	x = rp - rnodes;
	assert(x == dylen(nodes) - ncoarsed);
	dyresize(rnodes, x);
	dyresize(nodes, x);
	dyresize(edges, ne2);
	dyresize(redges, ne);
	TIME("shrink arrays");
	drawing.fcoarse = (float)nnodes / dylen(nodes);
	resizenodes();
	TIME("reset node lengths");
	regenedges(eset, ne, ne2);
	TIME("restore edges");
	thawworld();
	es_destroy(eset);
	DPRINT(Debugcoarse, "coarsen: done.");
//	DPRINT(Debugcoarse, "current graph: %d (%d) nodes, %d (%d) edges",
//		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	warn("graph after coarsening: %d (%d) nodes, %d (%d) edges\n",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	ncoarsed = 0;
	printgraph();
	checktree();
	return 0;
}

#undef TIME

/* FIXME: yuck */
int
commit(void)
{
	int r;

	if((r = uncoarsen()) < -1 && (r = coarsen()) < -1)
		return 0;
	return r;
}

static inline CNode *
top(CNode *U)
{
	while(U->parent != -1)
		U = cnodes + U->parent;
	return U;
}

static inline CNode *
lastchild(CNode *U)
{
	int j;

	for(j=U->child, U=nil; j!=-1; j=U->sibling)
		U = cnodes + j;
	return U;
}

/* inactive node implies its entire subtree is inactive; we have to reset
 * lengths here because of the way we're storing it... */
static inline int
hide(CNode *U, CNode *P)
{
	Node *u;

	if(U->idx == -1)
		return 0;
	u = nodes + U->idx;
	if(u->flags & FNalias)
		return 0;
	u->flags |= FNalias;
	assert(U != P);
	assert(P->idx != -1);
	nodes[P->idx].length += u->length;
	U->idx = -1;
	u->id = P - cnodes;
	ncoarsed++;
	return 1;
}

/* FIXME: don't need to go through entire tree every time */
static inline int
hideall(CNode *U, CNode *P, int nlevels)
{
	int n;
	ioff j;

	if(nlevels-- == 0)	/* negative = no limit */
		return 0;
	DPRINT(Debugcoarse, "hideall %zd idx %d pid %zd", U - cnodes, U->idx, P - cnodes);
	if((n = hide(U, P)) == 0)
		return 0;
	if((j = U->child) != -1)
		n += hideall(cnodes + j, U, nlevels);
	for(j=U->sibling; j!=-1; j=U->sibling){
		U = cnodes + j;
		if(hide(U, P)){
			n++;
			if(U->child != -1)
				n += hideall(cnodes + U->child, U, nlevels);
		}
	}
	return n;
}

static inline int
hidedescendants(CNode *U, int nlevels)
{
	int j;

	if((j = U->child) == -1)
		return 0;
	return hideall(cnodes + j, U, nlevels);
}

/* NOTE: assumption: a node's being collapsed means all of its children being
 *	hidden; a node being hidden implies its child subtree is completely hidden */
/* collapse top-down selection down to one node per component; because the ct
 * creates a hierarchy without creating new nodes, we may be selecting nodes
 * and their parents at the same time, including root nodes */
/* FIXME: might just use same 10% or w/e threshold here and do bottom up also? */
int
collapsedown(ioff *ids)
{
	ioff i, m, *p, *pe;
	CNode *U;

	/* FIXME: better: instead of only one round of coarsening, coarsen down to a
	 * threshold like 10% or sth, in which case we have to use a different
	 * function that starts from the leaves and goes up */
	m = dylen(ids);
	DPRINT(Debugcoarse, "collapse: %d/%d nodes", m, dylen(nodes));
	for(p=ids, pe=p+m; p<pe; p++){
		i = *p;
		U = cnodes + i;
		if((U->flags & FCNvisited) == 0){
			DPRINT(Debugcoarse, "collapsedown %zd idx=%d: already collapsed", i, U->idx);
			continue;
		}
		U->flags &= ~FCNvisited;
		if(U->idx == -1){
			DPRINT(Debugcoarse, "collapsedown %zd: already hidden", i);
			continue;
		}
		if(U->parent == -1 && U->child == -1 && U->nedges != 0){
			warn("FIXME collapsedown: %zd idx=%d not part of coarsening tree\n", i, U->idx);
			continue;
		}
		hidedescendants(U, -1);	/* FIXME: use threshold? argument? */
		//hidedescendants(U, 1);
	}
	return 0;
}

/* FIXME: rename collapse*() functions to something less confusing */
/* when collapsing entire graph, do it bottom-up until a threshold is reached */
int
collapseup(ioff *ids)
{
	int j, r;
	ioff m, n, k, l, i, *p, *pp, *pe;
	CNode *U, *V;

	n = dylen(nodes);
	l = 0.5 * n;
	m = dylen(ids);
	//l = 0.5 * m;
	/* FIXME: ids size can be 1 even on a large graph??? */
	DPRINT(Debugcoarse, "collapseup: %d/%d nodes, threshold %d nodes", m, dylen(nodes), l);
	r = 1;
	while(n > l && m > 0){	/* seems ok to only check once per round */
		for(p=pp=ids, pe=ids+m; p<pe; p++){
			i = *p;
			U = cnodes + i;
			if(U->parent == -1){
				DPRINT(Debugcoarse, "collapseup %d: can\'t collapse top node", i);
				continue;
			}
			/* check if collapsible */
			for(k=0, j=U->child; j!=-1; j=V->sibling){	/* FIXME: inefficient */
				V = cnodes + j;
				if(V->idx != -1){
					k++;
					break;
				}
			}
			if(k == 0){
				DPRINT(Debugcoarse, "collapseup %d: collapsible leaf node", i);
				if(hide(U, cnodes + U->parent)){	/* fails if already hidden */
					n--;
					*pp++ = U->parent;
				}
			}else
				*pp++ = i;	/* try again on next round */
		}
		m = pp - ids;
		warn("collapseup: round %d: remain %d/%d ratio %.2f thresh %d queued %d\n",
			r, n, dylen(nodes), (double)n/(pe-ids), l, m);
		r++;
	}
	return 0;
}

/* FIXME: hashset vs. flags: can we modify a khashl set while iterating
 * through it? */
static inline ioff *
getleaves(CNode *U, ioff *ids)
{
	ioff n, j;
	CNode *V;

	if(U->idx == -1)
		return ids;
	for(n=dylen(ids), j=U->child; j!=-1; j=V->sibling){
		V = cnodes + j;
		ids = getleaves(V, ids);
	}
	if(dylen(ids) == n){
		j = U - cnodes;
		DPRINT(Debugcoarse, "getleaves: adding leaf %zd", j);
		dypush(ids, j);
	}
	return ids;
}

ioff *
collapseall(void)
{
	ioff *ids;
	Node *u, *ue;
	CNode *U;

	ids = nil;
	assert(cnodes != nil);
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++){
		U = cnodes + u->id;
		if(U->parent != -1)
			continue;
		DPRINT(Debugcoarse, "collapseall: pulling from %zd, %d so far", U - cnodes, dylen(ids));
		ids = getleaves(U, ids);
	}
	DPRINT(Debugcoarse, "collapseall: pulled %d leaves out of %d nodes", dylen(ids), dylen(nodes));
	return ids;
}

/* FIXME: as is, contains redundancies: a parent may be part of the selection,
 * but for now, that's ok */
ioff *
pushcollapseop(ioff id, ioff *ids)
{
	CNode *U;
	Node *u;

	DPRINT(Debugcoarse, "collapse %d", id);
	assert(cnodes != nil);
	if(id < 0 || id >= nnodes){
		werrstr("out of bounds %d > %d", id, nnodes);
		return nil;
	}
	U = cnodes + id;
	if(U->idx == -1)			/* not visible */
		return ids;
	u = nodes + U->idx;
	if(u->nedges == 0)			/* can't collapse */
		return ids;
	if(U->flags & FCNvisited)	/* unset during processing */
		return ids;
	U->flags |= FCNvisited;		/* FIXME: poor man's hashset */
	dypush(ids, id);
	return ids;
}

/* FIXME: at what point do we use vlong instead of ioff? separate
 * functions based on nnodes? at least detect it and bail */

/* size of this node's current neighborhood; assumed used only during ct
 * construction, ie. u->id == U->idx */
static inline int
cdegree(CNode *U, ioff i)	/* external adjacencies only */
{
	int n;
	ioff j, *e, *ee;
	CNode *V;

	/* FIXME: we should really compute an edge set instead and
	 * corresponding degrees, this isn't correct */
	for(n=0, e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
		j = *e >> 2;
		V = cnodes + j;
		if(j == i)
			continue;
		V = top(V);
		if(j == i)
			continue;
		n++;
	}
	return n;
}

static int
cmp_dscdeg(const void *a, const void *b)
{
	int da, db;
	ioff i, j;

	i = *(ioff *)a;
	da = cdegree(cnodes + i, i);
	j = *(ioff *)b;
	db = cdegree(cnodes + j, j);
	return da > db ? -1 : da < db ? 1 : 0;
}
static int
cmp_ascdeg(const void *a, const void *b)
{
	int da, db;
	ioff i, j;

	i = *(ioff *)a;
	da = cdegree(cnodes + i, i);
	j = *(ioff *)b;
	db = cdegree(cnodes + j, j);
	return da < db ? -1 : da > db ? 1 : 0;
}

/* note: sum in and out degree and don't count self-edges */
static inline void
sort_dscdeg(ioff *buf, vlong n)
{
	qsort(buf, n, sizeof *buf, cmp_dscdeg);
}
static inline void
sort_ascdeg(ioff *buf, vlong n)
{
	qsort(buf, n, sizeof *buf, cmp_ascdeg);
}

static inline void
sort_fisheryates(ioff *buf, vlong n)	/* FIXME: verify */
{
	ioff t, k, *x, *p, *e;

	for(p=buf, e=p+n; p<e-1; p++, n--){
		k = xnrand(n);
		x = p + k;
		if(x == p)
			continue;
		t = *p;
		*p = *x;
		*x = t;
	}
}

/* passed uninitialized */
void
sort(ioff *buf, vlong n, int type)
{
	switch(type){
	case Srandom: sort_fisheryates(buf, n); break;
	case Sascdeg: sort_ascdeg(buf, n); break;
	case Sdscdeg: sort_dscdeg(buf, n); break;
	default: sysfatal("sort: unknown type %d", type);
	}
}

/* FIXME: avoid the duplication of these buffers from fs */
static void
init(void)
{
	ioff i;
	ssize n;
	CNode *U, *E;
	Node *u;

	nnodes = dylen(nodes);
	n = nnodes;
	cnodes = emalloc(n * sizeof *cnodes);
	for(i=0, u=nodes, U=cnodes, E=U+n; U<E; U++, u++, i++){
		U->idx = i;
		U->eoff = u->eoff;
		U->nedges = u->nedges;
		U->parent = U->child = U->sibling = -1;
		/* FIXME: can get out of sync if loaded from setattr */
		if((U->length = u->length) == 0.0f)	/* FIXME */
			U->length = 1.0f;
	}
	nedges = dylen(edges);
	n = nedges * sizeof *edges;
	cedges = emalloc(n);
	memcpy(cedges, edges, n);
}

/* FIXME: seems to me it would be better to push neighbors than ourselves,
 *  to a new array; we're doing a lot of extra redundant work otherwise; the
 *  problem then is that we won't merge top nodes: maybe merge down first then
 *  merge tops after? but that's equivalent to what we're doing here */
int
buildct(void)
{
	int n, r;
	ioff i, j, m, *e, *ee, *p, *pe, *pp, *ids;
	CNode *U, *UE, *V, *W, *T;

	if(cnodes != nil)
		return 0;
	init();
	m = nnodes;
	ids = emalloc(m * sizeof *ids);
	for(i=0, p=ids, pe=p+m; p<pe; p++)	/* FIXME */
		*p = i++;
	sort(ids, m, Sascdeg);
	for(r=0; m>0; r++){
		DPRINT(Debugcoarse, "buildct round %d: %d/%d remaining",
			r+1, m, nnodes);
		for(p=pp=ids, pe=p+m; p<pe; p++){
			i = *p;
			U = cnodes + i;
			DPRINT(Debugcoarse, "buildct: node %d idx=%d %d %d %d f=%x",
				i, U->idx, U->parent, U->child, U->sibling, U->flags);
			assert((U->flags & FCNvisited) == 0);
			U->flags |= FCNvisited;
			T = nil;
			for(n=0, e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
				j = (*e >> 2);
				V = cnodes + j;
				DPRINT(Debugcoarse, "buildct: adj %d idx=%d %d %d %d",
					j, V->idx, V->parent, V->child, V->sibling);
				if(V == U){
					DPRINT(Debugcoarse, "buildct: %d: V is U, skipping", i);
					continue;
				}
				if(V->flags & FCNvisited){
					DPRINT(Debugcoarse, "buildct: skip top node");
					continue;
				}
				if(j == U->parent){
					DPRINT(Debugcoarse, "buildct: skip our parent");
					continue;
				}
				/* two nodes within the same component won't have the same
				 * top until the appropriate parent links are established */
				if(V->parent != -1){
					if(T == nil)
						T = top(U);
					if((V = top(V)) == T){
						DPRINT(Debugcoarse, "buildct: %d: top(V) is top(U), skipping", i);
						continue;
					}
					j = V - cnodes;
					assert(V->parent == -1);
					DPRINT(Debugcoarse, "buildct: grabbing subtree up to %d", j);
				}
				DPRINT(Debugcoarse, "buildct: %d has new child %d", i, j);
				V->parent = i;
				if(n == 0)
					W = lastchild(U);
				if(W == nil)
					U->child = j;
				else
					W->sibling = j;
				W = V;
				n++;
			}
			if(n > 0)
				*pp++ = i;
		}
		m = pp - ids;
		for(p=ids, pe=p+m; p<pe; p++)	/* FIXME */
			cnodes[*p].flags &= ~FCNvisited;
		/* FIXME: are we computing degree correctly? ← no. after first
		 * round we no longer have a good degree count; also we go through
		 * all nodes, then reevaluate those that successfully nabbed a
		 * neighbor, is that what we want? */
		/* FIXME: what is even degree in this context anymore? define it
		 * first */
		if(m > 1)
			sort(ids, m, Sascdeg);
	}
	free(ids);
	for(U=cnodes, UE=U+nnodes; U<UE; U++)	/* FIXME */
		U->flags &= ~FCNvisited;
	checkctree();
	printctree();
	return 0;
}
