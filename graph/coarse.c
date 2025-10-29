#include "strpg.h"
#include "lib/khashl.h"
#include "threads.h"
#include "cmd.h"
#include "graph.h"
#include "layout.h"
#include "drw.h"
#include "fs.h"
#include "coarse.h"
#include "strawk/awk.h"
#include "var.h"

CNode *cnodes;
ioff *cedges;
ioff nnodes, nedges;

typedef struct Adj Adj;
struct Adj{
	ioff u;
	uint deg;
	ioff adj[];
};

KHASHL_SET_INIT(KH_LOCAL, edgeset, es, u64int, kh_hash_uint64, kh_eq_generic)

static int ncoarsed;
static ioff *expnodes;
static QLock tablock;

enum{
	Srandom,
	Sascdeg,
	Sdscdeg,
};

/* FIXME */
/* not thread-safe */
ioff
getnodeidx(ioff id)
{
	ioff idx;

	if((graph.flags & GFctarmed) == 0)
		return id;
	if(id < 0 || id >= nnodes){
		werrstr("out of bounds cnode id: %d > %d", id, nnodes-1);
		return -1;
	}
	/* FIXME: fugly */
	if(!canqlock(&tablock)){
		werrstr("coarsening in progress");
		return -1;
	}
	idx = cnodes[id].idx;
	if(idx >= dylen(nodes))
		panic("out of bounds node idx: %d > %d", idx, dylen(nodes)-1);
	qunlock(&tablock);
	return idx;
}

int
exportct(char *path)
{
	int r;
	ioff i;
	char buf[1024], *p;
	File *fs;
	CNode *U, *UE;

	if((graph.flags & GFctarmed) == 0){	/* FIXME: build it? */
		werrstr("no tree yet");
		return -1;
	}
	if((fs = openfs(path, OWRITE)) == nil)
		return -1;
	r = -1;
	p = seprint(buf, buf + sizeof buf, "digraph {\n");
	if(writefs(fs, buf, p - buf) < 0)
		goto end;
	for(i=0, U=cnodes, UE=U+nnodes; U<UE; U++, i++){
		if(U->parent != -1){
			p = seprint(buf, buf + sizeof buf, "\t%d -> %d\n", U->parent, i);
			if(writefs(fs, buf, p - buf) < 0)
				goto end;
		}
	}
	p = seprint(buf, buf + sizeof buf, "}\n");
	r = writefs(fs, buf, p - buf);
end:
	freefs(fs);
	return r;
}

static inline void
checkcnode(CNode *U)
{
	DPRINT(Debugcoarse, "checkcnode: id=%zd %d %d %d idx=%d",
		U - cnodes, U->parent, U->child, U->sibling, U->idx);
	if(U->flags & FCNvisited)
		panic("check: cycle detected or flags dirty id=%zd idx=%d",
			U - cnodes, U->idx);
	U->flags |= FCNvisited;
}

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
		if(V->parent != U->parent)
			panic("check: broken parent link of %d to %d not %zd",
				j, V->parent, U->parent);
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
		if((U->flags & FCNvisited) == 0)
			panic("check: unvisited cnode %zd %d %d %d idx=%d",
				i, U->parent, U->child, U->sibling, U->idx);
		U->flags &= ~FCNvisited;
		if(U->parent != -1){
			V = cnodes + U->parent;
			if(V->child == -1)
				panic("check: broken parent %zd of %zd, no children",
					U->parent, i);
			V = cnodes + V->child;
			while(V != U){
				V = cnodes + V->sibling;
				if(V->sibling == -1)
					break;
			}
			if(V != U)
				panic("check: broken child list of %d missing child %d",
					U->parent, i);
		}
		if(U->idx != -1){
			if(U->idx < 0 || U->idx >= dylen(nodes))
				panic("check: %d node index %d out of range (0-%d)",
					i, U->idx, dylen(nodes));
			u = nodes + U->idx;
			if(u->id != i)
				panic("check: broken node link %d not %d idx=%d",
					u->id, i, U->idx);
		}
	}
}

