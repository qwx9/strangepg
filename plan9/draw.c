#include "strpg.h"
#include "drw.h"
#include "threads.h"
#include <draw.h>

struct Color{
	u32int col;
	Image *i;
	Image *shad;
};

static Point panmax;
static Rectangle viewr, statr;
static Image *viewfb, *selfb;
static Channel *drawc, *ticc;
static QLock ticker;

static Image *
eallocimage(Rectangle r, uint chan, int repl, uint col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

void
freecolor(Color *c)
{
	khiter_t k;

	if(c == nil)
		return;
	k = kh_get(cmap, cmap, c->col);
	assert(k != kh_end(cmap));
	kh_del(cmap, cmap, k);
	freeimage(c->i);
	freeimage(c->shad);
	free(c);
}

static Color *
alloccolor(u32int col)
{
	Color *c;

	c = emalloc(sizeof *c);
	c->col = col;
	c->i = eallocimage(Rect(0,0,1,1), (view.flags & VFhaxx0rz) != 0 ? screen->chan : ARGB32, 1, col);
	c->shad = eallocimage(Rect(0,0,1,1), XRGB32, 1, setalpha(col, 0x7f));
	return c;
}

Color *
newcolor(u32int v)
{
	Color *c;

	v = v << 8 | 0xff;
	c = alloccolor(v);
	return c;
}

u32int
col2int(Color *c)
{
	return c->col >> 8;
}

#define centerscalev(v)	addv(subv(mulv((v), view.zoom), view.pan), view.center)

void
zoomdraw(float)
{
}

void
pandraw(float, float)
{
}

FIXME
#define	v2p(v)	Pt((v).x, (v).y)
/* FIXME: don't even think about it */
/*
Quad
centerscalequad(Quad q)
{
	q.v = addpt2(q.o, q.v);
	q.o = centerscalev(q.o);
	q.v = centerscalev(q.v);
	return q;
}
*/
/* FIXME:
Vertex ZV;
Quad ZQ;

Quad
Qd(Vertex o, Vector v)
{
	return (Quad){o, v};
}

double
qΔx(Quad q)
{
	return q.v.x - q.o.x;
}

double
qΔy(Quad q)
{
	return q.v.y - q.o.y;
}

int
ptinquad(Vertex v, Quad q)
{
	return v.x >= q.o.x && v.x < q.o.x + q.v.x
		&& v.y >= q.o.y && v.y < q.o.y + q.v.y;
}

Quad
insetquad(Quad q, int Δ)
{
	return (Quad){
		Vec2(q.o.x - Δ, q.o.y - Δ),
		Vec2(q.v.x + Δ, q.v.y + Δ)
	};
}

Quad
quadaddpt2(Quad q, Vector v)
{
	return (Quad){addpt2(q.o, v), addpt2(q.v, v)};
}

Quad
quadsubpt2(Quad q, Vector v)
{
	return (Quad){addpt2(q.o, v), subpt2(q.v, v)};
}

Vertex
floorpt2(Vertex v)
{
	return Vec2(floor(v.x), floor(v.y));
}

int
eqpt2(Point2 a, Point2 b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w;
}
*/

FIXME
static Rectangle
centerscalerect(Quad q)
{
	q = centerscalequad(q);
	return Rpt(v2p(q.o), v2p(q.v));
}

u32int
scrobj(int x, int y)
{
	Rectangle r;
	union { uchar u[4]; u32int v; } u;

	if(selfb == nil)
		return 0;
	r = Rpt(Pt(x, y), Pt(x+1, y+1));
	r = rectsubpt(r, screen->r.min);
	unloadimage(selfb, r, u.u, 4);
	return u.v;
}

static Image *
i2c(s32int idx)
{
	static Image *i;
	union { uchar u[4]; u32int v; } u;

	u.v = (u32int)idx + 1;
	if(i == nil)
		i = eallocimage(Rect(0,0,1,1), XRGB32, 1, u.v);
	else
		loadimage(i, i->r, u.u, sizeof u.u);
	return i;
}

void
showobj(Obj *o)
{
	char s[128];
	Node *n;
	Edge *e;

	if(selected.type == Onil)
		return;
	switch(selected.type){
	case Oedge:
		e = o->g->edges + o->idx;
		snprint(s, sizeof s, "E[%zx] %zx,%zx", o->idx, e->u, e->v);
		break;
	case Onode:
		n = o->g->nodes + o->idx;
		snprint(s, sizeof s, "V[%zx] %zx", o->idx, n->id);
		break;
	}
	string(screen, statr.min, color(theme[Ctext])->i, ZP, font, s);
}

static void
rotatenode(Graph *g, Node *u)
{
	double θ, vx, vy, rx, ry, sx, sy;

	u->θ = makenicerkindof(g, u);
	vx = Nodesz * (cos(u->θ) - sin(u->θ));	/* x´ = x cosβ - y sinβ */
	vy = Nodesz * (sin(u->θ) + cos(u->θ));	/* y´ = x sinβ + y cosβ */
	u->vrect.v = Vec2(vx, vy);
	u->q1 = u->vrect;
	u->q2 = u->vrect;
	θ = u->θ;
	if(fabs(u->θ) >= PI/2)
		θ += PI/2;
	else
		θ -= PI/2;
	rx = Nodesz/8 * (cos(θ) - sin(θ));
	ry = Nodesz/8 * (sin(θ) + cos(θ));
	if(fabs(u->θ) >= PI/2)
		θ -= PI;
	else
		θ += PI;
	sx = Nodesz/4 * (cos(θ) - sin(θ));
	sy = Nodesz/4 * (sin(θ) + cos(θ));
	u->q1.o.x += rx;
	u->q1.o.y += ry;
	u->q1.v.x = u->q1.v.x + sx;
	u->q1.v.y = u->q1.v.y + sy;
	u->q2.o.x -= rx;
	u->q2.o.y -= ry;
	u->q2.v.x = u->q2.v.x - sx;
	u->q2.v.y = u->q2.v.y - sy;
	u->shape.o = u->vrect.o;	// FIXME: redundant??
	// FIXME: both dimensions are erroneous
	u->shape.v = Vec2(Nodesz+PI, Nodesz/4);	// FIXME: systematic error above
}

int
drawlabel(Node *n, Color *c)
{
	Vertex v;
	Point p;
	char lab[128];

	/* FIXME: note: maybe treat em allocations like strings like dy; have
	 * an emstr allocator with its own bank, etc. or selectable banks like
	 * doom */
	/* FIXME: just keep label in em/memory, what's the point */
	v = centerscalev(n->pos);
	p = Pt(v.x + Ptsz, v.y + Ptsz);
	snprint(lab, sizeof lab, "%zx", n->id);
	string(viewfb, p, c->i, ZP, font, lab);
	return 0;
}

int
drawquad(Vertex pos, float θ, s32int idx, Color *c)
{
	Rectangle r1, r2;

	q1 = centerscalequad(q1);
	q2 = centerscalequad(q2);
	r1 = Rpt(v2p(q1.o), v2p(q2.v));
	r2 = Rpt(v2p(q2.o), v2p(q1.v));
	if(!rectXrect(canonrect(r1), viewfb->r) && !rectXrect(canonrect(r2), viewfb->r))
		return 0;
	Point p[] = {
		r1.max,
		r1.min,
		r2.min,
		r2.max,
		r1.max,
	};
	polyop(viewfb, p, nelem(p), 0, 0, 1, c->shad, ZP, SatopD);
	fillpoly(viewfb, p, nelem(p), ~0, c->i, ZP);
	if(idx >= 0){
		poly(selfb, p, nelem(p), 0, 0, 1, i2c(idx), ZP);
		fillpoly(selfb, p, nelem(p), ~0, i2c(idx), ZP);
	}
	return 0;
}

int
drawbezier(Vertex a, Vertex b, double w, s32int idx, Color *c)
{
	double θ;
	Point p2, p3;
	Rectangle r;

	q.v = subpt2(q.v, q.o);	// FIXME (re-added by centerscalerect)
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
		showarrows ? Endarrow : Endsquare, w, c->i, ZP);
	// FIXME: remove haxx0rz and zoom check altogether?
	// FIXME: haxx0rz check doesn't belong here?
	if(!haxx0rz && view.zoom > 1.){
		bezier(viewfb, r.min, p2, p3, r.max, Endsquare,
			showarrows ? Endarrow : Endsquare, w+1, c->shad, ZP);
		if(idx >= 0)
			bezier(selfb, r.min, p2, p3, r.max, Endsquare,
				showarrows ? Endarrow : Endsquare, w+1, i2c(idx), ZP);
	}else if(idx >= 0)
		bezier(selfb, r.min, p2, p3, r.max, Endsquare,
			showarrows ? Endarrow : Endsquare, w, i2c(idx), ZP);
	return 0;
}

