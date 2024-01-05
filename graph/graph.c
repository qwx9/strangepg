#include "strpg.h"
#include "em.h"

// FIXME: make slides describing the model, for notes + pres
// sorted Supers pointing to other super or containing a node
// + data types and layout
// id's are always consecutive, from 0 to nsuper+nnodes
// Super either contains a node or links to another Super representing a first child
// Siblings are neighboring Supers immediately following the first child
// if a child procreates, it retains its id but now points to a child out of range
//		!! child indices are determined by the bct
//		that means that child generation creates gaps
//		but that's ok, both Supers and Nodes are linked lists
//		Supers point to their Node or a Super
// in the next implementation, we will have blocks instead
// defining intervals we skip over of a single type (super, node, gap)
// until then, this is fine
// FIXME: ^- wrong (but take note of design)

Graph *graphs;

// FIXME: scale/em/other ds; precomputed or use em; O(1) find node
// FIXME: write note: linear arrays fine for csr-like format with
//	easy translation; otherwise a hashtable makes more sense;
//	if we can have external hashing, with khash or other, great
//	don't try to do it ourselves...
// FIXME: take note: node id's are stable; edge id's should be
//	stable as well; ie. don't worry about sparsity in the main
//	tables, not until it's actually a problem; also: em

void
printgraph(Graph *g)
{
	ssize i, *np, *nq;
	Edge *e;
	Node *n;

	if(debug == 0)
		return;
	warn("graph %#p nn %zd ne %zd ns %zd\n", g, g->nnodes, g->nedges, g->nsuper);
	for(i=g->edge0.next; i>=0; i=e->next){
		e = g->edges + i;
		warn("e[%04zx] %zx,%zx\n", i, e->u >> 1, e->v >> 1);
	}
	assert(dylen(g->nodes) > 0);
	for(i=g->node0.next; i>=0; i=n->next){
		n = g->nodes + i;
		warn("n[%04zx] %#p %zx pid %zx weight %d ch %zx → [ ",
			i, n, n->id, n->pid, n->weight, n->ch);
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
	assert(i >= 0 && i < dylen(g->edges));
	return g->edges + i;
}

Node *
getnode(Graph *g, ssize id)
{
	ssize i;
	khiter_t k;

	k = kh_get(idmap, g->nmap, id);
	if(k == kh_end(g->nmap))
		return nil;
	i = kh_val(g->nmap, k);
	assert(i >= 0 && i < dylen(g->nodes));
	return g->nodes + i;
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
void
linknode(Node *p, Node *a, Node *b)
{
	p[b->prev].next = a->next; \
	p[a->next].prev = b->prev; \
	b->prev = a - p; \
	a->next = b - p; \
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

/*
#define LINK(p, l, r)	do{ \
	p[(r)->prev].next = (l)->next; \
	p[(l)->next].prev = (r)->prev; \
	(r)->prev = (l) - (p); \
	(l)->next = (r) - (p); \
}while(0)
#define UNLINK(p, l, r)	do{ \
	p[(l)->prev].next = (r)->next; \
	p[(r)->next].prev = (l)->prev; \
	(l)->prev = (r) - (p); \
	(r)->next = (l) - (p); \
}while(0)
*/

static void
cuttie(ssize *e, ssize id)
{
	ssize *es, *ee;

	for(es=e, ee=e+dylen(e); e<ee; e++)
		if(*e == id){
			if(e+1 < ee)
				memmove(e, e+1, ee-1 - e);
			dypop(es);
			break;
		}
	assert(e != ee);
}
static void
redactedge(Graph *g, ssize id)
{
	Edge *e;
	Node *u, *v;
	khiter_t k;

	e = getedge(g, id);
	assert(e != nil);
	u = getnode(g, e->u >> 1);
	v = getnode(g, e->v >> 1);
	assert(u != nil && v != nil);
	cuttie((e->u & 1) == 0 ? u->out : u->in, id);
	cuttie((e->v & 1) == 0 ? v->in : v->out, id);
	memset(e, 0xfe, sizeof *e);
	k = kh_get(idmap, g->emap, id);
	kh_del(idmap, g->emap, k);
	UNLINK(g->edges, &g->edge0, e, e);
}

static void
redactnode(Graph *g, Node *n)
{
	while(dylen(n->out) > 0)
		redactedge(g, n->out[0]);
	while(dylen(n->in) > 0)
		redactedge(g, n->in[0]);
	dyfree(n->out);
	dyfree(n->in);
	UNLINK(g->nodes, &g->node0, n, n);
	n->id = Bupkis;
}

static Node *
newnode(Graph *g, ssize id, ssize pid, int w)
{
	int ret;
	ssize i;
	Node n = {0}, *np;
	khiter_t k;

	//memset(&n, 0, sizeof n);
	k = kh_put(idmap, g->nmap, id, &ret);
	assert(ret != 0);
	n.id = id;
	n.pid = pid;
	n.weight = w;
	n.ch = -1;
	i = g->nsuper > 0 ? g->nsuper - id : dylen(g->nodes);	// FIXME: what the fuck is the point of the hashtab then
	dyinsert(g->nodes, i, n);
	kh_val(g->nmap, k) = i;
	np = g->nodes + i;
	np->prev = np->next = i;
	warn("newnode %zx <%zx\n", id, pid);
	return np;
}

Node *
pushnode(Graph *g, ssize id, ssize pid, int w)
{
	ssize i;
	Node *n, *m;

	n = newnode(g, id, pid, w);
	warn("pushnode %zx id %zx pid %zx len %zd\n", n - g->nodes, n->id, n->pid, dylen(g->nodes));
	printgraph(g);
	for(i=g->node0.next, m=&g->node0; i>=0; i=m->next){
		m = g->nodes + i;
		if(m->id > n->id)	/* ids in reverse order */
			break;
	}
	LINK(g->nodes, &g->node0, m, n);
	return n;
}

Node *
pushsibling(Graph *g, ssize id, Node *m, int w)
{
	Node *n;

	n = newnode(g, id, m->pid, w);
	warn("pushsibling %zx id %zx pid %zx\n", n - g->nodes, n->id, n->pid);
	n->lvl = m->lvl;
	LINK(g->nodes, &g->node0, m, n);
	printgraph(g);
	return n;
}

Node *
pushchild(Graph *g, ssize id, Node *pp, int w)
{
	ssize i;
	Node *n, *m;

	n = newnode(g, id, pp != nil ? pp->id : -1, w);
	warn("pushchild %zx id %zx pid %zx len %zd\n", n - g->nodes, n->id, n->pid, dylen(g->nodes));
	if(pp != nil){
		pp->ch = n - g->nodes;
		UNLINK(g->nodes, &g->node0, pp, pp);
		n->lvl = pp->lvl + 1;
	}else
		n->lvl = 0;
	for(i=g->node0.prev, m=&g->node0; i>=0; i=m->prev){
		m = g->nodes + i;
		if(m->id > n->id)	/* ids in reverse order */
			break;
	}
	LINK(g->nodes, &g->node0, m, n);
	printgraph(g);
	return n;
}

Node *
pushnamednode(Graph *g, char *s)
{
	int ret;
	ssize i, id;
	Node *n;
	khiter_t k;
	//char *kk;
	//usize vv;

	warn("pushnamednode %s\n", s);
	//kh_foreach(g->strnmap, kk, vv, {warn("%s:%zx\n", kk, vv);});
	if(getnamednode(g, s) != nil){
		werrstr("duplicate node id %s", s);
		return nil;
	}
	i = dylen(g->nodes);
	id = i;
	n = pushnode(g, id, -1, 1);
	s = estrdup(s);
	k = kh_put(strmap, g->strnmap, s, &ret);
	assert(ret != 0);
	kh_val(g->strnmap, k) = id;
	return n;
}

// FIXME: need a higher level view with containers, there's no other way
//	or generate code from awk, or some awkscript (not executed), or something
// FIXME: pointers vs ints in the linked lists will bite us in the ass with em; should be offsets always?

Edge *
pushedge(Graph *g, Node *u, Node *v, int udir, int vdir)
{
	ssize i, id;
	int ret;
	Edge e = {0}, *ep, *pp;
	khiter_t k;

	warn("pushedge %zx,%zx\n", u->id, v->id);
	id = dylen(g->edges);
	k = kh_put(idmap, g->emap, id, &ret);
	assert(ret != 0);
	e.u = u->id << 1 | udir;
	e.v = v->id << 1 | vdir;
	dypush(u->out, id);
	dypush(v->in, id);
	i = id;
	dyinsert(g->edges, i, e);
	kh_val(g->emap, k) = i;
	ep = g->edges + i;
	ep->prev = ep->next = i;
	if(g->edge0.prev >= 0)
		pp = g->edges+g->edge0.prev;	/* goddamn macros */
	else
		pp = &g->edge0;
	LINK(g->edges, &g->edge0, pp, ep);
	return ep;
}

/* id's in edges are always packed with direction bit */
Edge *
pushnamededge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	Node *u, *v;
	Edge *e;

	warn("pushnamededge %s,%s\n", eu, ev);
	if((u = getnamednode(g, eu)) == nil)
		return nil;
	if((v = getnamednode(g, ev)) == nil)
		return nil;
	e = pushedge(g, u, v, d1, d2);
	return e;
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
	redactnode(g, n);
}

void
clearmeta(Graph *g)
{
	if(g->strnmap == nil)
		return;
	kh_destroy(strmap, g->strnmap);
	g->strnmap = nil;
}

void
cleargraph(Graph *g)
{
	Node *n;

	clearmeta(g);
	for(n=g->nodes; n<g->nodes+dylen(g->nodes); n++){
		dyfree(n->in);
		dyfree(n->out);
	}
	dyfree(g->nodes);
	dyfree(g->edges);
	g->edges = nil;
	g->nodes = nil;
	kh_destroy(idmap, g->nmap);
	kh_destroy(idmap, g->emap);
	g->nmap = g->emap = nil;
}

void
nukegraph(Graph *g)
{
	cleargraph(g);
	freefs(g->f);	// FIXME: probably not necessary to have in the first place
	free(g->layout.aux);
	memset(g, 0, sizeof *g);
	dydelete(graphs, g-graphs);
}

Graph*
initgraph(void)
{
	Graph g = {0};

	g.layout.tid = -1;
	g.node0.next = g.node0.prev = -1;
	g.edge0.next = g.edge0.prev = -1;
	g.nmap = kh_init(idmap);
	g.emap = kh_init(idmap);
	g.strnmap = kh_init(strmap);
	dypush(graphs, g);
	return graphs;
}
