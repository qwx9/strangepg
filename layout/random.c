#include "strpg.h"
#include "layout.h"

static void *
new(Graph *g)
{
	int x, y;
	ssize i;
	Node *u;

	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		x = -view.dim.v.x / 2 + nrand(view.dim.v.x);
		y = -view.dim.v.y / 2 + nrand(view.dim.v.y);
		u->vrect = Qd(Vec2(x, y), ZV);
	}
	return nil;
}

static Shitkicker ll = {
	.name = "random",
	.new = new,
};

Shitkicker *
regrandom(void)
{
	return &ll;
}