static void
checktree(void)
{
	Node *u, *ue;
	CNode *U;

	if((debug & Debugcoarse) == 0)
		return;
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++){
		U = cnodes + u->id;
		if(u - nodes != U->idx)
			panic("check: [0] node %zd ≠ idx %d of cnode %d",
				u-nodes, U->idx, U-cnodes);
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
	e &= 3;
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
spawn(ioff i, ioff p)
{
	CNode *P;
	RNode r, *rp;

	if(p >= i){
		P = cnodes + cnodes[nodes[p].id].parent;
		assert(P->idx != -1);
		spawn(p, P->idx);
	}
	rp = rnodes + i;
	if(rp->pos[0] != 0.0f || rp->pos[1] != 0.0f || rp->pos[2] != 0.0f)
		return;
	/* FIXME: might need more jitter */
	r = rnodes[p];
	r.pos[0] += 0.5 - xfrand();
	r.pos[1] += 0.5 - xfrand();
	if(drawing.flags & DF3d)
		r.pos[2] += 0.5 - xfrand();	/* FIXME: ? */
	*rp = r;
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
		if(u < nodes || u >= nodes+dylen(nodes))
			panic("oob: u %#p %d/%d id %llx", u, u-nodes, dylen(nodes), uv);
		assert((u->flags & FNalias) == 0);
		edges[u->eoff+u->nedges++] = x;
		if(u == v)	/* the mirror is what we just added */
			continue;
		n++;
		if(v < nodes || v >= nodes+dylen(nodes))
			panic("oob: v %#p %d/%d id %llx", v, v-nodes, dylen(nodes), uv);
		assert((v->flags & FNalias) == 0);
		x = uv >> 30 & 0xfffffffcULL | (uv >> 1 & 1 | uv << 1 & 2) ^ 3;
		edges[v->eoff+v->nedges++] = x;
	}
	DPRINT(Debugcoarse, "generated %d unique edges, %d adjacencies; expected %d and %d", m, n, nedge, nadj);
	printgraph();
	assert(n == nadj && m == nedge);
}

/* FIXME: too much recursive shit */
static inline vlong
sublength(CNode *U)
{
	vlong n;
	ioff i;

	n = getnodelength(U - cnodes);
	for(i=U->child; i!=-1; i=U->sibling){
		U = cnodes + i;
		if(U->idx == -1)
			n += sublength(U);
	}
	return n;
}

static inline int
pushedge(ioff i, ioff j, int dir, edgeset *eset, short *deg)
{
	if(!newedge(i, j, dir, eset)){
		DPRINT(Debugcoarse, "> reject edge %d%c,%d%c",
			i, dir&1?'-':'+', j, dir&2?'-':'+');
		return 0;
	}
	DPRINT(Debugcoarse, "> save edge %d%c,%d%c",
		i, dir&1?'-':'+', j, dir&2?'-':'+');
	deg[i]++;
	if(j != i)
		deg[j]++;
	return 1;
}

/* note: only resize global arrays at the very end to avoid racing given
 * our very loose safeguards */
