#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include "drw.h"

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
drawquad(Quad r)
{
	//draw(viewfb, r, col[Ctext], nil, ZP);
	Point p[] = {
		Pt(r.o.x, r.o.y),
		Pt(r.v.x, r.o.y),
		Pt(r.v.x, r.v.y),
		Pt(r.o.x, r.v.y),
		Pt(r.o.x, r.o.y)
	};
	poly(viewfb, p, nelem(p), Endsquare, Endsquare, 0, col[Cnode], ZP);
	return 0;
}

int
drawline(Vertex u, Vertex v, double w)
{
	line(viewfb, v2p(u), v2p(v), Endsquare, Endarrow, w, col[Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	draw(screen, screen->r, viewfb, nil, v2p(view.dim.o));
	flushimage(display, 1);
}

void
cleardraw(void)
{
	//lockdisplay(display);
	// FIXME: unnecessary if view/pan is bounded
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(viewfb, viewfb->r, col[Cbg], nil, ZP);
	//unlockdisplay(display);
}

int
resetdraw(void)
{
	viewr = Rpt(ZP, Pt(Dx(screen->r)+1, Dy(screen->r)+1));
	hudr.min = addpt(screen->r.min, Pt(2, viewr.max.y+2));
	hudr.max = addpt(hudr.min, Pt(screen->r.max.x, font->height*3));
	freeimage(viewfb);
	viewfb = eallocimage(viewr, 0, DBlack);
	return 0;
}

// FIXME: colors/styles do not belong here
int
initdrw(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	//display->locking = 1;
	//unlockdisplay(display);
	col[Cbg] = display->black;
	col[Ctext] = display->white;
	col[Cnode] = eallocimage(Rect(0,0,1,1), screen->chan, DYellow);
	col[Cedge] = eallocimage(Rect(0,0,1,1), screen->chan, 0x777777FF);
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	return 0;
}
