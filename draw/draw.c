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
drawedge(Edge *e, Quad r)
{
	return drawline(r, e->w);
}

static int
drawnode(Quad r)
{
	dprint("drawnode %d,%d,%d,%d\n", r.u.x, r.u.y, r.v.x, r.v.y);
	return drawquad(r);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Quad r;

	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		r = vx2r(e->u->q.u, e->v->q.u);
		r = scaletrans(r, view.zoom, view.vpan);
		drawedge(e, r);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Quad r;
	Node *u, *ue;

	warn("dim %s\n", vertfmt_(&g->dim));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		r = scaletrans(u->q, view.zoom, view.vpan);
		drawnode(r);
	}
	return 0;
}

// FIXME: for all graphs, same for the others
static void
drawworld(Graph *g)
{
	warn("drawworld %#p\n", g);
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

void
centerdraw(Vertex v)
{
	Vertex p;

	p.x = view.pan.x + (view.w - v.x) / 2;
	p.y = view.pan.y + (view.h - v.y) / 2;
	view.vpan = p;
}

int
redraw(Graph *g)
{
	cleardraw();
	centerdraw(g->dim);
	drawworld(g);
	return updatedraw(g);
}

int
resetdraw(void)
{
	return resetdraw_();
}

int
initvdraw(void)
{
	return initdraw_();
}
