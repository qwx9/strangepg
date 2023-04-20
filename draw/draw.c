#include "strpg.h"
#include "drw.h"

View view;

/* top-left to center (kludge) */
static Quad
centernode(Quad q)
{
	dprint("centernode %s → ", quadfmt(&q));
	q.o = addpt2(q.o, divpt2(q.v, 2));
	dprint("%s\n", quadfmt(&q));
	return q;
}

static int
drawedge(Quad q, double w)
{
	return drawline(q.o, addpt2(q.o, q.v), w);
}

static int
drawnode(Quad q)
{
	dprint("drawnode %s\n", quadfmt(&q));
	return drawquad(q);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Node *u, *v;
	Quad q;

	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		if((u = id2n(g, e->from >> 1)) == nil
		|| (v = id2n(g, e->to >> 1)) == nil)
			return -1;
		q = Qd(v->q.o, subpt2(u->q.v, u->q.o));
		// FIXME: shouldn't have to do translation at all
		q = scaletrans(q, view.zoom, view.vpan);
		drawedge(q, e->w);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Quad q;
	Node *u, *ue;

	dprint("drawnodes dim %s\n", vertfmt(&g->dim.v));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		q = scaletrans(u->q, view.zoom, view.vpan);
		drawnode(q);
	}
	return 0;
}

static void
drawworld(void)
{
	Graph *g;

	for(g=graphs; g<graphs+ngraphs; g++){
		if(g->ll == nil)	// FIXME: weak check
			continue;
		dprint("drawworld: draw graph %#p\n", g);
		drawnodes(g);
		drawedges(g);
	}
}

static void
drawui(void)
{
}


// FIXME: until there is a need to do drawing asynchronously,
// there's no need to distinguish draw and ui layers, it will
// all be redrawn anyway
int
updatedraw(void)
{
	drawui();
	flushdraw();
	return 0;
}

void
centerdraw(void)
{
	Graph *g;
	Vector p, v;

	p = ZV;
	for(g=graphs; g<graphs+ngraphs; g++){
		v = g->dim.v;
		if(v.x > p.x)
			p.x = v.x;
		if(v.y > p.y)
			p.y = v.y;
	}
	p.x = (view.dim.v.x - p.x) / 2;
	p.y = (view.dim.v.y - p.y) / 2;
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
