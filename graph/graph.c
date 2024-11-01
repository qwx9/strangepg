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

	r.len = 1.0f;
	off = dylen(g->nodes);
	dypush(g->nodes, n);
	col = "";
	setcolor(r.col, somecolor(off, &col));
	dypush(rnodes, r);
	DPRINT(Debugfs, "addnode %d:%s\n", off, s != nil ? s : "");
	if(s != nil)
		pushcmd("addnode(%d,\"%s\",%s)", off, s, col);
	else
		pushcmd("addnode(%d,\"%d\",%s)", off, off, col);
	return off;
}

ioff
newedge(Graph *g, ioff u, ioff v, int urev, int vrev, char *label)
{
	ioff off;
	REdge r = {0};

	off = dylen(redges);
	assert((off & 3 << 30) == 0);	/* give up for now */
	setcolor(r.col, theme[Cedge]);
	dypush(redges, r);
	dypush(g->nodes[u].out, v << 2 | urev << 1 & 2 | vrev & 1);
	dypush(g->nodes[v].in, u << 2 | vrev << 1 & 2 | urev & 1);
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
	/* FIXME: selection box kludge */
	REdge r = {
		.pos1 = {0.0f, 0.0f, 0.0f},
		.pos2 = {0.0f, 0.0f, 0.0f},
		.col = {1.0f, 0.0f, 0.0f, 0.8f},
	};

	dypush(redges, r);
	dypush(redges, r);
	dypush(redges, r);
	dypush(redges, r);
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
