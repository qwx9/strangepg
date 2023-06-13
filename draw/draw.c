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
	return (Quad){centerscalept2(q.o), centerscalept2(q.v)};
}

void
drawguides(void)
{
	drawline(Qd(ZV, view.center), 0, 1);
	drawline(Qd(ZV, view.pan), 0, 2);
}

// shitprint: maybe just do qk1's va()? but no custom fmt's

/* FIXME: the interfaces here need refactoring, too cumbersome and
 * redundant */
static int
drawedge(Quad q, double w)
{
	dprint("drawedge %.1f,%.1f:%.1f,%.1f\n", q.o.x, q.o.y, q.v.x, q.v.y);
	q.v = subpt2(q.v, q.o);	// FIXME
	q = centerscalequad(q);
	return drawbezier(q, w);
}

static int
drawnode(Quad p, Quad q, double θ, int c)
{
	dprint("drawnode2 p %.1f,%.1f:%.1f,%.1f q %.1f,%.1f:%.1f,%.1f\n", p.o.x, p.o.y, p.v.x, p.v.y, q.o.x, q.o.y, q.v.x, q.v.y);
	p = centerscalequad(p);
	q = centerscalequad(q);
	drawquad2(p, q, θ, 1, c);
	return drawquad2(p, q, θ, 0, c);
}

static int
drawnodevec(Quad q)
{
	dprint("drawnodevec %.1f,%.1f:%.1f,%.1f\n", q.o.x, q.o.y, q.v.x, q.v.y);
	q = centerscalequad(q);
	return drawline(q, 0, 1);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Node *u, *v;
	Quad q;

	// FIXME: get rid of .o vertex + .v vector, just .min .max points or w/e
	for(e=g->edges.buf, ee=e+g->edges.len; e<ee; e++){
		u = e2n(g, e->from);
		v = e2n(g, e->to);
		q = Qd(addpt2(u->vrect.o, u->vrect.v), v->vrect.o);
		drawedge(q, e->w);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Node *u, *ue;

	dprint("drawnodes dim %.1f,%.1f\n", g->dim.v.x, g->dim.v.y);
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		if(showarrows)
			drawnodevec(u->vrect);
		drawnode(u->q1, u->q2, u->θ, u - (Node *)g->nodes.buf);
	}
	return 0;
}

static void
drawworld(void)
{
	Graph *g;

	for(g=graphs; g<graphs+ngraphs; g++){
		if(g->layout.ll == nil)
			continue;
		dprint("drawworld: draw graph %#p\n", g);
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

/* threading and relaying commands is left up to the os stuff, screw it */
int
initalldraw(void)
{
	initsysdraw();
	return 0;
}
