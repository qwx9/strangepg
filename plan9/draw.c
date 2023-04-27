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

Rectangle dim;

int
drawquad(Quad q)
{
	Rectangle r;

	q.v = addpt2(q.o, q.v);
	r = Rpt(v2p(mulpt2(q.o, view.zoom)), v2p(mulpt2(q.v, view.zoom)));
	if(debug){
		if(dim.min.x > r.min.x)
			dim.min.x = r.min.x;
		if(dim.max.x < r.max.x)
			dim.max.x = r.max.x;
		if(dim.min.y > r.min.y)
			dim.min.y = r.min.y;
		if(dim.max.y < r.max.y)
			dim.max.y = r.max.y;
	}
	r = rectaddpt(r, v2p(view.center));
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
	u = addpt2(mulpt2(u, view.zoom), view.center);
	v = addpt2(mulpt2(v, view.zoom), view.center);
	line(viewfb, v2p(u), v2p(v), Endsquare, showarrows ? Endarrow : Endsquare, w, col[Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	Point o;

	o = v2p(view.pan);
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(screen, screen->r, viewfb, nil, o);
	flushimage(display, 1);
}

void
cleardraw(void)
{
	Graph *g;
	Rectangle r, q;

	dim = ZR;
	r = Rpt(ZP, v2p(view.dim.v));
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
	viewr = Rpt(ZP, v2p(view.dim.v));
	dprint("resetdraw %R\n", viewr);
	hudr.min = addpt(screen->r.min, Pt(2, Dy(screen->r)+2));
	hudr.max = addpt(hudr.min, Pt(Dx(screen->r), font->height*3));
	freeimage(viewfb);
	viewfb = eallocimage(viewr, 0, DNofill);
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
	col[Cnode] = eallocimage(Rect(0,0,1,1), screen->chan, DYellow);
	col[Cedge] = eallocimage(Rect(0,0,1,1), screen->chan, 0x777777FF);
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	return 0;
}
