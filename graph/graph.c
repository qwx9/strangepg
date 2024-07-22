#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "em.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"

/* FIXME: think about an alternate, easier, em-backed representation: just a tree
 *	expanded nodes are marked expanded, otherwise a traversal ends on that,
 *	shit like that; em makes our life considerably easier here, why not just
 *	do that? benchmark to see what the performance hit would be, if any
 *	same for layouting; use em there as well
 * still, reconsider if the below is fine or not, perhaps use it in coarsen
 * BEST: indices == bct indices, always, no renaming -> easier edge management, etc;
 *	avoid long traversals by keeping a current pointer/index to sibling/child/edge,
 *	etc; describe the ds, ask for opinions, measure performance; cache lines
 */

/* nodes: .in and .out:
 *	undirected: .in is ignored;
 *	directed: .in/.out are incoming/outgoing edges
 *	bidirected: .in/.out are head/tail sides, paths define orientation.
 * active/inactive nodes/edges:
 *	nodes are stored by id in a linear dynamic array in reverse order
 *	ie. starting from 0; active (visible) nodes are linked in a
 *	circular doubly linked list; a hashmap maps id's to indices;
 *  same for edges. for all, indices into the backing array are
 *	kept instead of pointers to decouple them from the underlying
 *	memory model.
 * node expansion:
 *	the node and its edges are removed from the graph; the node is
 *	unlinked and .ch now points to its first child; additional
 *	children are added next to the first as siblings; thus the
 *	definition of a child pointer defines a parent node; to reconnect
 *	existing nodes, all edges from the new, lower depth level of
 *	the children (at least one endpoint is a new child) are added;
 *	since the coarsening tree is breadth-first, adjacent nodes
 *	might not have been expanded at this time and the existing node
 *	may be further up the chain, which must therefore be jumped
 *	through to find it before adding the new edge; to avoid
 *	invalid references, all nodes at a new level are added without
 *	linking them to the active list
 * this is obviously deficient and does not scale but is soon to
 *	be replaced by data structures that do but are tricker.
 */

Graph *graphs;
RWLock graphlock;

void
printgraph(Graph *g)
{
	ssize i, *np, *nq;
	Edge *e;
	Node *n;

	if((debug & Debugcoarse) == 0)
		return;
	warn("graph %#p nn %zd ne %zd ns %zd: actual nn %zd ne %zd\n", g, g->nnodes, g->nedges, g->nsuper, dylen(g->nodes), dylen(g->edges));
	for(i=g->edge0.next; i>=0; i=e->next){
		e = g->edges + i;
		warn("e[%04zx] %#p %zx %zx,%zx\n", i, e, e->id, e->u >> 1, e->v >> 1);
	}
	assert(dylen(g->nodes) > 0);
	for(i=g->node0.next; i>=0; i=n->next){
		n = g->nodes + i;
		warn("n[%04zx] %#p %zx pid %zx idx %zx weight %d ch %zx → [ ",
			i, n, n->id, n->pid, n->idx, n->weight, n->ch);
		for(np=n->out, nq=np+dylen(np); np<nq; np++)
			warn("%zx ", *np);
		warn("] ← [ ");
		for(np=n->in, nq=np+dylen(np); np<nq; np++)
			warn("%zx ", *np);
		warn("] >%zx <%zx\n", n->next, n->prev);
	}
}

Edge *
getedge(Graph *g, ssize id)
{
	ssize i;
	khiter_t k;

	k = kh_get(idmap, g->emap, id);
	if(k == kh_end(g->emap))
		return nil;
	i = kh_val(g->emap, k);
	if(i < 0)
		return nil;
	assert(i >= 0 && i < dylen(g->edges));
	return g->edges + i;
}

Node *
getnode(Graph *g, ssize id)
{
	ssize i;
	khiter_t k;

	k = kh_get(idmap, g->nmap, id);
	if(k == kh_end(g->nmap)){
		werrstr("getnode: no such node id=%zd\n", id);
		return nil;
	}
	i = kh_val(g->nmap, k);
	assert(i >= 0 && i < dylen(g->nodes));
	return g->nodes + i;
}

