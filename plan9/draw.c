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

Rectangle dim;

int
drawquad(Quad q)
{
	Rectangle r;
	Point o;

	q.v = addpt2(q.o, q.v);

	r = Rpt(v2p(q.o), v2p(q.v));
	if(dim.min.x > r.min.x)
		dim.min.x = r.min.x;
	if(dim.max.x < r.max.x)
		dim.max.x = r.max.x;
	if(dim.min.y > r.min.y)
		dim.min.y = r.min.y;
	if(dim.max.y < r.max.y)
		dim.max.y = r.max.y;

	//rectaddpt(r, v2p(view.center));
	//quadaddpt2(q, view.center);
	//r = Rpt(v2p(q.o), v2p(q.v));
/*
FIXME: we already know what the bounds are before we do any drawing
we want g->dim to be the bounding area for our drawing
its coordinates are not pixel coordinates
we will translate it to have no negative coordinates or at least be centered
then, the screen is a second variable


so:
	- we have the graph's bounding box (or should)
	- we have the viewsize == viewfb rectangle
	- we have a panning offset (for flush)
what we want:
	- center of viewfb == center of graph box, or even a given node
		=> offset drawing into viewfb
unless we're ok with redrawing any time we pan,
	we must have a viewfb larger than the screen, or a constant size
	based on default lengths, etc.
*/



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

// NOTE: don't need to take into account edges for max dimensions,
// they'll never go beyond the nodes
int
drawline(Vertex u, Vertex v, double w)
{
	Point o;

	//u = addpt2(u, view.center);
	//v = addpt2(v, view.center);
	line(viewfb, v2p(u), v2p(v), Endsquare, Endarrow, w, col[Cedge], ZP);
	return 0;
}

void
flushdraw(void)
{
	Point o, z;

	line(viewfb, Pt(viewfb->r.max.x/2,0), Pt(viewfb->r.max.x/2,viewfb->r.max.y), Endsquare, Endarrow, 0, col[Ctext], ZP);
	line(viewfb, Pt(0,viewfb->r.max.y/2), Pt(viewfb->r.max.x,viewfb->r.max.y/2), Endsquare, Endarrow, 0, col[Ctext], ZP);

	Point pl[] = {
		dim.min,
		Pt(dim.max.x, dim.min.y),
		dim.max,
		Pt(dim.min.x, dim.max.y),
		dim.min
	};
	poly(viewfb, pl, nelem(pl), Endsquare, Endsquare, 0, col[Cnode], v2p(view.center));

	o = v2p(view.pan);
	draw(screen, screen->r, viewfb, nil, o);
	//draw(screen, rectsubpt(screen->r, o), viewfb, nil, ZP);
	flushimage(display, 1);
}

void
cleardraw(void)
{
	dim = Rpt(viewfb->r.max, viewfb->r.min);
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