/* FIXME: split into smaller functions (separate commit?); separate files? */
int
uncoarsen(void)
{
	short *deg, *d;
	ioff off, i, j, x, nn, nnew, *e, *ee, *ip, *ie;
	vlong t;
	Node *u, *ue, *unew;
	CNode *U, *V, *UE;
	edgeset *eset;

	if(expnodes == nil){
		werrstr("nothing to do");
		return -2;
	}
	if((graph.flags & GFctarmed) == 0){
		werrstr("no coarsening tree");
		return -1;
	}
	DPRINT(Debugcoarse, "uncoarsen: freezing draw and render...");
	freezeworld();	/* FIXME: maybe just check for flag in draw/w/e loops */
	DPRINT(Debugcoarse, "current graph: %d (%d) nodes, %d (%d) edges",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	t = μsec();
	nnew = dylen(expnodes);
	nn = dylen(nodes);
	unew = emalloc(nnew * sizeof *unew);
	TIME("uncoarsen", "alloc", t);
	for(u=unew, ip=expnodes, ie=ip+nnew; ip<ie; ip++, u++){
		i = *ip;
		U = cnodes + i;
		u->id = i;
		assert(U->idx == nn + ip - expnodes);
		V = cnodes + U->parent;
		assert(V->idx != -1);
	}
	dyfree(expnodes);
	TIME("uncoarsen", "collecting new inner nodes", t);
	if((eset = es_init()) == nil)
		sysfatal("uncoarsen: %s", error());
	deg = emalloc((nn + nnew) * sizeof *deg);
	/* FIXME: good enough for now (1e7 edges: 500ms on p14s), correct
	 * shortcuts are hard to find; fix it later */
	for(U=cnodes, UE=U+nnodes; U<UE; U++){
		if((i = U->idx) == -1)
			i = activetop(U)->idx;
		for(e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
			x = *e;
			V = cnodes + (x >> 2);
			if(V == U)
				continue;
			if((j = V->idx) == -1)
				j = activetop(V)->idx;
			if(j == i && (U->idx == -1 || V->idx == -1))
				continue;
			pushedge(i, j, x, eset, deg);
		}
	}
	TIME("uncoarsen", "collecting inner edges", t);
	qlock(&tablock);
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	TIME("uncoarsen", "wait for green light", t);
	dyresize(nodes, nn + nnew);
	dyresize(rnodes, nn + nnew);
	memcpy(nodes + nn, unew, nnew * sizeof *nodes);
	free(unew);
	for(i=off=0, d=deg, u=nodes, ue=u+nn+nnew; u<ue; u++, i++){
		u->eoff = off;
		off += *d++;
		u->nedges = 0;	/* for regenedges */
		U = cnodes + u->id;
		if(getnodelength(u->id) > 0)
			updatenodelength(u - nodes, sublength(U));
		if(i >= nn)
			spawn(i, cnodes[U->parent].idx);
	}
	qunlock(&tablock);
	free(deg);
	TIME("uncoarsen", "regenerate offsets and rnodes", t);
	dyresize(edges, off);
	x = kh_size(eset);
	dyresize(redges, x);
	TIME("uncoarsen", "resizing arrays", t);
	regenedges(eset, x, off);
	es_destroy(eset);
	TIME("uncoarsen", "regenerating edges", t);
	thawworld();
	logmsg(va("graph after expansion: %d (%d) nodes, %d (%d) edges\n",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges)));
	printgraph();
	checktree();
	return 0;
}

