#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "em.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "lib/khashl.h"

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

KHASHL_MAP_INIT(, namemap, names, char*, ioff, kh_hash_str, kh_eq_str)
static namemap *names;

void
printgraph(Graph *g)
{
	ioff i, ie, *np, *ne;
	Edge *e;
	Node *n;

	if((debug & Debugcoarse) == 0)
		return;
	warn("graph %#p nn %zd ne %zd\n", g, dylen(g->nodes), dylen(g->edges));
	for(i=0, ie=i+dylen(g->edges); i<ie; i++){
		e = g->edges + i;
		warn("e[%04x] %x%c:%x%c\n", i,
			e->u >> 1, e->u & 1 ? '-' : '+',
			e->v >> 1, e->v & 1 ? '-' : '+');
	}
	for(i=0, ie=i+dylen(g->nodes); i<ie; i++){
		n = g->nodes + i;
		warn("n[%04x] ch %x → [ ", i, n->ch);
		for(np=n->out, ne=np+dylen(np); np<ne; np++)
			warn("%x ", *np);
		warn("] ← [ ");
		for(np=n->in, ne=np+dylen(np); np<ne; np++)
			warn("%x ", *np);
		warn("]\n");
	}
}

/*
// an inactive node is a future child and points to itself
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

int
ischild(Graph *g, ioff ch, ioff pid)
{
	Node *n;

	if((n = getnode(g, ch)) == nil)
		return 0;
	return n->pid == pid;
}

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
cuttie(ioff *es, ioff id)
{
	ioff i, n;

	for(i=0, n=dylen(es); i<n; i++)
		if(es[i] == id){
			//es[i] = -1;
			dydelete(es, i);
			break;
		}
}
static void
redactedge(Graph *g, ioff id)
{
	Edge *e;
	Node *u, *v;
	khint_t k;

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

Node *
pushsibling(Graph *g, ioff id, Node *m, ioff idx, int w)
{
	Node *n;

	n = touchnode(g, id, m->pid, idx, w);
	n->lvl = m->lvl;
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

Node *
pushchild(Graph *g, ioff id, Node *pp, ioff idx, int w)
{
	ioff i;
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
		if(m->id > n->id)	// ids in reverse order
			break;
	}
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

void
hidenode(Graph *g, Node *n)
{
	ioff *e, *ee;

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
*/

/* usable once loading completed */
ioff
str2idx(char *s)
{
	ioff id;
	char *t;

	if((id = strtoll(s, &t, 0)) == 0 && t == s)
		return -1;
	return id;
}

/* usable solely during loading */
ioff
getid(Graph *, char *s)
{
	ioff id;
	khint_t k;

	k = names_get(names, s);
	if(k == kh_end(names))
		return -1;
	id = kh_val(names, k);
	return id;
}

static ioff
pushid(Graph *, char *s, ioff id)
{
	int abs;
	khint_t k;

	k = names_put(names, s, &abs);
	assert(abs);
	kh_val(names, k) = id;
	return 0;
}

static inline int
newnode(Graph *g, Node *n)
{
	ioff id;

	id = dylen(g->nodes);
	n->ch = -1;
	n->length = 1;	/* FIXME */
	n->weight = 1;	/* FIXME */
	return id;
}

ioff
pushinode(Graph *g, char **cname)
{
	ioff id;
	Node n = {0};
	RNode r = {0};

	if((id = newnode(g, &n)) < 0)
		return -1;
	dypush(g->nodes, n);
	setcolor(r.col, somecolor(id, cname));
	r.len = 1.0f;
	dypush(rnodes, r);
	return id;
}

ioff
pushnode(Graph *g, char *s)
{
	ioff id;
	char *cname;

	/* nodes may be defined after an edge references them, should
	 * not be considered as an error */
	if((id = getid(g, s)) >= 0){
		DPRINT(Debugfs, "duplicate node[%d] %s", id, s);
		return id;
	}
	s = estrdup(s);
	if((id = pushinode(g, &cname)) < 0 || pushid(g, s, id) < 0){
		free(s);
		return -1;
	}
	pushcmd("addnode(%d,\"%s\",%s)", id, s, cname);
	return id;
}

static inline ioff
newedge(Graph *g, Edge *e, ioff u, ioff v)
{
	ioff id;

	id = dylen(g->edges);
	e->u = u;
	e->v = v;
	dypush(g->nodes[u>>1].out, id);
	dypush(g->nodes[v>>1].in, id);
	return id;
}

ioff
pushiedge(Graph *g, ioff u, ioff v)
{
	ioff id;
	Edge e = {0};
	REdge r = {0};

	if((id = newedge(g, &e, u, v)) < 0)
		return -1;
	dypush(g->edges, e);
	setcolor(r.col, theme[Cedge]);
	dypush(redges, r);
	return id;
}

ioff
pushedge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	char *s, *sf;
	ioff id, f, n, u, v;

	if((u = pushnode(g, eu)) < 0 || (v = pushnode(g, ev)) < 0)
		return -1;
	n = strlen(eu) + strlen(ev) + 8;
	s = emalloc(n);
	sf = nil;
	snprint(s, n, "%s%c\x1c%s%c", eu, d1 ? '-' : '+', ev, d2 ? '-' : '+');
	/* detect redundancies by always flipping edges such that u<v, or
	 * for self-edges if u is reversed */
	if(f = v > u || v == u && d1 == 1){
		sf = estrdup(s);
		snprint(s, n, "%s%c\x1c%s%c", ev, d2 ? '+' : '-', eu, d1 ? '+' : '-');
	}
	if((id = getid(g, s)) >= 0){
		DPRINT(Debugfs, "duplicate edge[%d] %s%s", id, s, f ? " (flipped)" : "");
		free(s);
		free(sf);
		return id;
	}
	u = u << 1 | d1;
	v = v << 1 | d2;
	if((id = pushiedge(g, u, v)) < 0 || pushid(g, s, id) < 0){
		free(s);
		id = -1;
	}else{
		/* always push only what was actually in the input */
		pushcmd("addedge(%d,\"%s\")", id, sf != nil ? sf : s);
	}
	free(sf);
	return id;
}

void
cleargraphtempshit(Graph *)
{
	khint_t k;

	if(names == nil)
		return;
	kh_foreach(names, k)
		free(kh_key(names, k));
	names_destroy(names);
	names = nil;
}

static void
cleargraph(Graph *g)
{
	Node *n;

	for(n=g->nodes; n<g->nodes+dylen(g->nodes); n++){
		dyfree(n->in);
		dyfree(n->out);
	}
	dyfree(g->nodes);
	dyfree(g->edges);
}

void
nukegraph(Graph *g)
{
	if(g->type <= FFdead)
		return;
	reqlayout(g, Lstop);
	cleargraph(g);
	freefs(g->f);
	memset(g, 0, sizeof *g);
}

/* FIXME: avoid pointless pass by value */
void
pushgraph(Graph gp)
{
	Graph *g;

	lockgraphs(1);
	dypush(graphs, gp);
	unlockgraphs(1);
	g = graphs + dylen(graphs) - 1;
	newlayout(g, -1);
	if(gottagofast && reqlayout(g, Lstart) < 0)
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

	/* no refcounting nor mooltigraph */
	if(names == nil)
		names = names_init();
	g.type = type;
	return g;
}
