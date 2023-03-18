#include "strpg.h"
#include "layoutprv.h"

/* nodes are points in space, distances are in unit vectors */

Layer ZL;
static Layout *ll[LLnil];

/* FIXME: LSB already set? */
void
putnode(Layer *l, usize id, int x, int y, double w)
{
	Vnode n;

	n = (Vnode){id, (Vertex){x, y}, w};
	kv_push(Vnode, l->nodes, n);
}

void
putedge(Layer *l, usize id, int x1, int y1, int x2, int y2, double w)
{
	Vedge e;

	e = (Vedge){id, (Vertex){x1, y1}, (Vertex){x2, y2}, w};
	kv_push(Vedge, l->edges, e);
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
