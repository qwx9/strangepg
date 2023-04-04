#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

// FIXME: functional style? pipeline vector of nodes

// FIXME: layout: add an angle or 

enum{
	Nodesz = 2 * 10,
	Ptsz = 25,
};

static int                                                            
rendershapes(Graph *g)
{
	Node *u, *ue;
	Vertex Δ, *n;

	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		n = &u->q.u;
		u->q = insetvx(scalevx(*n, Ptsz), Nodesz/2);
		if(g->dim.u.x > n->x)
			g->dim.u.x = n->x;
		else if(g->dim.u.x < n->x)
			g->dim.u.x = n->x;
		if(g->dim.u.y > n->y)
			g->dim.u.y = n->y;
		else if(g->dim.u.y > n->y)
			g->dim.u.y = n->y;
	}
	Δ = ZV;
	if(g->dim.u.x < 0)
		Δ.x = -g->dim.u.x;
	if(g->dim.u.y < 0)
		Δ.y = -g->dim.u.y;
	g->dim = quadaddvx(g->dim, Δ);
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