Node *
str2node(Graph *g, char *s)
{
	ssize id;
	char *p;

	id = strtoll(s, &p, 0);
	if(p == s){
		werrstr("unknown node id \'%s\'", s);
		return nil;
	}
	return getnode(g, id);
}

/* an inactive node is a future child and points to itself */
Node *
getactivenode(Graph *g, Node *n)
{
	DPRINT(Debugcoarse, "getactivenode [%zx]%zx %zx %zx",
		n - g->nodes, n->id, n->next, n->idx);
	while(n->next == n - g->nodes && n->pid >= 0 && n->ch == -1)
		n = getnode(g, n->pid);
	assert(n != nil);
	return n;
}

static Node *
getnamednode(Graph *g, char *s)
{
	ssize i;
	khiter_t k;

	k = kh_get(strmap, g->strnmap, s);
	if(k == kh_end(g->strnmap))
		return nil;
	i = kh_val(g->strnmap, k);
	assert(i >= 0 && i < dylen(g->nodes));
	return g->nodes + i;
}

int
ischild(Graph *g, ssize ch, ssize pid)
{
	Node *n;

	if((n = getnode(g, ch)) == nil)
		return 0;
	return n->pid == pid;
}

/* screw it */
#define LINK(p, l0, l, r)	do{ \
	if((r)->prev >= 0) \
		p[(r)->prev].next = (l)->next; \
	else \
		(l0)->next = (l)->next; \
	if((l)->next >= 0) \
		p[(l)->next].prev = (r)->prev; \
	else \
		(l0)->prev = (r)->prev; \
	(r)->prev = (l) != (l0) ? (l) - (p) : -1; \
	(l)->next = (r) != (l0) ? (r) - (p) : -1; \
}while(0)

#define UNLINK(p, l0, l, r)	do{ \
	if((l)->prev >= 0) \
		p[(l)->prev].next = (r)->next; \
	else \
		(l0)->next = (r)->next; \
	if((r)->next >= 0) \
		p[(r)->next].prev = (l)->prev; \
	else \
		(l0)->prev = (l)->prev; \
	(l)->prev = (r) != (l0) ? (r) - (p) : -1; \
	(r)->next = (l) != (l0) ? (l) - (p) : -1; \
}while(0)

static void
cuttie(ssize *es, ssize id)
{
	ssize i, n;

	for(i=0, n=dylen(es); i<n; i++)
		if(es[i] == id){
			//es[i] = -1;
			dydelete(es, i);
			break;
		}
}
static void
redactedge(Graph *g, ssize id)
{
	Edge *e;
	Node *u, *v;
	khiter_t k;

	DPRINT(Debugcoarse, "redactedge %#p %zx", g, id);
	if((e = getedge(g, id)) == nil){
		warn("redactedge %zx: already gone\n", id);
		return;
	}
	assert(e != nil);
	u = getnode(g, e->u >> 1);
	v = getnode(g, e->v >> 1);
	DPRINT(Debugcoarse, "removing %zx,%zx", u->id, v->id);
	assert(u != nil && v != nil);
	cuttie((e->u & 1) == 0 ? u->out : u->in, id);
	cuttie((e->v & 1) == 0 ? v->in : v->out, id);
	k = kh_get(idmap, g->emap, id);
	kh_del(idmap, g->emap, k);
	pushcmd("deledge(%d)", e->id);
	UNLINK(g->edges, &g->edge0, e, e);
	printgraph(g);
}

void
hidenode(Graph *g, Node *n)
{
	ssize *e, *ee;

	DPRINT(Debugcoarse, "hidenode %#p %zx >%zd <%zd", g, n->id, dylen(n->out), dylen(n->in));
	assert(n->ch < 0);	// FIXME: opposite unhandled, but should it be?
	for(e=n->out, ee=e+dylen(n->out); e<ee; e++)
		redactedge(g, *e);
	for(e=n->in, ee=e+dylen(n->in); e<ee; e++)
		redactedge(g, *e);
	pushcmd("delnode(%d)", n->id);
	UNLINK(g->nodes, &g->node0, n, n);
	dyfree(n->out);
	dyfree(n->in);
}