int
drawline(Vertex a, Vertex b, double w, int emph, s32int idx, Color *c)
{
	Rectangle r;

	r = centerscalerect(q);
	if(!rectXrect(canonrect(r), viewfb->r))
		return 0;
	line(viewfb, r.min, r.max,
		Endsquare, showarrows||emph ? Endarrow : Endsquare, w, c->i, ZP);
	if(idx >= 0)
		line(selfb, r.min, r.max,
			Endsquare, showarrows||emph ? Endarrow : Endsquare, w, i2c(idx), ZP);
	return 0;
}

static void
flushdraw(void)
{
	drawop(screen, screen->r, viewfb, nil, ZP, SoverD);
	drawui();
	flushimage(display, 1);
}

static void
resetdraw(void)
{
	view.w = Dx(screen->r);
	view.h = Dy(screen->r);
	viewr = rectsubpt(screen->r, screen->r.min);
	DPRINT(Debugdraw, "resetdraw %R", viewr);
	freeimage(viewfb);
	freeimage(selfb);
	viewfb = eallocimage(viewr, (view.flags & VFhaxx0rz) != 0 ? screen->chan : XRGB32, 0, DNofill);
	selfb = eallocimage(viewr, XRGB32, 0, DBlack);
	statr.min = addpt(screen->r.min, Pt(0, viewr.max.y - font->height));
	statr.max = viewr.max;
}

