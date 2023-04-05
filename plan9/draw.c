#include "strpg.h"
#include <thread.h>
#include <draw.h>

typedef Vertex Point;
typedef Quad Rectangle;

enum{
	Cbg,
	Ctext,
	Cend,
};
static Image *col[Cend];
static Point viewΔ, panmax;
static Rectangle viewr, hudr;
static Image *viewfb, *board;

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
	p = addpt(p, viewΔ);
	p = subpt(p, Pt(1,1));
	p = addpt(p, view.pan);
	if(!ptinrect(p, viewr))
		return Pt(-1,-1);
	return p;
}

// FIXME: custom fmt for linux??
char *
quadfmt_(Quad *r)
{
	static char buf[128];

	snprint(buf, sizeof buf, "%R", *r);
	return buf;
}
char *
vertfmt_(Vertex *v)
{
	static char buf[128];

	snprint(buf, sizeof buf, "%P", *v);
	return buf;
}

int
drawquad_(Quad r)
{
	draw(viewfb, r, col[Ctext], nil, ZP);
	return 0;
}

int
drawline_(Quad r, double w)
{
	line(viewfb, r.u, r.v, Endsquare, Endarrow, w, col[Ctext], ZP);
	return 0;
}

void
flushdraw_(void)
{
	draw(screen, screen->r, viewfb, nil, subpt(view.pan, viewΔ));
	flushimage(display, 1);
}

void
cleardraw_(void)
{
	//lockdisplay(display);
	// FIXME: unnecessary if view/pan is bounded
	draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(viewfb, viewfb->r, col[Cbg], nil, ZP);
	//unlockdisplay(display);
}

int
resetdraw_(void)
{
	viewr = Rpt(ZP, Pt(Dx(screen->r)+1, Dy(screen->r)+1));

	viewΔ = subpt(screen->r.max, screen->r.min);
	viewΔ = subpt(ZP, viewΔ);
	viewΔ = addpt(viewΔ, viewr.v);
	viewΔ = divpt(viewΔ, 2);
	if(-viewΔ.y < font->height * 2)
		viewΔ.y = 0;

	hudr.u = addpt(screen->r.min, subpt(Pt(2, viewr.v.y+2), viewΔ));
	hudr.v = addpt(hudr.u, Pt(screen->r.max.x, font->height*3));
	/* all the way up to here */
	freeimage(viewfb);
	viewfb = eallocimage(viewr, 0, DBlack);
	freeimage(board);
	board = eallocimage(viewr, 0, DBlack);
	return 0;
}

// FIXME: colors/styles do not belong here
int
initdraw_(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	//display->locking = 1;
	//unlockdisplay(display);
	col[Cbg] = display->black;
	col[Ctext] = display->white;
	view.w = Dx(screen->r);
	view.h = Dy(screen->r);
	return 0;
}
