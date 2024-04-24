#include "strpg.h"
#include "layout.h"
#include "drw.h"

static void *
new(Graph *g)
{
	ssize i;
	Node *u;

	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		u->pos.x = -view.w / 2 + nrand(view.w);
		u->pos.y = -view.h / 2 + nrand(view.h);
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
