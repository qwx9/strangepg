#include "strpg.h"
#include "drw.h"

View view;
int showarrows, drawstep;

Vertex
centerscalept2(Vertex v)
{
	return addpt2(subpt2(mulpt2(v, view.zoom), view.pan), view.center);
	//return mulpt2(addpt2(subpt2(v, view.pan), view.center), view.zoom);
}

Quad
centerscalequad(Quad q)
{
	q.v = addpt2(q.o, q.v);
	q.o = centerscalept2(q.o);
	q.v = centerscalept2(q.v);
	return q;
}

void
drawguides(void)
{
	drawline(Qd(ZV, view.center), 0, 1);
	drawline(Qd(ZV, view.pan), 0, 2);
}

/* FIXME: the interfaces here need refactoring, too cumbersome and
 * redundant */
static int
drawedge(Quad q, double w)
{
	DPRINT(Debugdraw, "drawedge %.1f,%.1f:%.1f,%.1f", q.o.x, q.o.y, q.v.x, q.v.y);
	q.v = subpt2(q.v, q.o);	// FIXME
	return drawbezier(q, w);
}

static int
drawnode(Quad p, Quad q, Quad u, double θ, int c, vlong id)
{
	DPRINT(Debugdraw, "drawnode2 p %.1f,%.1f:%.1f,%.1f q %.1f,%.1f:%.1f,%.1f", p.o.x, p.o.y, p.v.x, p.v.y, q.o.x, q.o.y, q.v.x, q.v.y);
	if(drawquad2(p, q, u, θ, 1, c) < 0
	|| drawquad2(p, q, u, θ, 0, c) < 0
	|| drawlabel(p, q, u, id) < 0)
		return -1;
	return 0;
}

static int
drawnodevec(Quad q)
{
	DPRINT(Debugdraw, "drawnodevec %.1f,%.1f:%.1f,%.1f", q.o.x, q.o.y, q.v.x, q.v.y);
	return drawline(q, MAX(0., view.zoom/5), 1);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Node *u, *v;
	Quad q;

	// FIXME: get rid of .o vertex + .v vector, just .min .max points or w/e
	for(e=g->edges, ee=e+dylen(g->edges); e<ee; e++){
		yield();
		u = getinode(g, e->u >> 1);
		v = getinode(g, e->v >> 1);
		assert(u != nil && v != nil);
		q = Qd(addpt2(u->vrect.o, u->vrect.v), v->vrect.o);
		drawedge(q, MAX(0., view.zoom/5));
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Node *u, *ue;

	DPRINT(Debugdraw, "drawnodes dim %.1f,%.1f", g->dim.v.x, g->dim.v.y);
	for(u=g->nodes, ue=u+dylen(g->nodes); u<ue; u++){
		yield();
		if(showarrows)
			drawnodevec(u->vrect);
		drawnode(u->q1, u->q2, u->shape, u->θ, u - g->nodes, u->sid);
	}
	return 0;
}

static void
drawworld(void)
{
	Graph *g;

	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->layout.ll == nil)
			continue;
		DPRINT(Debugdraw, "drawworld: draw graph %#p", g);
		drawedges(g);
		drawnodes(g);
		if(debug)
			drawline(Qd(ZV, g->off), 0, g - graphs + 3);
	}
	if(debug)
		drawguides();
}

static void
drawui(void)
{
}

int
updatedraw(void)
{
	drawui();
	return 0;
}

int
shallowdraw(void)
{
	DPRINT(Debugdraw, "shallowdraw");
	return updatedraw();
}

int
redraw(void)
{
	DPRINT(Debugdraw, "redraw");
	cleardraw();
	drawworld();
	return updatedraw();
}
