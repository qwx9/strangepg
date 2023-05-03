#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include "drw.h"

QLock drawlock;

enum{
	Cbg,
	Ctext,
	Cnode,
	Cedge,
	Cend,
};
static Image *col[Cend];
static Point panmax;
static Rectangle viewr, hudr;
static Image *viewfb;

static Image *
eallocimage(Rectangle r, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, screen->chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

static Vertex
p2v(Point p)
{
	return Vec2(p.x, p.y);
}

static Point
v2p(Vertex v)
{
	return Pt(v.x, v.y);
}

static Point
s2p(Vertex v)
{
	Point p;

	p = subpt(v2p(v), screen->r.min);
	p = subpt(p, Pt(1,1));
	p = addpt(p, v2p(view.dim.o));
	if(!ptinrect(p, viewr))
		return Pt(-1,-1);
	return p;
}

// FIXME: custom fmt for linux??
char *
quadfmt(Quad *r)
{
	static char buf[128];

	snprint(buf, sizeof buf, "[%.2f,%.2f][%.2f,%.2f]",
		r->o.x, r->o.y, r->v.x, r->v.y);
	return buf;
}
char *
vertfmt(Vertex *v)
{
	static char buf[128];

	snprint(buf, sizeof buf, "%.2f,%.2f", v->x, v->y);
	return buf;
}

int
drawquad(Quad q)
{
	Rectangle r;

	q.v = addpt2(subpt2(mulpt2(addpt2(q.o, q.v), view.zoom), view.pan), view.center);
	q.o = addpt2(subpt2(mulpt2(q.o, view.zoom), view.pan), view.center);
	r = canonrect(Rpt(v2p(q.o), v2p(q.v)));
	if(!rectXrect(r, viewfb->r))
		return 0;
	Point p[] = {
		r.min,
		Pt(r.max.x, r.min.y),
		r.max,
		Pt(r.min.x, r.max.y),
		r.min
	};
	poly(viewfb, p, nelem(p), Endsquare, Endsquare, 0, col[Cnode], ZP);
	return 0;
}

int
drawline(Vertex u, Vertex v, double w)
{
	Rectangle r;

	u = addpt2(subpt2(mulpt2(u, view.zoom), view.pan), view.center);
	v = addpt2(subpt2(mulpt2(v, view.zoom), view.pan), view.center);
	r = Rpt(v2p(u), v2p(v));
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	line(viewfb, r.min, r.max, Endsquare, showarrows ? Endarrow : Endsquare, w, col[Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(screen, screen->r, viewfb, nil, ZP);
	flushimage(display, 1);
}

void
cleardraw(void)
{
	Graph *g;
	Rectangle r, q;

	r = Rpt(ZP, v2p(addpt2(addpt2(view.dim.v, view.center), view.pan)));
	for(g=graphs; g<graphs+ngraphs; g++){
		g->off = ZV;
		dprint("cleardraw: graph %#p dim %.1f,%.1f\n", g, g->dim.v.x, g->dim.v.y);
		q = Rpt(v2p(g->dim.o), v2p(addpt2(g->dim.o, g->dim.v)));
		if(qΔx(g->dim) + Nodesz > Dx(r))
			r.max.x = qΔx(g->dim) + Nodesz + 1;
		if(qΔy(g->dim) + Nodesz > Dy(r))
			r.max.y = qΔy(g->dim) + Nodesz + 1;
		if(q.min.x < 0)
			g->off.x = -q.min.x + Nodesz;
		else if(q.min.x > 0)
			g->off.x = -q.min.x;
		if(q.min.y < 0)
			g->off.y = -q.min.y + Nodesz;
		else if(q.min.y > 0)
			g->off.y = -q.min.y;
	}
	if(!eqrect(r, viewfb->r)){
		view.dim.v = p2v(r.max);
		resetdraw();
	}
	// FIXME: unnecessary if view/pan is bounded
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(viewfb, r, col[Cbg], nil, ZP);
	if(debug){
		Point pl[] = {
			r.min,
			Pt(r.max.x, r.min.y),
			r.max,
			Pt(r.min.x, r.max.y),
			r.min
		};
		r = insetrect(r, 1);
		poly(viewfb, pl, nelem(pl), Endsquare, Endsquare, 0, col[Ctext], ZP);
		line(viewfb,
			Pt(viewfb->r.max.x/2, 0),
			Pt(viewfb->r.max.x/2, viewfb->r.max.y),
			Endsquare, Endarrow, 0, col[Ctext], ZP);
		line(viewfb,
			Pt(0, viewfb->r.max.y/2),
			Pt(viewfb->r.max.x,viewfb->r.max.y/2),
			Endsquare, Endarrow, 0, col[Ctext], ZP);
	}
}

int
resetdraw(void)
{
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	viewr = rectsubpt(screen->r, screen->r.min);
	dprint("resetdraw %R\n", viewr);
	freeimage(viewfb);
	viewfb = eallocimage(viewr, 0, DNofill);
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(viewfb, viewfb->r, col[Cbg], nil, ZP);
	return 0;
}

// FIXME: colors/styles do not belong here
int
initdrw(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	col[Cbg] = display->black;
	col[Ctext] = display->white;
	col[Cnode] = eallocimage(Rect(0,0,1,1), screen->chan, 0xffff00ff);
	col[Cedge] = eallocimage(Rect(0,0,1,1), screen->chan, 0x777777ff);
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	return 0;
}
