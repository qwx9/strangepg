#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "em.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "lib/khashl.h"

Graph *graphs;
RWLock graphlock;

/* usable once topology has been loaded */
ioff
str2idx(char *s)
{
	ioff id;
	char *t;

	if((id = strtoll(s, &t, 0)) == 0 && t == s)
		return -1;
	return id;
}

ioff
newnode(Graph *g, char *s)
{
	ioff off;
	char *col;
	Node n = {0};
	RNode r = {0};

	n.length = 1;
	r.len = 1.0f;
	off = dylen(g->nodes);
	dypush(g->nodes, n);
	col = "";
	setcolor(r.col, somecolor(off, &col));
	dypush(rnodes, r);
	if(s != nil)
		pushcmd("addnode(%d,\"%s\",%s)", off, s, col);
	else
		pushcmd("addnode(%d,\"%d\",%s)", off, off, col);
	return off;
}

/* u and v both already shifted */
ioff
newedge(Graph *g, ioff u, ioff v, char *label)
{
	ioff off;
	Edge e = {0};
	REdge r = {0};

	off = dylen(g->edges);
	e.u = u;
	e.v = v;
	dypush(g->edges, e);
	setcolor(r.col, theme[Cedge]);
	dypush(redges, r);
	/* FIXME: won't work if we delete edges; must be a better way to store
	 * these (with both orientations) */
	dypush(g->nodes[u>>1].out, off);
	dypush(g->nodes[v>>1].in, off);
	if(label != nil)
		pushcmd("addedge(%d,\"%s\")", off, label);
	else
		pushcmd("addedge(%d,\"%d\")", off, off);
	return off;
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

void
pushgraph(Graph *g)
{
	lockgraphs(1);
	dypush(graphs, *g);
	unlockgraphs(1);
	g = graphs + dylen(graphs) - 1;
	newlayout(g, -1);
	if(gottagofast && reqlayout(g, Lstart) < 0){
		warn("pushgraph: %s\n", error());
		pushcmd("cmd(\"FHJ142\")");
	}
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
	return g;
}
