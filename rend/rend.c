#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

// FIXME: functional style? pipeline vector of nodes
// FIXME: layout: add an angle or 

static int                                                            
rendershapes(Graph *g)
{
	Node *u, *ue;
	Quad d;
	Vertex p;

	d = Qd(view.dim.o, addpt2(view.dim.o, view.dim.v));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		p = u->q.o;
		if(p.x < d.o.x)
			d.o.x = p.x;
		else if(p.x > d.v.x)
			d.v.x = p.x;
		if(p.y < d.o.y)
			d.o.y = p.y;
		else if(p.y > d.v.y)
			d.v.y = p.y;
	}
	g->dim = Qd(d.o, subpt2(d.v, d.o));
	return 0;
}

// TODO: produce a vector of SDL lines and rectangles to draw
// TODO/draw: draw them; scale by zoom factor first

int
render(Graph *g)
{
	return rendershapes(g);
}

void
initrend(void)
{
}
