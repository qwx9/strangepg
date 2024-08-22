#include "strpg.h"
#include "layout.h"
#include "drw.h"

static void *
new(Graph *)
{
	int x;
	RNode *r, *re;

	for(x=0, r=rnodes, re=r+dylen(r); r<re; r++, x+=Nodesz+Ptsz){
		r->pos[0] = -Vdefw / 2 + nrand(Vdefw);
		r->pos[1] = -Vdefh / 2 + nrand(Vdefh);;
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
