#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

// FIXME: functional style? pipeline vector of nodes

// FIXME: layout: add an angle or 

enum{
	Nodesz = 8,
	Ptsz = 2,
};

static int                                                            
rendershapes(Graph *g)
{
	int d;
	Node *u, *ue;
	Vertex *n;

	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		n = &u->q.u;
		u->q = insetvx(addvx(*n, (Vertex){Ptsz,Ptsz}), Nodesz/2);
		d = u->q.v.x + dxvx(u->q);
		if(g->dim.x < d)
			g->dim.x = d;
		d = u->q.v.y +  dyvx(u->q);
		if(g->dim.y < d)
			g->dim.y = d;
	}
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
