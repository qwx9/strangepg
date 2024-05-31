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
		u->pos.x = -Vdefw / 2 + nrand(Vdefw);
		u->pos.y = -Vdefh / 2 + nrand(Vdefh);
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
