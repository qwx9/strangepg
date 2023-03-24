#include "strpg.h"

/* .w will be used to modify the shape and isn't saved any more */

static void                                                            
rendershapes(Graph *g, Layer *l, Render *r)
{
	Vnode *v, *ve;
	Vedge *e, *ee;
	Shape s;

	USED(g);
	kv_init(r->shapes);
	for(v=&kv_A(l->nodes, 0), ve=v+kv_size(l->nodes); v<ve; v++){
		s = (Shape){v->id & ~1 | SHrect, insetvx(v->v, -1)};
		kv_push(Shape, r->shapes, s);
	}
	for(e=&kv_A(l->edges, 0), ee=e+kv_size(l->edges); e<ee; e++){
		s = (Shape){e->id & ~1 | SHline, vx2r(e->u, e->v)};
		kv_push(Shape, r->shapes, s);
	}
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