static Node *
newnode(Graph *g, ssize id, ssize pid, ssize idx, int w)
{
	int ret;
	ssize i;
	Node n = {0}, *np;
	khiter_t k;
	char bleh[32];

	k = kh_put(idmap, g->nmap, id, &ret);
	if(ret == 0){
		warn("newnode: not overwriting existing node %zx\n", id);
		i = kh_val(g->nmap, k);
		return g->nodes + i;
	}
	n.id = id;
	n.pid = pid;
	n.dir = V(1.0f, 0.0f, 0.0f);
	n.idx = idx;
	n.length = 1;
	n.weight = w;
	n.ch = -1;
	n.col = somecolor(g);
	i = g->nsuper > 0 ? g->nsuper - id : dylen(g->nodes);	// FIXME: what the fuck is the point of the hashtab then
	dyinsert(g->nodes, i, n);
	kh_val(g->nmap, k) = i;
	np = g->nodes + i;
	np->prev = np->next = i;
	snprint(bleh, sizeof bleh, "!@#$%%^&*()_+%zd", id);	// FIXME: gfa only?
	pushcmd("addnode(%d,\"%s\",\"0x%x\")", id, bleh, col2int(n.col));
	return np;
}

// FIXME: lvl not set, same as pushnode; defer setting or allow dynamic level? (why?)
/* create node and references but don't actually use it */
Node *
touchnode(Graph *g, ssize id, ssize pid, ssize idx, int w)
{
	Node *n;

	n = newnode(g, id, pid, idx, w);
	return n;
}

