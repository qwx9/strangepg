#include "strpg.h"
#include "lib/khashl.h"
#include "threads.h"
#include "graph.h"
#include "layout.h"
#include "drw.h"

KHASHL_SET_INIT(KH_LOCAL, edgeset, es, u64int, kh_hash_uint64, kh_eq_generic)

typedef struct CNode CNode;

enum{
	CFinactive = 1<<0,
};
struct CNode{
	ioff idx;	/* correspondence in visible nodes[] */
	char flags;
	ioff eoff;
	ioff nedges;
	ioff parent;
	ioff child;
	ioff sibling;
};
static CNode *cnodes;	/* immutable */
static ioff *cedges;	/* immutable */
static ioff nnodes, nedges;
static QLock nodelock;

ioff
getnodeidx(ioff id)
{
	ioff idx;

	if(cnodes == nil)
		return id;
	qlock(&nodelock);
	if(id < 0 || id >= nnodes){
		qunlock(&nodelock);
		warn("getnodeidx: out of bounds node id: %d > %d", id, nnodes-1);
		return -1;
	}
	idx = cnodes[id].idx;
	qunlock(&nodelock);
	return idx;
}

static void
printtree(void)
{
	ioff x, *e, *ee;
	Node *u, *ue;

	for(u=nodes, ue=u+dylen(u); u<ue; u++){
		warn("node[%zd] id=%d off=%d ne=%d\n", u-nodes, u->id, u->eoff, u->nedges);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			warn(" - edge[%zd] %zd%c%d%c\n", e-edges, u-nodes, x&1?'-':'+', x>>2, x&2?'-':'+');
		}
	}
}

static inline int
newedge(ioff i, ioff j, ioff e, edgeset *eset)
{
	int abs;
	u64int id;

	if(i > j || i == j && e & 1)	/* see fs/gfa.c:/^readedge */
		id = (u64int)j << 32 | i << 2 | (e >> 1 & 1 | e << 1 & 2) ^ 3;
	else
		id = (u64int)i << 32 | j << 2 | e & 3;
	es_put(eset, id, &abs);
	if(abs){
		DPRINT(Debugcoarse, "newedge: [%llx] %d%c%d%c", id, i, e&1?'-':'+', j, e&2?'-':'+');
		return 1;
	}
	DPRINT(Debugcoarse, "newedge: %d%c%d%c is redundant", i, e&1?'-':'+', j, e&2?'-':'+');
	return 0;
}

int
expand(ioff)
{
	return 0;
}

/* this should be generic enough to allow the use of any coarsening method but just
 * update the internal data structures */
