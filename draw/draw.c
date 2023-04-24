#include "strpg.h"
#include "drw.h"

View view;

void
centergraph(Graph *g)
{
	Vector v;

	v = subpt2(g->dim.v, view.dim.v);
	if(v.x > 0)
		v.x /= 2;
	else
		v.x = 0;
	if(v.y > 0)
		v.y /= 2;
	else
		v.y = 0;
	view.center = addpt2(g->off, v);
}

static int
drawedge(Quad q, double w)
{
	Qd(q.o, addpt2(q.o, q.v));
	dprint("drawedge %s\n", quadfmt(&q));
	return drawline(q.o, q.v, w);
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
		q = Qd(addpt2(u->q.o, u->q.v), v->q.o);
		// FIXME: shouldn't have to do translation at all
		q = scaletrans(q, view.zoom, ZV);
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
		q = scaletrans(u->q, view.zoom, ZV);
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
		centergraph(g);
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

int
shallowdraw(void)
{
	dprint("shallowdraw\n");
	return updatedraw();
}

int
redraw(void)
{
	dprint("redraw\n");
	cleardraw();
	rendernew();
	drawworld();
	return updatedraw();
}