int
expand(ioff i)
{
	ioff j, n, nid;
	CNode *U, *V;

	if((graph.flags & GFctarmed) == 0){
		werrstr("no coarsening tree");
		return -1;
	}else if(i < 0 || i >= nnodes){
		werrstr("out of bounds %d > %d", i, nnodes);
		return -1;
	}
	U = cnodes + i;
	nid = dylen(nodes) + dylen(expnodes);
	if(U->idx == -1){
		for(V=U, j=U->parent; j!=-1; j=V->parent){	/* check first */
			V = cnodes + j;
			if(V->idx != -1)
				break;
		}
		if(V == U || V->idx == -1){
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
		V->idx = nid + n++;
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

	if((graph.flags & GFctarmed) == 0){
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

/* this should be generic enough to allow the use of any coarsening
 * method but just update the internal data structures */
int
coarsen(void)
{
	ioff x, nn, j, off, ne, ne2, *e, *ee, *deg, *d;
	vlong t;
	edgeset *eset;
	Node *u, *ue, *v, *up;
	RNode *r, *rp;
	CNode *U, *V;

	if(ncoarsed == 0){
		werrstr("nothing to do");
		return -2;
	}
	if((graph.flags & GFctarmed) == 0){
		werrstr("no coarsening tree");
		return -1;
	}
	DPRINT(Debugcoarse, "coarsen: freezing draw and render...");
	if(graph.flags & GFarmed)
		freezeworld();
	if(dylen(nodes) == nnodes && dylen(edges) == nedges){
		DPRINT(Debugcoarse, "current graph: %d nodes, %d edges", nnodes, nedges);
	}else{
		DPRINT(Debugcoarse, "current graph: %d/%d (%d) nodes, %d (%d/%d) edges",
			dylen(rnodes), nnodes, dylen(nodes), dylen(redges), dylen(edges), nedges);
	}
	t = μsec();
	if((eset = es_init()) == nil)
		sysfatal("coarsen: %s", error());
	DPRINT(Debugcoarse, "assigning new top node slots...");
	for(off=0, u=nodes, ue=u+dylen(u); u<ue; u++){
		if(u->flags & FNalias){
			DPRINT(Debugcoarse, "> skipping aliased rnode %zd (→%d)", u-nodes, u->id);
			continue;
		}
		DPRINT(Debugcoarse, "> store node[%03d] = %d", off, u->id);
		U = cnodes + u->id;
		U->idx = off++;
	}
	nn = off;
	TIME("coarsen", "assign slot", t);
	deg = emalloc(dylen(nodes) * sizeof *deg);
	DPRINT(Debugcoarse, "storing frontier edgeset...");
	for(ne=ne2=0, u=nodes; u<ue; u++){
		U = cnodes + u->id;
		if(U->idx == -1){
			DPRINT(Debugcoarse, "inactive node %d: try to get parent", u->id);
			if((V = activetop(U)) == U || V->idx == -1)
				continue;
			DPRINT(Debugcoarse, "looking at %zd instead", V - cnodes);
			U = V;
		}
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
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
				V = activetop(V);
				if(V != U && V->idx == -1)
					panic("edge %d%c,%d%c (%zd,%zd): V %d %d/%d/%d can't be inactive and an orphan",
						u-nodes, x&1?'-':'+', j, x&2?'-':'+',
						U-cnodes, V-cnodes, V->idx, V->parent, V->child, V->sibling);
				DPRINT(Debugcoarse, "> external edge to collapsed node %d%c,%d%c (%zd,%zd)",
					u-nodes, x&1?'-':'+', j, x&2?'-':'+', u->id, v->id);
			}
			if(newedge(U->idx, V->idx, x, eset)){
				deg[U->idx]++;
				ne2++;
				DPRINT(Debugcoarse, "> keep edge %d%c,%d%c; du=%d, dv=%d",
					U->idx, x&1?'-':'+', V->idx, x&2?'-':'+', deg[U->idx], deg[V->idx]);
				if(U != V){
					assert(V->idx >= 0 && V->idx < dylen(nodes));
					deg[V->idx]++;
					ne2++;
				}
				ne++;
			}else
				DPRINT(Debugcoarse, "> discard redundant edge %d%c,%d%c (%zd,%zd)",
					u-nodes, x&1?'-':'+', j, x&2?'-':'+', u->id, v->id);
		}
	}
	DPRINT(Debugcoarse, "kept %d/%d unique edges, /%d adj", ne, dylen(edges), ne2);
	TIME("coarsen", "save unique edges", t);
	DPRINT(Debugcoarse, "new nodes[]:");
	qlock(&tablock);	/* FIXME: hacks */
	for(d=deg, off=0, r=rp=rnodes, u=up=nodes; u<ue; u++, r++){
		if(u->flags & FNalias)
			continue;
		*up = *u;
		up->nedges = 0;
		up->eoff = off;
		DPRINT(Debugcoarse, "> node[%zd] off=%d deg=%d",
			up-nodes, up->eoff, *d);
		off += *d++;
		up++;
		if(rp != nil)
			*rp++ = *r;
	}
	assert(off == ne2);
	free(deg);
	x = up - nodes;
	assert(x == dylen(nodes) - ncoarsed);
	TIME("coarsen", "recompute offsets", t);
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	TIME("coarsen", "wait for green light", t);
	assert(nn == dylen(nodes) - ncoarsed);
	dyresize(rnodes, nn);
	dyresize(nodes, nn);
	dyresize(edges, ne2);
	dyresize(redges, ne);
	qunlock(&tablock);
	TIME("coarsen", "shrink arrays", t);
	regenedges(eset, ne, ne2);
	TIME("coarsen", "restore edges", t);
	if(graph.flags & GFarmed)
		thawworld();
	es_destroy(eset);
	logmsg(va("graph after coarsening: %d (%d) nodes, %d (%d) edges\n",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges)));
	ncoarsed = 0;
	printgraph();
	checktree();
	return 0;
}

/* FIXME: logmsg or just print? */
int
commit(int quiet)
{
	int r;
	double t;
	char *name;
	int (*fn)(void);

	t = μsec();
	if(ncoarsed > 0){
		fn = coarsen;
		name = "collapse";
	}else if(expnodes != nil){
		fn = uncoarsen;
		name = "expand";
	}else{
		if(!quiet)
			logmsg("collapse/expand: no effect.\n");
		else
			DPRINT(Debugcoarse, "collapse/expand: no effect");
		return 0;
	}
	switch(r = fn()){
	case -1: return -1;
	case -2: DPRINT(Debuginfo, "%s: %s", name, error()); /* wet floor */
	default: r = 0;
	}
	t = (μsec() - t) / 1000.0;
	logmsg(va("%s: done (%.2f ms).\n", name, t));
	return 0;
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
	updatenodelength(P->idx, nodes[P->idx].length + u->length);
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
	n = hide(U, P);
	if((j = U->child) != -1)
		n += hideall(cnodes + j, P, nlevels);
	for(j=U->sibling; j!=-1; j=U->sibling){
		U = cnodes + j;
		if(hide(U, P)){
			n++;
			if(U->child != -1)
				n += hideall(cnodes + U->child, P, nlevels);
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
		hidedescendants(U, -1);
	}
	return 0;
}

/* FIXME: rename collapse*() functions to something less confusing */
/* when collapsing entire graph, do it bottom-up until a threshold is reached */
int
collapseup(ioff *ids, ssize max)
{
	int j, r;
	ioff m, n, k, i, *p, *pp, *pe;
	CNode *U, *V;

	n = dylen(nodes);
	if(max <= 0)
		max = 0.5 * n;
	m = dylen(ids);
	DPRINT(Debugcoarse, "collapseup: %d/%d nodes, threshold %d nodes", m, n, max);
	r = 1;
	while(n > max && m > 0){	/* seems ok to only check once per round */
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
		DPRINT(Debugcoarse, "collapseup: round %d: remain %d/%d ratio %.2f thresh %d queued %zd",
			r, n, dylen(nodes), (double)n/(pe-ids), max, pp - ids);
		if(m == pp - ids)
			break;
		m = pp - ids;
		r++;
	}
	return 0;
}

/* FIXME: assert? */
ioff *
collapseall(void)
{
	ioff *ids;
	Node *u, *ue;

	ids = nil;
	assert(cnodes != nil);
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++){
		if((u->flags & FNfixed) == 0)
			dypush(ids, u->id);
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

static void
collapseupto(int n)
{
	ioff *ids;

	ids = collapseall();	/* FIXME: yuck */
	if(collapseup(ids, n) < 0)
		sysfatal("collapseup: %s", error());
	dyfree(ids);
	drawing.flags |= DFiwasfrozentoday;
	switch(coarsen()){
	case -2: DPRINT(Debuginfo, "coarsen: %s", error()); break;
	case -1: sysfatal("coarsen: %s", error()); break;
	}
	drawing.flags &= ~DFiwasfrozentoday;
}

static void
reallyinitcoarse(void)
{
	voff i;
	ssize n;
	CNode *U, *UE;
	Node *u;

	for(i=0, u=nodes, U=cnodes, UE=U+nnodes; U<UE; U++, u++, i++){
		U->idx = i;
		U->eoff = u->eoff;
		U->nedges = u->nedges;
		U->flags = 0;
	}
	free(cedges);
	n = nedges * sizeof *cedges;
	cedges = emalloc(n);
	memcpy(cedges, edges, n);
	graph.flags |= GFctarmed;
}

/* FIXME: avoid the duplication of these buffers from fs */
int
initcoarse(void)
{
	voff i;
	CNode *U, *UE;
	Node *u;

	if(graph.flags & GFctarmed){
		werrstr("coarsening already initialized");
		return -1;
	}
	cnodes = emalloc(nnodes * sizeof *cnodes);
	for(i=0, u=nodes, U=cnodes, UE=U+nnodes; U<UE; U++, u++, i++){
		U->idx = i;
		U->eoff = -1;
		U->parent = U->child = U->sibling = -1;
	}
	return 0;
}

/* FIXME: ugly overloading */
static inline CNode *
top(CNode *U)
{
	CNode *T;

	T = U->eoff != -1 ? cnodes + U->eoff : U;
	while(T->parent != -1)
		T = cnodes + T->parent;
	if(T != U)
		U->eoff = T - cnodes;
	return T;
}

static inline CNode *
lastchild(CNode *U)
{
	int j;
	CNode *C;

	j = U->idx != U - cnodes ? U->idx : U->child;
	for(C=cnodes + j; j!=-1; j=C->sibling)
		C = cnodes + j;
	U->idx = C - cnodes;
	return C;
}

/* FIXME: split up */
static Adj *
sortedges(Adj **adjp, Adj *adje, edgeset **eset, ioff *offof, int type)
{
	int abs;
	uint d;
	ioff i, j, nn, n, m, w, o, *op, *off, *e, *ee, *ep, *tp, *te, *totals;
	ssize sz;
	u64int uv;
	vlong t;
	Adj *a, *adj;
	CNode *U, *V;
	Node *u, *ue;
	edgeset *es;
	khint_t kk;

	USED(type);
	nn = 0;
	totals = nil;	/* degree d frequency count */
	t = μsec();
	if(eset == nil){
		es = nil;
		for(u=nodes, ue=u+dylen(u); u<ue; u++){
			d = u->nedges;
			dygrow(totals, d);
			totals[d]++;
			nn++;
		}
	}else{
		if((es = *eset) != nil)
			es_clear(es);
		else{
			if((es = es_init()) == nil)
				sysfatal("buildct: %s", error());
			es_resize(es, nedges);
			*eset = es;
		}
		for(a=*adjp; a<adje; a=(Adj*)e){
			U = top(cnodes + a->u);
			i = U - cnodes;
			for(e=ep=a->adj, ee=e+a->deg; e<ee; e++){
				V = top(cnodes + *e);
				if(V == U)
					continue;
				j = V - cnodes;
				uv = i < j ? (uvlong)i << 32 | j : (uvlong)j << 32 | i;
				es_put(es, uv, &abs);
				if(abs){
					U->nedges++;
					V->nedges++;
				}
			}
		}
		for(a=*adjp; a<adje; a=(Adj*)(a->adj + a->deg)){
			i = a->u;
			U = cnodes + i;
			U->flags = 0;
			if((d = U->nedges) == 0)
				continue;
			offof[i] = -1;
			dygrow(totals, d);
			totals[d]++;
			nn++;
		}
		free(*adjp);
	}
	TIME("sortedges", "occurrence counting", t);
	if(nn == 0){
		*adjp = nil;
		return nil;
	}
	w = dylen(totals);
	off = emalloc(w * sizeof *off);
	for(n=m=0, d=0, op=off, tp=totals, te=tp+w; tp<te; tp++, d++, op++){
		w = *tp;
		*tp = n;		/* now cumulative number of nodes up to degree d */
		n += w;
		if(d > 0){
			*op = m;
			m += w * d;	/* total number of edges in bucket */
		}
		DPRINT(Debugcoarse, "totals[%zd] %d, sum %d (%d)", tp-totals, w, n, m);
	}
	TIME("sortedges", "offset computation", t);
	sz = nn * sizeof *adj + m * sizeof adj->u;
	adj = emalloc(sz);
	adje = (Adj *)((uchar *)adj + sz);
	if(es == nil){
		for(i=0, u=nodes, ue=u+nn; u<ue; u++, i++){
			d = u->nedges;
			a = (Adj *)((ioff *)(adj + totals[d]) + off[d]);
			DPRINTS(Debugcoarse, "d=%d id=%d total %d off %d p %zd",
				d, i, totals[d], off[d], (uchar*)a-(uchar*)adj);
			off[d] += d;
			totals[d]++;
			a->u = i;
			a->deg = d;
			for(ep=a->adj, e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
				DPRINTN(Debugcoarse, " → [%zd] %zd",
					(uchar *)ep-(uchar *)adj, e-edges+u->eoff);
				*ep++ = *e >> 2;
			}
			DPRINTN(Debugcoarse, "\n");
		}
		if(debug & Debugcoarse){
			for(i=0, a=adj; a<adje; a=(Adj*)e, i++){
				DPRINT(Debugcoarse, "adj[%d] off=%zd id=%d d=%d",
					i, (uchar*)a-(uchar*)adj, a->u, a->deg);
				e = a->adj + a->deg;
			}
		}
	}else{
		kh_foreach(es, kk){
			uv = kh_key(es, kk);
			i = uv >> 32;
			j = uv & 0xffffffffU;
			DPRINTS(Debugcoarse, "uv %d,%d: i=%d", i, j, i);
			d = cnodes[i].nedges;
			assert(d > 0);
			assert(d < dylen(totals));
			o = off[d];
			DPRINTN(Debugcoarse, " d=%d off[d]=%d", d, o);
			if(offof[i] == -1){
				a = (Adj *)((ioff *)(adj + totals[d]) + o);
				off[d] += d;
				totals[d]++;
				offof[i] = (uchar *)a - (uchar *)adj;
				a->u = i;
			}else
				a = (Adj *)((uchar *)adj + offof[i]);
			a->adj[a->deg++] = j;
			DPRINTN(Debugcoarse, " :: j=%d", j);
			d = cnodes[j].nedges;
			assert(d > 0);
			assert(d < dylen(totals));
			o = off[d];
			DPRINTN(Debugcoarse, " d=%d off[d]=%d\n", d, o);
			if(offof[j] == -1){
				a = (Adj *)((ioff *)(adj + totals[d]) + o);
				off[d] += d;
				totals[d]++;
				offof[j] = (uchar *)a - (uchar *)adj;
				a->u = j;
			}else
				a = (Adj *)((uchar *)adj + offof[j]);
			a->adj[a->deg++] = i;
		}
		if(debug & Debugcoarse){
			for(i=0, a=adj; a<adje; a=(Adj*)e, i++){
				DPRINT(Debugcoarse, "adj[%d] off=%zd id=%d d=%d",
					i, (uchar*)a-(uchar*)adj, a->u, a->deg);
				e = a->adj + a->deg;
			}
		}
	}
	TIME("sortedges", "edge placement", t);
	dyfree(totals);
	free(off);
	*adjp = adj;
	return adje;
}

static void
buildct(void *)
{
	vlong t, tt, t0;
	ioff i, j, *e, *ee, *coff;
	edgeset *eset;
	Adj *adj, *adje, *a;
	CNode *U, *V, *C;

	logmsg("building coarsening table...\n");
	t = t0 = μsec();
	if(initcoarse() < 0)
		sysfatal("buildct: %s", error());
	TIME("buildct", "initcoarse", t);
	adj = nil;
	eset = nil;
	coff = emalloc(nnodes * sizeof *coff);
	adje = sortedges(&adj, nil, nil, coff, Sascdeg);
	TIME("buildct", "sortedges", t);
	while(adje > adj){
		tt = t = μsec();
		for(a=adj; a<adje; a=(Adj*)e){
			i = a->u;
			U = cnodes + i;
			U->nedges = 0;
			DPRINTS(Debugcoarse, "node %d d=%d %d %d %d ",
				i, a->deg, U->parent, U->child, U->sibling);
			U = top(U);
			i = U - cnodes;
			DPRINTN(Debugcoarse, "⇒ [%zd] %d %d %d\n",
				i, U->parent, U->child, U->sibling);
			C = nil;
			for(e=a->adj, ee=e+a->deg; e<ee; e++){
				j = *e;
				V = cnodes + j;
				DPRINTS(Debugcoarse, "adj %d %d %d %d ",
					j, V->parent, V->child, V->sibling);
				if((V = top(V)) == U){
					DPRINTN(Debugcoarse, "⇒ %zd %d %d %d = U\n",
						V-cnodes, V->parent, V->child, V->sibling);
					continue;
				}
				j = V - cnodes;
				DPRINTN(Debugcoarse, "⇒ %d %d %d %d visited %d\n",
					j, V->parent, V->child, V->sibling, V->flags & FCNvisited);
				if((V->flags & FCNvisited) == 0){
					V->flags |= FCNvisited;	/* don't reassign this turn */
					V->parent = i;
					if(U->child == -1)
						U->child = j;
					else{
						if(C == nil)
							C = lastchild(U);
						C->sibling = j;
					}
					C = V;
				}
			}
			U->flags |= FCNvisited;	/* leave alone until next iteration */
		}
		TIME("buildct", "node merging", tt);
		adje = sortedges(&adj, adje, &eset, coff, Sascdeg);
		TIME("buildct", "sortedges", tt);
		TIME("buildct", "round", t);
	}
	es_destroy(eset);
	free(adj);
	free(coff);
	if(status & FSdontmindme){
		TIME("buildct", "cleanup", t);	/* FIXME: cleaner */
		TIME("buildct", "total", t0);
		logmsg("coarsening table built.\n");
		return;
	}
	reallyinitcoarse();
	TIME("buildct", "cleanup", t);
	TIME("buildct", "total", t0);
	graph.flags |= GFctarmed;
	logmsg("coarsening table built.\n");
	pushcmd("cmd(\"HGI234\")");
	flushcmd();
}

/* must be done after node lengths are loaded, ie. after node tags,
 * preferably on the awk thread to avoid confusion */
void
armgraph(void)
{
	if(dylen(nodes) > 10000){	/* FIXME: constant */
		logmsg("collapsing graph...\n");
		collapseupto(10000);
	}else if(rnodes == nil){
		dyresize(rnodes, nnodes);
		dyresize(redges, nedges);	/* FIXME: wrong number */
	}
	graph.flags |= GFarmed;
}

/* FIXME: assumes single graph loaded; instead of calling this in
 * fs/gfa, signal awk and have it start this if no other graph files
 * are to be loaded */
void
initct(void)
{
	if(status & FSlockedctab)	/* FIXME: rename: FSctabloaded */
		return;
	status |= FSlockedctab;
	if((status & FSdontmindme) == 0)
		newthread(buildct, nil, nil, nil, "buildct", mainstacksize);
	else
		buildct(nil);
}
