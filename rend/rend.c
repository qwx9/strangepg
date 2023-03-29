#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

// FIXME: get rid of Render; just modify Layer .nodes
// FIXME: functional style? pipeline vector of nodes

// FIXME: layout: add an angle or 

enum{
	Nodesz = 2 * 10,
	Ptsz = 25,
};

static int                                                            
rendershapes(Graph *g)
{
	Vnode *v, *ve;
	Vnode p;
	Vertex Δ;

	// FIXME: ridiculous
	USED(g);
	kv_init(g->r.nodes);
	for(v=&kv_A(g->l.nodes, 0), ve=v+kv_size(g->l.nodes); v<ve; v++){
		p.id = v->id & ~1 | SHrect;
		p.q = insetvx(scalevx(v->q.u, Ptsz), Nodesz/2);
		kv_push(Vnode, g->r.nodes, p);
		if(g->r.dim.u.x > p.q.u.x)
			g->r.dim.u.x = p.q.u.x;
		else if(g->r.dim.v.x < p.q.v.x)
			g->r.dim.v.x = p.q.v.x;
		if(g->r.dim.u.y > p.q.u.y)
			g->r.dim.u.y = p.q.u.y;
		else if(g->r.dim.v.y > p.q.v.y)
			g->r.dim.v.y = p.q.v.y;
	}
	Δ = ZV;
	if(g->r.dim.u.x < 0)
		Δ.x = -g->r.dim.u.x;
	if(g->r.dim.u.y < 0)
		Δ.y = -g->r.dim.u.y;
	g->r.dim = quadaddvx(g->r.dim, Δ);
	return 0;
}

// TODO: produce a vector of SDL lines and rectangles to draw
// TODO/draw: draw them; scale by zoom factor first


// FIXME: to avoid getting confused, document graph.c by adding
// more helper functions: getting edge by id, etc.
// hide some of the henglisms behind nicer syntax
// maybe even just a simple wrapper around kvec and khash
// with nicer semantics; also check out github issues/pulls
// for useful code to take

int
render(Graph *g)
{
	return rendershapes(g);
}

void
initrend(void)
{
}
