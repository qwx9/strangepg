#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include "drw.h"

typedef Vertex Point;
typedef Quad Rectangle;

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
s2p(Vertex p)
{
	p = subpt(p, screen->r.min);
	p = subpt(p, Pt(1,1));
	p = addpt(p, view.pan);
	if(!ptinrect(p, viewr))
		return Pt(-1,-1);
	return p;
}

// FIXME: custom fmt for linux??
char *
quadfmt(Quad *r)
{
	static char buf[128];

	snprint(buf, sizeof buf, "%R", *r);
	return buf;
}
char *
vertfmt(Vertex *v)
{
	static char buf[128];

	snprint(buf, sizeof buf, "%P", *v);
	return buf;
}

int
drawquad(Quad r)
{
	//draw(viewfb, r, col[Ctext], nil, ZP);
	Point p[] = {
		(Point){r.u.x, r.u.y},
		(Point){r.v.x, r.u.y},
		(Point){r.v.x, r.v.y},
		(Point){r.u.x, r.v.y},
		(Point){r.u.x, r.u.y}
	};
	poly(viewfb, p, nelem(p), Endsquare, Endsquare, 0, col[Cnode], ZP);
	return 0;
}

int
drawline(Quad r, double w)
{
	line(viewfb, r.u, r.v, Endsquare, Endarrow, w, col[Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	draw(screen, screen->r, viewfb, nil, view.pan);
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
	hudr.u = addpt(screen->r.min, Pt(2, viewr.v.y+2));
	hudr.v = addpt(hudr.u, Pt(screen->r.max.x, font->height*3));
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
	view.dim.u = ZV;
	view.dim.v = (Vertex){Dx(screen->r), Dy(screen->r)};
	return 0;
}
