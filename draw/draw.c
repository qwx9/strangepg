#include "strpg.h"
#include "drw.h"

View view;
int showarrows, drawstep;

static Obj *visobj;

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

static void
drawguides(void)
{
	drawline(Qd(ZV, view.center), 0, 1, -1);
	drawline(Qd(ZV, view.pan), 0, 2, -1);
}

static int
mapvis(Graph *g, int type, int idx)
{
	Obj o;

	o = (Obj){g, type, idx};
	dypush(visobj, o);
	return dylen(visobj) - 1;
}

Obj
mouseselect(Vertex v)
{
	int i;

	if((i = scrobj(v)) < 0)
		return (Obj){nil, Onil, -1};
	assert(i < dylen(visobj));
	return visobj[i];
}

/* FIXME: the interfaces here need refactoring, too cumbersome and
 * redundant */
static int
drawedge(Graph *g, Quad q, double w, ssize idx)
{
	int i;

	DPRINT(Debugdraw, "drawedge %.1f,%.1f:%.1f,%.1f", q.o.x, q.o.y, q.v.x, q.v.y);
	i = mapvis(g, Oedge, idx);
	q.v = subpt2(q.v, q.o);	// FIXME
	return drawbezier(q, w, i);
}

static int
drawnode(Graph *g, Quad p, Quad q, Quad u, double θ, ssize id, ssize idx)
{
	int i;

	DPRINT(Debugdraw, "drawnode2 p %.1f,%.1f:%.1f,%.1f q %.1f,%.1f:%.1f,%.1f", p.o.x, p.o.y, p.v.x, p.v.y, q.o.x, q.o.y, q.v.x, q.v.y);
	i = mapvis(g, Onode, idx);
	if(drawquad2(p, q, u, θ, 1, idx, -1) < 0
	|| drawquad2(p, q, u, θ, 0, idx, i) < 0
	|| drawlabel(p, q, u, id) < 0)
		return -1;
	return 0;
}

static int
drawnodevec(Quad q)
{
	DPRINT(Debugdraw, "drawnodevec %.1f,%.1f:%.1f,%.1f", q.o.x, q.o.y, q.v.x, q.v.y);
	return drawline(q, MAX(0., view.zoom/5), 1, -1);
}

static int
drawedges(Graph *g)
{
	ssize i;
	Edge *e;
	Node *u, *v;
	Quad q;

	// FIXME: get rid of .o vertex + .v vector, just .min .max points or w/e
	for(i=g->edge0.next; i>=0; i=e->next){
		yield();
		e = g->edges + i;
		u = getnode(g, e->u >> 1);
		v = getnode(g, e->v >> 1);
		assert(u != nil && v != nil);
		q = Qd(addpt2(u->vrect.o, u->vrect.v), v->vrect.o);
		drawedge(g, q, MAX(0., view.zoom/5), e - g->edges);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	ssize i;
	Node *n;

	DPRINT(Debugdraw, "drawnodes dim %.1f,%.1f", g->dim.v.x, g->dim.v.y);
	for(i=g->node0.next; i>=0; i=n->next){
		yield();
		n = g->nodes + i;
		if(showarrows)
			drawnodevec(n->vrect);
		drawnode(g, n->q1, n->q2, n->shape, n->θ, n->id, n - g->nodes);
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
			drawline(Qd(ZV, g->off), 0, g - graphs + 3, -1);
	}
	if(debug)
		drawguides();
}

// FIXME: move more control shit from os-specific draw to here
//	+ clearer naming common vs. os

void
drawui(void)
{
	if(selected.type == Onil)
		return;
	showobj(&selected);
}

void
redraw(void)
{
	DPRINT(Debugdraw, "redraw");
	coffeetime();
	dyclear(visobj);
	cleardraw();
	drawworld();
	coffeeover();
	flushdraw();
}
