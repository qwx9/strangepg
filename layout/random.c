#include "strpg.h"
#include "layout.h"
#include "drw.h"

static void *
new(Graph *g)
{
	ioff i, ie;
	Node *u;

	for(i=0, ie=dylen(g->nodes); i<ie; i++){
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
