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
static Channel *drawc, *ticc;
static int ttid = -1;

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

static Rectangle
centerscalerect(Quad q)
{
	q = centerscalequad(q);
	return Rpt(v2p(q.o), v2p(q.v));
}

int
drawquad2(Quad q1, Quad q2, int sh, int c)
{
	Rectangle r1, r2;
	Pal *cp;

	if(haxx0rz && (showarrows || sh))
		return 0;
	q1 = centerscalequad(q1);
	q2 = centerscalequad(q2);
	r1 = Rpt(v2p(q1.o), v2p(q2.v));
	r2 = Rpt(v2p(q2.o), v2p(q1.v));
	if(!rectXrect(r1, viewfb->r) && !rectXrect(r2, viewfb->r))
		return 0;
	cp = &nodepal[c % nelem(nodepal)];
	Point p[] = {
		r1.max,
		r1.min,
		r2.min,
		r2.max,
		r1.max,
	};
	if(sh){
		polyop(viewfb, p, nelem(p), 0, 0, 1, cp->alt, ZP, SatopD);
//		polyop(viewfb, p, nelem(p), 0, 0, 1, cp->alt, ZP, SatopD);
//		polyop(viewfb, p, nelem(p), 0, 0, 2, col[Cnodesh2], ZP, SatopD);
	}else
		fillpoly(viewfb, p, nelem(p), ~0, cp->i, ZP);
	return 0;
}

int
drawquad(Quad q, int w)
{
	Rectangle r;

	r = canonrect(centerscalerect(q));
	if(!rectXrect(r, viewfb->r))
		return 0;
	Point p[] = {
		r.min,
		Pt(r.max.x, r.min.y),
		r.max,
		Pt(r.min.x, r.max.y),
		r.min
	};
	poly(viewfb, p, nelem(p), 0, 0, w, col[Cemph], ZP);
	
	return 0;
}

int
drawbezier(Quad q, double w)
{
	double θ;
	Point p2, p3;
	Rectangle r;

	w -= 1.;
	q.v = subpt2(q.v, q.o);	// FIXME
	r = centerscalerect(q);
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	θ = atan2(r.min.x - r.max.x, r.min.y - r.max.y);
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
	if(!haxx0rz)
		bezier(viewfb, r.min, p2, p3, r.max, Endsquare,
			showarrows ? Endarrow : Endsquare, 1+w, col[Cedgesh], ZP);
	return 0;
}

int
drawline(Quad q, double w, int emph)
{
	Rectangle r;
	Image *c;

	r = centerscalerect(q);
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	switch(emph){
	case 0: c = col[Cedge]; break;
	case 1: c = col[Cemph]; break;
	default: c = nodepal[emph % nelem(nodepal)].i; break;
	}
	line(viewfb, r.min, r.max, Endsquare, showarrows||emph ? Endarrow : Endsquare,
		w, c, ZP);
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
	view.center = divpt2(view.dim.v, 2);
	viewr = rectsubpt(screen->r, screen->r.min);
	dprint("resetdraw %R\n", viewr);
	freeimage(viewfb);
	viewfb = eallocimage(viewr, haxx0rz ? screen->chan : XRGB32, 0, haxx0rz ? DNofill : DTransparent);
	draw(screen, screen->r, col[Cscr], nil, ZP);
	return 0;
}

static void
drawproc(void *)
{
	int req;
	Graph *g;

	enum{
		Aredraw,
		Arefresh,
		Aend,
	};
	Alt a[] = {
		[Aredraw] {drawc, &req, CHANRCV},
		[Arefresh] {ticc, &g, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	resetdraw();
	for(;;){
		switch(alt(a)){
		case Aredraw:
			switch(req){
			case Reqresetdraw: resetdraw(); resetui(0); redraw(); break;
			case Reqresetui: resetui(1); redraw(); break;
			case Reqredraw: redraw(); break;
			case Reqshallowdraw: shallowdraw(); break;
			default: sysfatal("drawproc: unknown redraw cmd %d\n", req);
			}
			break;
		case Arefresh: renderlayout(g); redraw(); break;
		}
	}
}

void
reqdraw(int r)
{
	nbsend(drawc, &r);
}

static void
ticproc(void *)
{
	int n;
	vlong t, t0, Δt, step;
	Graph *g;

	t0 = nsec();
	step = drawstep ? Nsec/1000 : Nsec/72;
	for(;;){
		for(g=graphs, n=0; g<graphs+ngraphs; g++)
			if(g->layout.tid >= 0){
				dprint("tic: refresh %#p\n", g);
				sendp(ticc, g);
				n++;
			}
		if(n == 0)
			break;
		reqdraw(Reqredraw);
		t = nsec();
		Δt = t - t0;
		t0 += step * (1 + Δt / step);
		if(Δt < step)
			sleep((step - Δt) / 1000000);
	}
	ttid = -1;
	threadexits(nil);
}

void
startdrawclock(void)
{
	if(ttid >= 0)
		return;
	if((ttid = proccreate(ticproc, nil, mainstacksize)) < 0)
		sysfatal("proccreate ticproc: %r");
}

// FIXME: colors/styles do not belong here
int
initsysdraw(void)
{
	Pal *p;

	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	if(!haxx0rz){
		col[Cscr] = display->black;
		col[Cbg] = eallocimage(Rect(0,0,1,1), XRGB32, 1, DNotacolor);
		col[Ctext] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0x5555557f, 0x7f));
		col[Cnode] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x7f));
		col[Cnodesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x4f));
		col[Cnodesh2] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DBlue, 0x0f));
		col[Cedge] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0xbbbbbbff, 0x3f));
		col[Cedgesh] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(0xbbbbbbff, 0x0f));
		col[Cemph] = eallocimage(Rect(0,0,1,1), ARGB32, 1, setalpha(DRed, 0xdd));
	}else{
		col[Cscr] = display->black;
		col[Cbg] = eallocimage(Rect(0,0,1,1), XRGB32, 1, DTransparent);
		col[Ctext] = display->black;
		col[Cnode] = eallocimage(Rect(0,0,1,1), screen->chan, 1, DYellow);
		col[Cnodesh] = eallocimage(Rect(0,0,1,1), screen->chan, 1, DYellow);
		col[Cnodesh2] = eallocimage(Rect(0,0,1,1), screen->chan, 1, DYellow);
		col[Cedge] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777ff);
		col[Cedgesh] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777ff);
		col[Cemph] = eallocimage(Rect(0,0,1,1), screen->chan, 1, DRed);
	}
	for(p=nodepal; p<nodepal+nelem(nodepal); p++){
		p->i = eallocimage(Rect(0,0,1,1), haxx0rz ? screen->chan : ARGB32, 1, haxx0rz ? p->col : setalpha(p->col, 0xaa));
		p->alt = eallocimage(Rect(0,0,1,1), haxx0rz ? screen->chan : ARGB32, 1, haxx0rz ? p->col : setalpha(p->col, 0x3f));
	}
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	if((drawc = chancreate(sizeof(int), 2)) == nil
	|| (ticc = chancreate(sizeof(Graph*), 16)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(drawproc, nil, mainstacksize) < 0)
		sysfatal("proccreate drawproc: %r");
	return 0;
}
