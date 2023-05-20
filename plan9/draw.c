#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include "drw.h"

QLock drawlock;

typedef struct Pal Pal;
struct Pal{
	u32int col;
	Image *i;
	Image *alt;
};

static Pal nodepal[] = {
	/* 12 class paired */
	{0x1f78b4ff, nil, nil},
	{0x33a02cff, nil, nil},
	{0xe31a1cff, nil, nil},
	{0xff7f00ff, nil, nil},
	{0x6a3d9aff, nil, nil},
	{0xb15928ff, nil, nil},
	/* some bandage */
	{0x8080ffff, nil, nil},
	{0x8ec65eff, nil, nil},
	{0xc76758ff, nil, nil},
	{0xc893f0ff, nil, nil},
	{0xca9560ff, nil, nil},
	{0x7f5f67ff, nil, nil},
	{0xb160c9ff, nil, nil},
	{0x5fc69fff, nil, nil},
	{0xc96088ff, nil, nil},
	/* 12 class set3 */
	{0x8dd3c7ff, nil, nil},
	{0xbebadaff, nil, nil},
	{0xfb8072ff, nil, nil},
	{0x80b1d3ff, nil, nil},
	{0xfdb462ff, nil, nil},
	{0xb3de69ff, nil, nil},
	{0xfccde5ff, nil, nil},
	{0xd9d9d9ff, nil, nil},
	{0xbc80bdff, nil, nil},
	{0xccebc5ff, nil, nil},
	{0xffed6fff, nil, nil},
	{0xffffb3ff, nil, nil},
	/* 12 class paired, pale counterparts */
	{0xa6cee3ff, nil, nil},
	{0xb2df8aff, nil, nil},
	{0xfb9a99ff, nil, nil},
	{0xfdbf6fff, nil, nil},
	{0xcab2d6ff, nil, nil},
	{0xffff99ff, nil, nil},
};

enum{
	Cscr,
	Cbg,
	Ctext,
	Cnode,
	Cnodesh,
	Cnodesh2,
	Cedge,
	Cedgesh,
	Cemph,
	Cend,
};
static Image *col[Cend];
static Point panmax;
static Rectangle viewr, hudr;
static Image *viewfb;

static Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
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

int
drawquad2(Quad q1, Quad q2, int sh, int c)
{
	Rectangle r1, r2;
	Image *cc;

	q1.v = addpt2(q1.o, q1.v);
	q2.v = addpt2(q2.o, q2.v);
	q1.v = addpt2(subpt2(mulpt2(q1.v, view.zoom), view.pan), view.center);
	q1.o = addpt2(subpt2(mulpt2(q1.o, view.zoom), view.pan), view.center);
	q2.v = addpt2(subpt2(mulpt2(q2.v, view.zoom), view.pan), view.center);
	q2.o = addpt2(subpt2(mulpt2(q2.o, view.zoom), view.pan), view.center);
	r1 = Rpt(v2p(q1.o), v2p(q2.v));
	r2 = Rpt(v2p(q2.o), v2p(q1.v));
	if(!rectXrect(r1, viewfb->r) && !rectXrect(r2, viewfb->r))
		return 0;
	cc = nodepal[c % nelem(nodepal)].i;
	Point p[] = {
		r1.max,
		r1.min,
		r2.min,
		r2.max,
		r1.max,
	};
	if(sh){
		polyop(viewfb, p, nelem(p), 0, 0, 1, cc, ZP, SatopD);
//		polyop(viewfb, p, nelem(p), 0, 0, 1, col[Cnodesh], ZP, SatopD);
//		polyop(viewfb, p, nelem(p), 1, 1, 2, col[Cnodesh2], ZP, SatopD);
	}else
		fillpoly(viewfb, p, nelem(p), ~0, cc, ZP);
	return 0;
}

