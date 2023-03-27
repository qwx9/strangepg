#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

// FIXME: get rid of Render; just modify Layer .nodes
// FIXME: functional style? pipeline vector of nodes

// FIXME: layout: add an angle or 

enum{
	Nodesz = 2 * 10,
	Ptsz = 25,
};

static void                                                            
rendershapes(Graph *g, Layer *l, Render *r)
{
	Vnode *v, *ve;
	Vnode p;
	Vertex Δ;

	// FIXME: ridiculous
	USED(g);
	kv_init(r->nodes);
	for(v=&kv_A(l->nodes, 0), ve=v+kv_size(l->nodes); v<ve; v++){
		p.id = v->id & ~1 | SHrect;
		p.q = insetvx(scalevx(v->q.u, Ptsz), Nodesz/2);
		kv_push(Vnode, r->nodes, p);
		if(r->dim.u.x > p.q.u.x)
			r->dim.u.x = p.q.u.x;
		else if(r->dim.v.x < p.q.v.x)
			r->dim.v.x = p.q.v.x;
		if(r->dim.u.y > p.q.u.y)
			r->dim.u.y = p.q.u.y;
		else if(r->dim.v.y > p.q.v.y)
			r->dim.v.y = p.q.v.y;
	}
	Δ = ZV;
	if(r->dim.u.x < 0)
		Δ.x = -r->dim.u.x;
	if(r->dim.u.y < 0)
		Δ.y = -r->dim.u.y;
	r->dim = quadaddvx(r->dim, Δ);
}

// TODO: produce a vector of SDL lines and rectangles to draw
// TODO/draw: draw them; scale by zoom factor first


// FIXME: to avoid getting confused, document graph.c by adding
// more helper functions: getting edge by id, etc.
// hide some of the henglisms behind nicer syntax
// maybe even just a simple wrapper around kvec and khash
// with nicer semantics; also check out github issues/pulls
// for useful code to take

Render
render(Graph *g, Layer *l)
{
	Render r;

	rendershapes(g, l, &r);
	return r;
}

void
initrend(void)
{
}
