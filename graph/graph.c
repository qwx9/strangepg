#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"

Graph *graphs;

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
	if(w)
		wlock(&g->lock);
	else
		rlock(&g->lock);
}

void
unlockgraph(Graph *g, int w)
{
	if(w)
		wunlock(&g->lock);
	else
		runlock(&g->lock);
}

Graph *
pushgraph(Graph *g)
{
	dypush(graphs, *g);
	g = graphs + dylen(graphs) - 1;
	newlayout(g, -1);
	return g;
}

void
initgraph(Graph *g, int type)
{
	memset(g, 0, sizeof *g);
	g->type = type;
}
