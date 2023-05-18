#include "strpg.h"
#include "drw.h"

View view;
int showarrows, drawstep;

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
drawedge(Quad q, double θ, double w)
{
	Qd(q.o, addpt2(q.o, q.v));
	dprint("drawedge %s\n", shitprint('q', &q));
	return drawbezier(q.o, q.v, w, θ);
}

static int
drawnode(Quad q)
{
	int r;

	dprint("drawnode %s\n", shitprint('q', &q));
	r = drawquad(q);
	return r + drawline(q.o, addpt2(q.o, q.v), 0, 1);
}

static int
drawedges(Graph *g)
{
	double Δθ;
	Edge *e, *ee;
	Node *u, *v;
	Quad q;

	// FIXME: orientation: at least choose a corner
	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		u = e2n(g, e->from);
		v = e2n(g, e->to);
		Δθ = fabs(u->θ - v->θ);
		q = Qd(addpt2(u->q.o, u->q.v), v->q.o);
		drawedge(q, Δθ, e->w);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Node *u, *ue;

	dprint("drawnodes dim %s\n", shitprint('v', &g->dim.v));
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
		drawedges(g);
		drawnodes(g);
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