void
cleardraw(void)
{
	drawop(viewfb, viewr, color(theme[Cbg])->i, nil, ZP, S);
	draw(selfb, selfb->r, display->black, nil, ZP);
}

// FIXME: → merge with evloop in generic draw code as frame() or sth
static void
drawproc(void *)
{
	ulong req;
	static Clk clk = {.lab = "flush"};

	resetdraw();
	for(;;){
		if((req = recvul(drawc)) == 0)
			break;
		if((req & Reqresetdraw) != 0){
			lockdisplay(display);
			resetdraw();
			unlockdisplay(display);
		}
		if((req & Reqresetui) != 0)
			resetui(1);
		if((req & Reqrefresh) != 0 && !norefresh || (req & Reqshape) != 0){
			if(!redraw((req & Reqrefresh) != 0 && !norefresh || (req & Reqshape) != 0)
				stopdrawclock();
		}
		/* FIXME: long lock */
		/* FIXME: better to put these in every single drawquad/bezier call? */
		if(req != Reqshallowdraw){
			lockdisplay(display);
			if(!redraw((req & Reqrefresh) != 0 && !norefresh || (req & Reqshape) != 0)
				stopdrawclock();
			unlockdisplay(display);
		}
		CLK0(clk);
		lockdisplay(display);
		flushdraw();
		unlockdisplay(display);
		CLK1(clk);
	}
}

/* throttling of draw requests happens here */
void
reqdraw(int r)
{
	static ulong f;

	f |= r;
	if(nbsendul(drawc, f) != 0)
		f = 0;
}

/* FIXME: portable code */
static void
ticproc(void *)
{
	ulong stop;
	double t0, step;
	vlong t, Δt;

	t0 = μsec();
	step = Nsec / 60.;
	Alt a[] = {
		{ticc, &stop, CHANRCV},
		{nil, nil, CHANEND},
	};
	for(;;){
		switch(alt(a)){
		case -1: return;
		case 0:
			if(stop){
				a[1].op = CHANEND;
				continue;
			}
			a[1].op = CHANNOBLK;
			break;
		}
		t = μsec();
		Δt = t - t0;
		t0 += step * (1 + Δt / step);
		if(Δt < step)
			sleep((step - Δt) / Nmsec);
		reqdraw(Reqrefresh);
	}
}

void
stopdrawclock(void)
{
	sendul(ticc, 1);
}

void
startdrawclock(void)
{
	sendul(ticc, 0);
}

// FIXME: portable code
void
initsysdraw(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	display->locking = 1;
	unlockdisplay(display);
	view.w = Dx(screen->r);
	view.h = Dy(screen->r);
	if((drawc = chancreate(sizeof(ulong), 0)) == nil
	|| (ticc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	newthread(ticproc, nil, nil, nil, "tic", mainstacksize);
	newthread(drawproc, nil, nil, nil, "draw", mainstacksize);
}