int
coarsen(void)
{
	u64int edge;
	ioff x, i0, i, j, ci, cj, off, ne, *e, *ee, *remap, *degree;
	edgeset *eset;
	khint_t k;
	Node *u, *ue, *v, *p;
	RNode *r, *rp;
	CNode *U;

	if(cnodes == nil){
		werrstr("no coarsening tree");
		return -1;
	}
	reqlayout(Lstop);
	reqdraw(Reqstop);
	if(debug & Debugcoarse)
		printtree();
	DPRINT(Debugcoarse, "coarsen: pass 1: remap positions");
	remap = nil;
	for(i0=0, u=nodes, ue=u+dylen(u); u<ue; u++){
		DPRINT(Debugcoarse, "coarsen: node %zd off=%d ne=%d", u-nodes, u->eoff, u->nedges);
		ci = u->id;
		U = cnodes + ci;
		if((U->flags & CFinactive) == 0){	/* reassign parent node to first available slot */
			DPRINT(Debugcoarse, "coarsen: remapping active node: %zd→%d", u-nodes, i0);
			dypush(remap, ci);
			u->id = i0++;
			continue;
		}
		U->idx = -1;
		u->flags |= FNalias;	/* mark node, not cnode, as a pointer */
		do{	/* get first active parent */
			ci = U->parent;
			assert(ci >= 0 && ci < nnodes);	/* must have one */
			U = cnodes + ci;
		}while(U->flags & CFinactive);
		DPRINT(Debugcoarse, "coarsen: first active parent: %d", ci);
		/* find parent node */
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			DPRINT(Debugcoarse, "coarsen: %zd%c%d%c", u-nodes, x&1?'-':'+', j, x&2?'-':'+');
			v = nodes + j;
			if(v->flags & FNalias){
				DPRINT(Debugcoarse, "coarsen: %zd: adj %d can't be my parent", u-nodes, j);
				continue;
			}
			else if(v < u){
				DPRINT(Debugcoarse, "coarsen: %zd: check remapped %d→%zd→%zd", u-nodes,
					j, v->id, remap[v->id]);
				cj = remap[v->id];
			}else{
				DPRINT(Debugcoarse, "coarsen: %zd: check unseen %d→%zd", u-nodes, j, v->id);
				cj = v->id;
			}
			if(ci == cj){
				DPRINT(Debugcoarse, "coarsen: %zd: adj %d is my parent", u-nodes, j);
				u->id = j;
				break;
			}
		}
		assert(e < ee);
	}
	/* corner case: invalid usage or assumptions */
	if(i0 == u - nodes){
		for(e=remap, u=nodes; u<ue; u++)
			u->id = *e++;
		dyfree(remap);
		werrstr("no op");
		return -1;
	}else
		DPRINT(Debugcoarse, "coarsen: %d silenced nodes", (u - nodes) - i0);
	eset = es_init();
	DPRINT(Debugcoarse, "coarsen: pass 2: collect belongings");
	degree = emalloc(dylen(nodes) * sizeof *degree);	/* FIXME: FUCK */
	for(ne=0, u=nodes; u<ue; u++){
		i = i0 = u->id;
		e = edges + u->eoff;
		ee = e + u->nedges;
		DPRINT(Debugcoarse, "coarsen: node %zd→%d", u-nodes, i);
		if(u->flags & FNalias){	/* pointer to parent pointing to its new slot */
			i = nodes[i].id;
			DPRINT(Debugcoarse, "coarsen: %d alias of %d", i0, i);
		}
		for(; e<ee; e++){
			x = *e;
			j = x >> 2;
			DPRINT(Debugcoarse, "coarsen: %d: edge[%zd] with %d", i0, e-edges, j);
			v = nodes + j;
			j = v->id;
			if(v->flags & FNalias)
				j = nodes[j].id;
			if(i == j && (v->flags | u->flags) & FNalias){
				DPRINT(Debugcoarse, "coarsen: %d: prune artificial self-edge");
				continue;
			}
			if(!newedge(i, j, x, eset))
				continue;
			degree[i]++;
			ne++;
			if(i != j)
				degree[j]++;
		}
	}
	DPRINT(Debugcoarse, "coarsen: pass 3: squash resistance");
	qlock(&nodelock);	/* FIXME: sucks */
	for(off=0, e=remap, ee=degree, r=rp=rnodes, p=u=nodes; u<ue; u++, r++){
		if(u->flags & FNalias)
			continue;
		memcpy(p, u, sizeof *p);
		p->nedges = *ee++;
		off += p->nedges;
		p->eoff = off;	/* set one past u's last edge, to store them backwards */
		p->flags &= ~FNalias;
		p->id = *e++;
		cnodes[p->id].idx = p - nodes;
		p++;
		memcpy(rp, r, sizeof *rp);
		rp++;
	}
	qunlock(&nodelock);
	assert(p < u);
	assert(off < dylen(edges));
	free(degree);
	dyfree(remap);
	DPRINT(Debugcoarse, "coarsen: %d → %zd nodes, %d → %d (%d) edges",
		dylen(rnodes), p-nodes, dylen(redges), ne, off);
	x = p - nodes;
	dyresize(nodes, x);
	dyresize(rnodes, x);
	dyresize(edges, off);
	dyresize(redges, ne);
	/* reassign edges */
	DPRINT(Debugcoarse, "coarsen: redistribute power");
	kh_foreach(eset, k){
		edge = kh_key(eset, k);
		i = edge >> 32 & 0xffffffff;
		x = edge & 0xffffffff;
		edges[--nodes[i].eoff] = x;
		j = edge >> 2 & 0x3fffffff;
		if(i == j)	/* the mirror is what we just added */
			continue;
		x = edge >> 30 & 0xfffffffc | (edge >> 1 & 1 | edge << 1 & 2) ^ 3 ;
		edges[--nodes[j].eoff] = x;
	}
	es_destroy(eset);
	reqlayout(Lstart);
	if(debug & Debugcoarse)
		printtree();
	return 0;
}

