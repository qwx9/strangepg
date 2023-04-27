#include "strpg.h"
#include "drw.h"

View view;
int showarrows;

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

	// FIXME: orientation: at least choose a corner
	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		u = e2n(g, e->from);
		v = e2n(g, e->to);
		q = Qd(addpt2(u->q.o, u->q.v), v->q.o);
		drawedge(q, e->w);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Node *u, *ue;

	dprint("drawnodes dim %s\n", vertfmt(&g->dim.v));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++)
		drawnode(u->q);
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
	dprint("redraw %d\n", getpid());
	cleardraw();
	drawworld();
	return updatedraw();
}
