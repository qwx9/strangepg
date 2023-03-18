#include "strpg.h"
#include <thread.h>
#include <draw.h>

typedef Vertex Point;

enum{
	Cbg,
	Ctext,
	Cend,
};
static Image *col[Cend];
static Point viewΔ, panmax;
static Rectangle viewr, hudr;
static Image *viewbg, *board;

static Image *
eallocimage(Rectangle r, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, screen->chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

void
errmsg(char *fmt, ...)
{
	char s[256];
	va_list arg;
	Point p;

	va_start(arg, fmt);
	vseprint(s, s+sizeof s, fmt, arg);
	va_end(arg);
	p = addpt(screen->r.min, subpt(Pt(2, viewr.max.y+2), viewΔ));
	p.y += font->height * 2;
	string(screen, p, col[Ctext], ZP, font, s);
}

/* FIXME: layer violation */

Point
s2p(Point p)
{
	p = addpt(subpt(addpt(subpt(p, screen->r.min), viewΔ), Pt(1,1)), view.pan);
	if(!ptinrect(p, viewr))
		return Pt(-1,-1);
	return p;
}

static void
flipview(void)
{
	draw(screen, screen->r, viewbg, nil, addpt(viewΔ, view.pan));
	flushimage(display, 1);
}

static void
redraw(int clear)
{
	if(clear)
		draw(screen, screen->r, col[Cbg], nil, ZP);
	draw(viewbg, viewbg->r, board, nil, ZP);
}

void
updateview(void)
{
	lockdisplay(display);
	redraw(0);
	unlockdisplay(display);
	flipview();
}

void
resetview(void)
{
	viewr = Rpt(ZP, Pt(Dx(screen->r)+1, Dy(screen->r)+1));
	viewΔ = divpt(addpt(subpt(ZP, subpt(screen->r.max, screen->r.min)), viewr.max), 2);
	if(-viewΔ.y < font->height * 2)
		viewΔ.y = 0;
	hudr.min = addpt(screen->r.min, subpt(Pt(2, viewr.max.y+2), viewΔ));
	hudr.max = addpt(hudr.min, Pt(screen->r.max.x, font->height*3));
	/* all the way up to here */
	freeimage(viewbg);
	viewbg = eallocimage(viewr, 0, DBlack);
	freeimage(board);
	board = eallocimage(viewr, 0, DBlack);
	/* FIXME: in sequence, actual drawing later */
	updateview();
}

View
initsysview(void)
{
	View v = {0};

	// FIXME: just do sdl init
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	display->locking = 1;
	unlockdisplay(display);
	col[Cbg] = display->black;
	col[Ctext] = display->white;
	v.w = Dx(screen->r);
	v.h = Dy(screen->r);
	return v;
}
