#include "strpg.h"
#include "drawprv.h"

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
	Node *u, *v;
	Quad r;

	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		if((u = id2n(g, e->u >> 1)) == nil
		|| (v = id2n(g, e->v >> 1)) == nil)
			return -1;
		r = vx2r(u->q.v, v->q.u);

		/*
		p = e->p->q;
		q = e->q->q;
		Δu = Vx(dxvx(p), dyvx(p));
		Δv = Vx(dxvx(q), dyvx(q));
		p = quadaddvx(p, scalevx(Δu, 0.5));
		q = quadaddvx(q, scalevx(Δv, 0.5));
		//q = quadaddvx(q, Δv);
		r = vx2r(p.p, q.p);
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

static void
drawworld(void)
{
	Graph *g;

	for(g=graphs; g<graphs+ngraphs; g++){
		if(g->ll != nil)	// FIXME: weak check
			continue;
		dprint("update %#p\n", g);
		drawnodes(g);
		drawedges(g);
	}
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
updatedraw(void)
{
	drawui();
	flush();
	return 0;
}

void
centerdraw(void)
{
	Graph *g;
	Vertex p, v;

	v = ZV;
	for(g=graphs; g<graphs+ngraphs; g++){
		v = g->dim;
		if(v.x > p.x)
			p.x = v.x;
		if(v.y > p.y)
			p.y = v.y;
	}
	p.x = view.pan.x + (view.dim.v.x - p.x) / 2;
	p.y = view.pan.y + (view.dim.v.y - p.y) / 2;
	view.vpan = p;
}

int
redraw(void)
{
	dprint("redraw\n");
	cleardraw();
	rendernew();
	centerdraw();
	drawworld();
	return updatedraw();
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
