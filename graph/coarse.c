#include "strpg.h"
#include "lib/khashl.h"
#include "threads.h"
#include "cmd.h"
#include "graph.h"
#include "layout.h"
#include "drw.h"
#include "fs.h"
#include "coarse.h"

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
KHASHL_MAP_INIT(KH_LOCAL, degmap, dg, ioff, uint, kh_hash_uint32, kh_eq_generic)

static int ncoarsed;
static ioff *expnodes;

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

	if(cnodes == nil)
		return id;
	if(id < 0 || id >= nnodes){
		werrstr("out of bounds cnode id: %d > %d", id, nnodes-1);
		return -1;
	}
	idx = cnodes[id].idx;
	if(idx >= dylen(nodes))
		panic("out of bounds node idx: %d > %d", idx, dylen(nodes)-1);
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

	if(cnodes == nil){	/* FIXME: build it? */
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

	/* FIXME
	if((debug & Debugcoarse) == 0)
		return;
	*/
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

	n = U->length;
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
	if(cnodes == nil){
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
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	TIME("uncoarsen", "wait for green light", t);
	dyresize(nodes, nn + nnew);
	memcpy(nodes + nn, unew, nnew * sizeof *nodes);
	free(unew);
	for(off=0, d=deg, u=nodes, ue=u+nn+nnew; u<ue; u++){
		u->eoff = off;
		off += *d++;
		u->nedges = 0;	/* for regenedges */
		U = cnodes + u->id;
		if(U->length > 0)
			setnodelength(u, sublength(U));
		if(u >= nodes + nn)
			spawn(cnodes[U->parent].idx);
	}
	assert(dylen(nodes) == dylen(rnodes));
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

/* this should be generic enough to allow the use of any coarsening
 * method but just update the internal data structures */
int
coarsen(void)
{
	ioff x, j, off, ne, ne2, *e, *ee, *deg, *d;
	vlong t;
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
		DPRINT(Debugcoarse, "current graph: %d nodes, %d edges", nnodes, nedges);
	}else{
		DPRINT(Debugcoarse, "current graph: %d/%d (%d) nodes, %d (%d/%d) edges",
			dylen(rnodes), nnodes, dylen(nodes), dylen(redges), dylen(edges), nedges);
	}
	t = μsec();
	if((eset = es_init()) == nil)
		sysfatal("coarsen: %s", error());
	DPRINT(Debugcoarse, "assigning new top node slots...");
	for(off=0, r=rp=rnodes, u=nodes, ue=u+dylen(u); u<ue; u++, r++){
		if(u->flags & FNalias){
			DPRINT(Debugcoarse, "> skipping aliased rnode %zd (→%d)", u-nodes, u->id);
			continue;
		}
		DPRINT(Debugcoarse, "> store node[%03d] = %d", off, u->id);
		U = cnodes + u->id;
		U->idx = off++;
		*rp = *r;
		rp++;
	}
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
	TIME("coarsen", "recompute offsets", t);
	while((drawing.flags & DFiwasfrozentoday) != DFiwasfrozentoday)
		lsleep(1000);
	TIME("coarsen", "wait for green light", t);
	x = rp - rnodes;
	assert(x == dylen(nodes) - ncoarsed);
	dyresize(rnodes, x);
	dyresize(nodes, x);
	dyresize(edges, ne2);
	dyresize(redges, ne);
	TIME("coarsen", "shrink arrays", t);
	regenedges(eset, ne, ne2);
	TIME("coarsen", "restore edges", t);
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
commit(void)
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
		logmsg("collapse/expand: no effect.\n");
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
	Node *u, *p;

	if(U->idx == -1)
		return 0;
	u = nodes + U->idx;
	if(u->flags & FNalias)
		return 0;
	u->flags |= FNalias;
	assert(U != P);
	assert(P->idx != -1);
	p = nodes + P->idx;
	setnodelength(p, p->length + u->length);
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
collapseup(ioff *ids)
{
	int j, r;
	ioff m, n, k, l, i, *p, *pp, *pe;
	CNode *U, *V;

	n = dylen(nodes);
	l = 0.5 * n;
	m = dylen(ids);
	//l = 0.5 * m;
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
		DPRINT(Debugcoarse, "collapseup: round %d: remain %d/%d ratio %.2f thresh %d queued %d",
			r, n, dylen(nodes), (double)n/(pe-ids), l, m);
		r++;
	}
	return 0;
}

ioff *
collapseall(void)
{
	ioff *ids;
	Node *u, *ue;

	ids = nil;
	assert(cnodes != nil);
	for(u=nodes, ue=u+dylen(nodes); u<ue; u++)
		dypush(ids, u->id);
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
int
initcoarse(void)
{
	ioff i;
	ssize n;
	CNode *U, *E;
	Node *u;

	if(cnodes != nil){
		werrstr("coarsening already initialized");
		return -1;
	}
	nnodes = dylen(nodes);
	n = nnodes;
	cnodes = emalloc(n * sizeof *cnodes);
	for(i=0, u=nodes, U=cnodes, E=U+n; U<E; U++, u++, i++){
		U->idx = i;
		U->eoff = u->eoff;
		U->nedges = u->nedges;
		U->parent = U->child = U->sibling = -1;
		U->length = u->length;	/* can be 0 if unset */
	}
	nedges = dylen(edges);
	n = nedges * sizeof *edges;
	cedges = emalloc(n);
	memcpy(cedges, edges, n);
	return 0;
}

/* FIXME: split up + benchmark vs. qsort */
static Adj *
sortedges(Adj **adjp, edgeset *eset, degmap *deg, int type)
{
	uint d;
	ioff i, j, nn, n, m, w, o, *op, *off, *offof, *e, *ee, *ep, *tp, *te, *totals;
	ssize sz;
	u64int uv;
	vlong t;
	Adj *adj, *adje, *a;
	CNode *U, *UE;
	khint_t k, kk;

	USED(type);
	nn = 0;
	totals = nil;	/* degree d frequency count */
	t = μsec();
	if(deg == nil){
		for(U=cnodes, UE=U+nnodes; U<UE; U++){
			d = U->nedges;
			dygrow(totals, d);
			totals[d]++;
		}
		nn = nnodes;
	}else{
		kh_foreach(deg, k){
			U = cnodes + kh_key(deg, k);
			U->flags &= ~FCNvisited;
			d = kh_val(deg, k);
			dygrow(totals, d);
			totals[d]++;
			nn++;
		}
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
		//warn("totals[%zd] %d, sum %d (%d)\n", tp-totals, w, n, m);
	}
	TIME("sortedges", "offset computation", t);
	sz = nn * sizeof *adj + m * sizeof adj->u;
	adj = emalloc(sz);
	adje = (Adj *)((uchar *)adj + sz);
	if(eset == nil){
		for(i=0, U=cnodes, UE=U+nnodes; U<UE; U++, i++){
			d = U->nedges;
			a = (Adj *)((ioff *)(adj + totals[d]) + off[d]);
			//warn("d=%d id=%d total %d off %d p %zd ", d, i, totals[d], off[d], (uchar*)a-(uchar*)adj);
			off[d] += d;
			totals[d]++;
			a->u = i;
			a->deg = d;
			for(ep=a->adj, e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
				//warn(" → [%zd] %zd", (uchar *)ep-(uchar *)adj, e-cedges+U->eoff);
				*ep++ = *e >> 2;
			}
			//warn("\n");
		}
		//for(i=0, a=adj; a<adje; a=(Adj*)e, i++){
		//	warn("adj[%d] off=%zd id=%d d=%d\n",
		//		i, (uchar*)a-(uchar*)adj, a->u, a->deg);
		//	e = a->adj + a->deg;
		//}
	}else{
		/* FIXME: SLOW */
		offof = emalloc(nnodes * sizeof *offof);
		kh_foreach(eset, kk){
			uv = kh_key(eset, kk);
			i = uv >> 32;
			j = uv & 0x7fffffff;
			i = top(cnodes + i) - cnodes;	/* FIXME: should be always the same */
			j = top(cnodes + j) - cnodes;
			if(i == j)
				continue;
			//warn("uv %d,%d: i=%d", i, j, i);
			k = dg_get(deg, i);
			assert(k != kh_end(deg));
			d = kh_val(deg, k);
			o = off[d];
			//warn(" d=%d off[d]=%d", d, o);
			if(offof[i] == 0){	/* avoid preinitializing by incrementing... */
				a = (Adj *)((ioff *)(adj + totals[d]) + o);
				off[d] += d;
				totals[d]++;
				offof[i] = (uchar *)a - (uchar *)adj + 1;
				a->u = i;
			}else
				a = (Adj *)((uchar *)adj + offof[i] - 1);
			a->adj[a->deg++] = j;
			//warn(" :: j=%d", j);
			k = dg_get(deg, j);
			assert(k != kh_end(deg));
			d = kh_val(deg, k);
			o = off[d];
			//warn(" d=%d off[d]=%d\n", d, o);
			if(offof[j] == 0){
				a = (Adj *)((ioff *)(adj + totals[d]) + o);
				off[d] += d;
				totals[d]++;
				offof[j] = (uchar *)a - (uchar *)adj + 1;
				a->u = j;
			}else
				a = (Adj *)((uchar *)adj + offof[j] - 1);
		}
		free(offof);
		//for(i=0, a=adj; a<adje; a=(Adj*)e, i++){
		//	warn("adj[%d] off=%zd id=%d d=%d\n",
		//		i, (uchar*)a-(uchar*)adj, a->u, a->deg);
		//	e = a->adj + a->deg;
		//}
	}
	TIME("sortedges", "edge placement", t);
	dyfree(totals);
	free(off);
	*adjp = adj;
	return adje;
}

int
buildct(void)
{
	int abs;
	uint d;
	u64int uv;
	vlong t;
	ioff i, j, *e, *ee;
	edgeset *eset;
	degmap *deg;
	Adj *adj, *adje, *a;
	CNode *U, *UE, *V, *C;
	khint_t k;

	if(status & FSlockedctab){
		if(cnodes != nil)
			return 0;
		warn("FIXME buildct: collapse request before ctab loaded\n");
		werrstr("ctab not yet ready");
		return -1;
	}else
		status |= FSlockedctab;
	t = μsec();
	if(initcoarse() < 0)
		return -1;
	TIME("buildct", "initcoarse", t);
	adje = sortedges(&adj, nil, nil, Sascdeg);	/* FIXME: weird api */
	TIME("buildct", "sortedges", t);
	deg = nil;
	eset = nil;
	while(adje > adj){
		if(deg != nil){
			dg_clear(deg);
			es_clear(eset);
		}else{
			if((deg = dg_init()) == nil
			|| (eset = es_init()) == nil)
				sysfatal("buildct: %s", error());
		}
		for(a=adj; a<adje; a=(Adj*)e){
			i = a->u;
			U = cnodes + i;
			//warn("node %d d=%d %d %d %d ", i, a->deg, U->parent, U->child, U->sibling);
			U = top(U);
			i = U - cnodes;
			//warn("⇒ [%zd] %d %d %d\n", i, U->parent, U->child, U->sibling);
			C = nil;
			for(e=a->adj, ee=e+a->deg; e<ee; e++){
				j = *e;
				V = cnodes + j;
				//warn("adj %d %d %d %d ", j, V->parent, V->child, V->sibling);
				if((V = top(V)) == U){
					//warn("⇒ %zd %d %d %d = U\n", V-cnodes, V->parent, V->child, V->sibling);
					continue;
				}
				j = V - cnodes;
				//warn("⇒ %d %d %d %d visited %d\n", j, V->parent, V->child, V->sibling, V->flags & FCNvisited);
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
				uv = (uvlong)i << 32 | j;
				es_put(eset, uv, &abs);
				/* FIXME: not the actual degree count */
				if(abs && V->parent != i){
					k = dg_get(deg, i);
					if(k == kh_end(deg)){
						d = 1;
						k = dg_put(deg, i, &abs);
					}else
						d = kh_val(deg, k) + 1;
					//warn("\tadd %d,%d d=%d\n", i, j, d);
					kh_val(deg, k) = d;
				}
			}
			U->flags |= FCNvisited;	/* leave alone until next iteration */
		}
		TIME("buildct", "round", t);
		free(adj);
		adje = sortedges(&adj, eset, deg, Sascdeg);
		TIME("buildct", "sortedges", t);
	}
	es_destroy(eset);
	dg_destroy(deg);
	free(adj);
	for(U=cnodes, UE=U+nnodes; U<UE; U++)	/* FIXME */
		U->flags &= ~FCNvisited;
	TIME("buildct", "cleanup", t);
	return 0;
}
