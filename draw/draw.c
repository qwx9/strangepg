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

static int
drawedge(Quad q, double w)
{
	dprint("drawedge %s\n", shitprint('q', &q));
	return drawbezier(q, w);
}

static int
drawnode(Quad p, Quad q, int c)
{
	dprint("drawnode2 %s %s\n", shitprint('q', &p), shitprint('q', &q));
	drawquad2(p, q, 1, c);
	return drawquad2(p, q, 0, c);
}

static int
drawnodevec(Quad q)
{
	dprint("drawnodevec %s\n", shitprint('q', &q));
	return drawline(q, 0, 1);
}

static int
drawedges(Graph *g)
{
	Edge *e, *ee;
	Node *u, *v;
	Quad q;

	// FIXME: orientation: at least choose a corner
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

	dprint("drawnodes dim %s\n", shitprint('v', &g->dim.v));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		if(showarrows)
			drawnodevec(u->vrect);
		drawnode(u->q1, u->q2, u - (Node *)g->nodes.buf);
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
