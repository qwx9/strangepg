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

// shitprint: maybe just do qk1's va()? but no custom fmt's

static int
drawedge(Quad q, double w)
{
	Qd(q.o, addpt2(q.o, q.v));
	dprint("drawedge %s\n", shitprint('q', &q));
	return drawbezier(q.o, q.v, w);
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
	return drawline(q.o, addpt2(q.o, q.v), 0, 1);
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
