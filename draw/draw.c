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

/* top-left to center (kludge) */
static Quad
centernode(Quad r)
{
	int dx, dy;

	dx = dxvx(r) / 2;
	dy = dyvx(r) / 2;
	dprint("centernode %s → ", quadfmt(&r));
	r = quadaddvx(r, Vx(dx, dy));
	dprint("%s\n", quadfmt(&r));
	return r;
}

static int
drawedge(Quad r, double w)
{
	return drawline(r, w);
}

static int
drawnode(Quad r)
{
	dprint("drawnode %s\n", quadfmt(&r));
	return drawquad(r);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Quad r, u, v;
	Vertex Δu, Δv;

	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		r = vx2r(e->u->q.v, e->v->q.u);
		USED(u, v, Δu, Δv);
		/*
		u = e->u->q;
		v = e->v->q;
		Δu = Vx(dxvx(u), dyvx(u));
		Δv = Vx(dxvx(v), dyvx(v));
		u = quadaddvx(u, scalevx(Δu, 0.5));
		v = quadaddvx(v, scalevx(Δv, 0.5));
		//v = quadaddvx(v, Δv);
		r = vx2r(u.u, v.u);
		*/

		r = scaletrans(r, view.zoom, view.vpan);
		drawedge(r, e->w);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Quad r;
	Node *u, *ue;

	warn("dim %s\n", vertfmt(&g->dim));
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
