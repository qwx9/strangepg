#include "strpg.h"
#include "layoutprv.h"

/* nodes are points in space, distances are in unit vectors */
/* edges: retrieved from graph; could be filtered, but not here */
/* representation: pretty much a binary map except we need node ids;
 * 	it's renderer's job to inflate nodes, draw's job to put edges */

Layer ZL;
static Layout *ll[LLnil];

/* FIXME: LSB already set? */
void
putnode(Layer *l, usize id, int x, int y)
{
	Vnode n;

	/* 0-size quad */
	n = (Vnode){id, (Vquad){(Vertex){x, y}, ZV}};
	kv_push(Vnode, l->nodes, n);
}

Layer
dolayout(Graph *g, int type)
{
	Layout *l;

	if(type < 0 || type >= LLnil){
		werrstr("invalid layout");
		return ZL;
	}
	l = ll[type];
	assert(l != nil);
	if(l->compute == nil){
		werrstr("unimplemented fs type");
		return ZL;
	}
	return l->compute(g);
}

void
initlayout(void)
{
	ll[LLconga] = regconga();
}