Node *
pushnode(Graph *g, ssize id, ssize pid, ssize idx, int w)
{
	ssize i;
	Node *n, *m;

	n = touchnode(g, id, pid, idx, w);
	for(i=g->node0.next, m=&g->node0; i>=0; i=m->next){
		m = g->nodes + i;
		if(m->id < n->id)	/* ids in reverse order */
			break;
	}
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

Node *
pushsibling(Graph *g, ssize id, Node *m, ssize idx, int w)
{
	Node *n;

	n = touchnode(g, id, m->pid, idx, w);
	n->lvl = m->lvl;
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

Node *
pushchild(Graph *g, ssize id, Node *pp, ssize idx, int w)
{
	ssize i;
	Node *n, *m;

	DPRINT(Debugcoarse, "pushchild [%zx]%zx←%zx", idx, id, pp!=nil?pp->id:-1);

	n = newnode(g, id, pp != nil ? pp->id : -1, idx, w);
	if(pp != nil){
		hidenode(g, pp);
		pp->ch = n - g->nodes;
		n->lvl = pp->lvl + 1;
	}else
		n->lvl = 0;
	for(i=g->node0.prev, m=&g->node0; i>=0; i=m->prev){
		m = g->nodes + i;
		if(m->id > n->id)	/* ids in reverse order */
			break;
	}
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

Node *
pushnamednode(Graph *g, char *s)
{
	int ret;
	ssize i, id;
	Node *n;
	khiter_t k;

	if((n = getnamednode(g, s)) != nil){
		DPRINT(Debugfs, "duplicate node %s", s);
		return n;
	}
	i = dylen(g->nodes);
	id = i;
	n = pushnode(g, id, -1, i, 1);
	s = estrdup(s);
	k = kh_put(strmap, g->strnmap, s, &ret);
	assert(ret != 0);
	kh_val(g->strnmap, k) = id;
	DPRINT(Debugcoarse, "pushnamednode %zd", id);
	pushcmd("addnode(%d,\"%s\")", id, s);
	return n;
}

static Edge *
newedge(Graph *g, Node *u, Node *v, int udir, int vdir)
{
	ssize i, id;
	int ret;
	Edge e = {0}, *ep;
	khiter_t k;

	/* FIXME: won't scale; could have another pass after input parsing to remap */
	//id = v->id * g->nsuper + u->id << 2 | udir << 1 & 1 | vdir & 1;
	//id = dylen(g->edges) << 2 | udir << 1 & 1 | vdir & 1;
	id = dylen(g->edges);
	DPRINT(Debugcoarse, "newedge %c%zd,%c%zd id=%zd", udir?'<':'>', u->id, vdir?'<':'>', v->id, id);
	k = kh_put(idmap, g->emap, id, &ret);
	if(ret == 0){
		warn("newedge: not overwriting existing edge [%zx] %zx,%zx\n", id, u->id, v->id);
		i = kh_val(g->emap, k);
		return g->edges + i;
	}
	e.id = id;
	e.u = u->id << 1 | udir & 1;
	e.v = v->id << 1 | vdir & 1;
	dypush(u->out, id);
	dypush(v->in, id);
	dypush(g->edges, e);
	i = dylen(g->edges) - 1;
	kh_val(g->emap, k) = i;
	ep = g->edges + i;
	ep->prev = ep->next = i;
	DPRINT(Debugcoarse, "newedge %zx[%zd]: %zd,%zd", e.id, i, e.u>>1, e.v>>1);
	pushcmd("addedge(%d,%d,%d,%d,%d)", id, e.u, udir, e.v, vdir);
	return ep;
}

Edge *
pushedge(Graph *g, Node *u, Node *v, int udir, int vdir)
{
	Edge *e, *pe;

	if((e = newedge(g, u, v, udir, vdir)) == nil)
		return nil;
	if(e->prev == e->next){	/* new */
		if((pe = g->edges + g->edge0.prev) < g->edges)
			pe = &g->edge0;
		LINK(g->edges, &g->edge0, pe, e);
	}
	return e;
}

/* id's in edges are always packed with direction bit */
Edge *
pushnamededge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	Node *u, *v;

	DPRINT(Debugcoarse, "pushnamededge %s,%s", eu, ev);
	if((u = getnamednode(g, eu)) == nil
	&& pushnamednode(g, eu) == nil)
		warn("pushnamededge: %s\n", error());
	if((v = getnamednode(g, ev)) == nil
	&& pushnamednode(g, ev) == nil)
		warn("pushnamededge: %s\n", error());
	if(u == nil || v == nil){
		u = getnamednode(g, eu);
		v = getnamednode(g, ev);
		assert(u != nil && v != nil);
	}
	return pushedge(g, u, v, d1, d2);
}

void 
poptree(Graph *g, Node *p)
{
	Node *n, *np;

	for(n=g->nodes+p->ch; n->pid==p->id; n=np){
		np = g->nodes + n->next;
		if(p->ch >= 0)
			poptree(g, n);
	}
	p->ch = -1;
	hidenode(g, n);
}

void
cleargraph(Graph *g)
{
	Node *n;

	for(n=g->nodes; n<g->nodes+dylen(g->nodes); n++){
		dyfree(n->in);
		dyfree(n->out);
	}
	dyfree(g->nodes);
	dyfree(g->edges);
	kh_destroy(idmap, g->nmap);
	kh_destroy(idmap, g->emap);
	g->nmap = g->emap = nil;
}

void
nukegraph(Graph *g)
{
	if(g->type <= FFdead)
		return;
	if(haltlayout(g) < 0)
		warn("nukegraph: %s\n", error());
	cleargraph(g);
	freefs(g->f);	// FIXME: probably not necessary to have in the first place
	memset(g, 0, sizeof *g);
}

/* FIXME: avoid pointless pass by value */
/* FIXME: since graphs[] can change, for mooltigraph we need again to use indices,
 *	or an array of pointers; same for nodes and edges maybe? avoiding the node0
 *	and edge0 dance? */
void
pushgraph(Graph g)
{
	lockgraphs(1);
	dypush(graphs, g);
	unlockgraphs(1);
	/* FIXME: selectable type */
	if(runlayout(graphs + dylen(graphs) - 1, -1) < 0)
		warn("pushgraph: %s\n", error());
}

void
lockgraphs(int w)
{
	if(w)
		wlock(&graphlock);
	else
		rlock(&graphlock);
}

void
unlockgraphs(int w)
{
	if(w)
		wunlock(&graphlock);
	else
		runlock(&graphlock);
}

Graph
initgraph(int type)
{
	Graph g = {0};

	g.type = type;
	g.node0.next = g.node0.prev = -1;
	g.edge0.next = g.edge0.prev = -1;
	g.nmap = kh_init(idmap);
	g.emap = kh_init(idmap);
	g.strnmap = kh_init(strmap);
	return g;
}