int
drawquad(Quad q)
{
	Rectangle r;

	q.v = addpt2(q.o, q.v);
	q.v = addpt2(subpt2(mulpt2(q.v, view.zoom), view.pan), view.center);
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
	poly(viewfb, p, nelem(p), 0, 0, 2, col[Cemph], ZP);
	
	return 0;
}

int
drawbezier(Vertex u, Vertex v, double w)
{
	double θ;
	Point p2, p3;
	Rectangle r;

	// FIXME: wrappers for conversion + get rid of stupid origin+vec interface
	u = addpt2(subpt2(mulpt2(u, view.zoom), view.pan), view.center);
	v = addpt2(subpt2(mulpt2(v, view.zoom), view.pan), view.center);
	r = Rpt(v2p(u), v2p(v));
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	θ = atan2(u.x - v.x, u.y - v.y);
	// FIXME: adjustments for short edges and rotation
	if(r.min.x - r.max.x > ceil(4 * Nodesz * view.zoom))
		p2 = subpt(r.min, mulpt(Pt(Nodesz,Nodesz), θ));
	else
		p2 = addpt(r.min, mulpt(Pt(Nodesz,Nodesz), θ));
	if(r.min.y - r.max.y > ceil(4 * Nodesz * view.zoom))
		p3 = addpt(r.max, mulpt(Pt(Nodesz,Nodesz), θ));
	else
		p3 = subpt(r.max, mulpt(Pt(Nodesz,Nodesz), θ));
	bezier(viewfb, r.min, p2, p3, r.max, Endsquare,
		showarrows ? Endarrow : Endsquare, w, col[Cedge], ZP);
	bezier(viewfb, r.min, p2, p3, r.max, Endsquare,
		showarrows ? Endarrow : Endsquare, 1+w, col[Cedgesh], ZP);
	return 0;
}

int
drawline(Vertex u, Vertex v, double w, int emph)
{
	Rectangle r;

	u = addpt2(subpt2(mulpt2(u, view.zoom), view.pan), view.center);
	v = addpt2(subpt2(mulpt2(v, view.zoom), view.pan), view.center);
	r = Rpt(v2p(u), v2p(v));
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	line(viewfb, r.min, r.max, Endsquare, showarrows ? Endarrow : Endsquare,
		w, col[emph?Cemph:Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	drawop(screen, screen->r, col[Cscr], nil, ZP, S);
	drawop(screen, screen->r, viewfb, nil, ZP, SoverD);
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
	drawop(screen, screen->r, col[Cscr], nil, ZP, S);
	drawop(viewfb, viewr, col[Cbg], nil, ZP, S);
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
	viewfb = eallocimage(viewr, XRGB32, 0, DTransparent);
	draw(screen, screen->r, col[Cscr], nil, ZP);
	return 0;
}

// FIXME: colors/styles do not belong here
int
initdrw(void)
{
	Pal *p;

	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	if(!haxx0rz){
		col[Cscr] = display->white;
		col[Cbg] = eallocimage(Rect(0,0,1,1), XRGB32, 1, DTransparent);
		col[Ctext] = display->white;
		col[Cnode] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DYellow, 0xaf));
		col[Cnodesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DYellow, 0x2f));
		col[Cnodesh2] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DYellow, 0x10));
		col[Cedge] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DYellow, 0x10));
		col[Cedgesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0x777777ff, 0x3f));
		col[Cemph] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DRed, 0xaf));
	}else{
		col[Cscr] = display->black;
		col[Cbg] = eallocimage(Rect(0,0,1,1), XRGB32, 1, DNotacolor);
		col[Ctext] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0x5555557f, 0x7f));
		col[Cnode] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x7f));
		col[Cnodesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x4f));
		col[Cnodesh2] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x0f));
		col[Cedge] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0x333333ff, 0x3f));
		col[Cedgesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0x333333ff, 0x0f));
		col[Cemph] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DRed, 0xaf));
	}
	for(p=nodepal; p<nodepal+nelem(nodepal); p++)
		p->i = eallocimage(Rect(0,0,1,1), screen->chan, 1, p->col);
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	return 0;
}
