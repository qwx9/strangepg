#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"

Graph *graphs;

static RWLock *locks;

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

void
lockgraph(Graph *g, int w)
{
	RWLock *l;

	l = locks + (g - graphs);
	if(w)
		wlock(l);
	else
		rlock(l);
}

void
unlockgraph(Graph *g, int w)
{
	RWLock *l;

	l = locks + (g - graphs);
	if(w)
		wunlock(l);
	else
		runlock(l);
}

Graph *
pushgraph(Graph *g)
{
	RWLock l;

	dypush(graphs, *g);
	g = graphs + dylen(graphs) - 1;
	newlayout(g, -1);
	memset(&l, 0, sizeof l);
	dypush(locks, l);
	return g;
}

void
initgraph(Graph *g, int type)
{
	memset(g, 0, sizeof *g);
	g->type = type;
}
