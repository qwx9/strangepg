#include "strpg.h"
#include "layout.h"

static void *
new(Graph *g)
{
	int x;
	ssize i;
	Node *u;

	for(i=g->node0.next, x=-(Nodesz+Ptsz)*(g->nnodes-1)/2; i>=0; i=u->next, x+=Nodesz+Ptsz){
		u = g->nodes + i;
		u->vrect = Qd(Vec2(x, 0), ZV);
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
