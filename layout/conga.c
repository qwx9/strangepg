#include "strpg.h"
#include "layout.h"
#include "drw.h"

static void *
new(Graph *g)
{
	int x;
	ioff i, ie;
	Node *u;

	x = -(Nodesz + Ptsz) * (dylen(g->nodes) - 1) / 2;
	for(i=0, ie=dylen(g->nodes); i<ie; i++, x+=Nodesz+Ptsz){
		u = g->nodes + i;
		u->pos.x = x;
		u->pos.y = 0;
	}
	return nil;
}

static Shitkicker ll = {
	.name = "congaline",
	.new = new,
};

Shitkicker *
regconga(void)
{
	return &ll;
}
