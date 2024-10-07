#include "strpg.h"
#include "layout.h"
#include "drw.h"

static void *
new(Graph *g)
{
	int x;
	RNode *r, *re;

	x = -(Nodesz + Ptsz) * (dylen(g->nodes) - 1) / 2;
	for(r=rnodes, re=r+dylen(r); r<re; r++, x+=Nodesz+Ptsz){
		r->pos[0] = x;
		r->pos[1] = 0;
	}
	return nil;
}

static Target ll = {
	.name = "congaline",
	.new = new,
};

Target *
regconga(void)
{
	return &ll;
}