static void
mkparent(ioff i, CNode *U)
{
	ioff j, x, *e, *ee;
	CNode *V;

	DPRINT(Debugcoarse, "mkparent %d", i);
	assert(i == U - cnodes);
	for(e=cedges+U->eoff, ee=e+U->nedges; e<ee; e++){
		x = *e;
		j = x >> 2;
		assert(j < nnodes);
		V = cnodes + j;
		DPRINT(Debugcoarse, "mkparent %d: child %d p=%d c=%d s=%d", i, j,
			V->parent, V->child, V->sibling);
		/* if V already is a branch, get its top supernode */
		while(V->parent != -1){
			V = cnodes + (j = V->parent);
			DPRINT(Debugcoarse, "mkparent %d: moving up to %d p=%d c=%d s=%d", i, j,
				V->parent, V->child, V->sibling);
		}
		if(V == U){
			DPRINT(Debugcoarse, "mkparent %d: aliased child %d", i, j);
			continue;
		}
		/* push new child */
		V->parent = i;
		if(U->child == -1){
			DPRINT(Debugcoarse, "mkparent %d: firstborn %d", i, j);
			U->child = j;
			if(V->sibling == -1)
				continue;
			while(V->sibling != -1){
				V = cnodes + V->sibling;
				DPRINT(Debugcoarse, "mkparent %d: new parent of %zd sibling of %d: %zd p=%d c=%d s=%d",
					i, V - cnodes, j, V->parent, V->child, V->sibling);
				if(V->parent == -1)
					V->parent = i;
				else
					assert(V->parent == i);
			}
			V->sibling = j;
		}else{
			for(V=cnodes+U->child; V->sibling!=-1; V=cnodes+V->sibling)
				assert(V->parent == i);
			V->sibling = j;
			DPRINT(Debugcoarse, "mkparent %d: new sibling of %d: %zd p=%d c=%d s=%d", i, j,
				V - cnodes, V->parent, V->child, V->sibling);
		}
	}
}

static void
init(void)
{
	ioff i;
	ssize n;
	CNode *U, *E;
	Node *u;

	initqlock(&nodelock);
	nnodes = dylen(nodes);
	n = nnodes;
	cnodes = emalloc(n * sizeof *cnodes);
	for(i=0, u=nodes, U=cnodes, E=U+n; U<E; U++, u++, i++){
		U->idx = i;
		U->eoff = u->eoff;
		U->nedges = u->nedges;
		U->parent = U->child = U->sibling = -1;
	}
	nedges = dylen(edges);
	n = nedges * sizeof *cedges;
	cedges = emalloc(n);
	memcpy(cedges, edges, n);
}

/* the choice of which node to collapse is the caller's responsibility */
int
collapse(ioff i)
{
	CNode *U, *V;

	DPRINT(Debugcoarse, "collapse %d", i);
	if(cnodes == nil)
		init();
	if(i < 0 || i >= nnodes){
		werrstr("out of bounds %d > %d", i, nnodes);
		return -1;
	}
	U = cnodes + i;
	if(U->flags & CFinactive){
		werrstr("inactive node");
		return -1;
	}
	if(U->nedges == 0){
		werrstr("no adjacencies");
		return -1;
	}
	if(U->idx < 0){
		werrstr("phase error: unlinked active node");
		return -1;
	}
	if(U->parent != -1)
		U = cnodes + U->parent;
	else if(U->child == -1)
		mkparent(i, U);
	assert(U->child != -1);
	V = cnodes + U->child;	/* cannot be blank */
	V->flags |= CFinactive;
	while(V->sibling != -1){
		V = cnodes + V->sibling;
		V->flags |= CFinactive;
	}
	if(debug & Debugcoarse)
		printtree();
	return 0;
}
