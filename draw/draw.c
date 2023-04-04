#include "strpg.h"
#include "drawprv.h"

/* interface:
 * take a quad layout
 * rotate rectangles (from quad)
 * draw rectangles based on their unit vector
 * draw edges: straight lines or bezier curves
 * color nodes
 */

View view;

static void
flush(void)
{
	flushdraw_();
}

static int
drawedge(Edge *e, Vquad *r)
{
	USED(e);
	return drawline(r->u, r->v, e->w);
}

static int
drawnode(Vquad *r)
{
	dprint("drawnode %d,%d,%d,%d\n", r->u.x, r->u.y, r->v.x, r->v.y);
	return drawquad(r->u, r->v);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Vquad r;

	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		r = vx2r(e->u->q.u, e->v->q.u);
		r = scaletrans(r, view.zoom, (Vertex){50,50});
		drawedge(e, &r);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Vquad r;
	Node *u, *ue;

	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		r = scaletrans(u->q, view.zoom, (Vertex){50,50});
		drawnode(&r);
	}
	return 0;
}

// FIXME: for all graphs, same for the others
static void
drawworld(Graph *g)
{
	drawnodes(g);
	drawedges(g);
}

static void
drawui(void)
{
}

static void
cleardraw(void)
{
	cleardraw_();
}

// FIXME: until there is a need to do drawing asynchronously,
// there's no need to distinguish draw and ui layers, it will
// all be redrawn anyway
int
updatedraw(Graph *g)
{
	USED(g);
	drawui();
	flush();
	return 0;
}

int
redraw(Graph *g)
{
	cleardraw();
	//centerdraw(g->r->dim);	// FIXME: center view, ie. default pan + zoom
	drawworld(g);
	return updatedraw(g);
}

int
resetdraw(Graph *g)
{
	resetdraw_();
	return redraw(g);
}

int
initvdraw(void)
{
	return initdraw_();
}
