#include "strpg.h"
#include "drw.h"
#include <draw.h>

QLock drawlock;
int norefresh;

struct Color{
	u32int col;
	Image *i;
	Image *shad;
};

static Point panmax;
static Rectangle viewr, statr;
static Image *viewfb, *selfb;
static Channel *drawc, *ticc;
static int ttid = -1;

static Image *
eallocimage(Rectangle r, uint chan, int repl, uint col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

// FIXME
u32int
p2col(Color *c, int alpha)
{
	return setalpha(c->col << 8 | 0xff, alpha);
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
	c->i = eallocimage(Rect(0,0,1,1), haxx0rz ? screen->chan : ARGB32, 1, col);
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
scrobj(Vertex p)
{
	Rectangle r;
	union { uchar u[4]; int v; } u;

	if(selfb == nil)
		return -1;
	r = Rpt(Pt(p.x,p.y), Pt(p.x+1, p.y+1));
	r = rectsubpt(r, screen->r.min);
	unloadimage(selfb, r, u.u, 4);
	return u.v - 1;
}

static Image *
i2c(int idx)
{
	static Image *i;
	union { uchar u[4]; int v; } u;

	u.v = idx + 1;
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

int
drawlabel(Node *, Quad, Quad, Quad q, vlong id, Color *c)
{
	char lab[128];

	q = centerscalequad(q);
	snprint(lab, sizeof lab, "%zx", id);
	string(viewfb, v2p(q.o), c->i, ZP, font, lab);
	return 0;
}

int
drawquad(Quad q1, Quad q2, Quad, double, int idx, Color *c)
{
	Rectangle r1, r2;

	if(haxx0rz && showarrows)
		return 0;
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
drawbezier(Quad q, double w, int idx, Color *c)
{
	double θ;
	Point p2, p3;
	Rectangle r;

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
drawline(Quad q, double w, int emph, int idx, Color *c)
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

void
flushdraw(void)
{
	drawop(screen, screen->r, viewfb, nil, ZP, SoverD);
	drawui();
	flushimage(display, 1);
}

static void
resetdraw(void)
{
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	viewr = rectsubpt(screen->r, screen->r.min);
	DPRINT(Debugdraw, "resetdraw %R", viewr);
	freeimage(viewfb);
	freeimage(selfb);
	viewfb = eallocimage(viewr, haxx0rz ? screen->chan : XRGB32, 0, DNofill);
	selfb = eallocimage(viewr, XRGB32, 0, DBlack);
	statr.min = addpt(screen->r.min, Pt(0, viewr.max.y - font->height));
	statr.max = viewr.max;
}

void
cleardraw(void)
{
	Graph *g;
	Rectangle r, q;

	r = Rpt(ZP, v2p(addpt2(addpt2(view.dim.v, view.center), view.pan)));
	lockgraphs(0);
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type == FFdead)
			continue;
		g->off = ZV;
		DPRINT(Debugdraw, "cleardraw: graph %#p dim %.1f,%.1f", g, g->dim.v.x, g->dim.v.y);
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
	// FIXME: ???
	if(!eqrect(r, viewfb->r)){
		view.dim.v = p2v(r.max);
		resetdraw();
	}
	drawop(viewfb, viewr, color(theme[Cbg])->i, nil, ZP, S);
	draw(selfb, selfb->r, display->black, nil, ZP);
	if(debug){
		Point pl[] = {
			r.min,
			Pt(r.max.x, r.min.y),
			r.max,
			Pt(r.min.x, r.max.y),
			r.min
		};
		r = insetrect(r, 1);
		poly(viewfb, pl, nelem(pl), Endsquare, Endsquare, 0, color(theme[Ctext])->i, ZP);
		line(viewfb,
			Pt(viewfb->r.max.x/2, 0),
			Pt(viewfb->r.max.x/2, viewfb->r.max.y),
			Endsquare, Endarrow, 0, color(theme[Ctext])->i, ZP);
		line(viewfb,
			Pt(0, viewfb->r.max.y/2),
			Pt(viewfb->r.max.x,viewfb->r.max.y/2),
			Endsquare, Endarrow, 0, color(theme[Ctext])->i, ZP);
	}
	unlockgraphs(0);
}

static void
drawproc(void *)
{
	int req;
	Graph *g;

	threadsetname("drawproc");
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
			lockdisplay(display);
			switch(req){
			case Reqresetdraw: resetdraw(); /* wet floor */
			case Reqresetui: resetui(1);	/* wet floor */
			case Reqredraw: redraw(); flushdraw(); break;
			case Reqshallowdraw: flushdraw(); break;
			case Reqrefresh: if(norefresh) break; rerender(1); redraw(); flushdraw(); break;
			default: sysfatal("drawproc: unknown redraw cmd %d\n", req);
			}
			unlockdisplay(display);
			break;
		case Arefresh:
			lockdisplay(display);
			renderlayout(g);
			redraw();
			flushdraw();
			unlockdisplay(display);
			break;
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

	threadsetname("ticproc");
	t0 = nsec();
	step = drawstep ? Nsec/140 : Nsec/60;
	for(;;){
		lockgraphs(0);
		for(g=graphs, n=0; g<graphs+dylen(graphs); g++){
			if(g->type == FFdead)
				continue;
			if(g->layout.tid >= 0){
				n++;
				unlockgraphs(0);
				sendp(ticc, g);
				lockgraphs(0);
			}
		}
		unlockgraphs(0);
		if(n == 0)
			break;
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

int
initsysdraw(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	display->locking = 1;
	unlockdisplay(display);
	view.dim.o = ZV;
	view.dim.v = Vec2(Dx(screen->r), Dy(screen->r));
	if((drawc = chancreate(sizeof(int), 1)) == nil
	|| (ticc = chancreate(sizeof(Graph*), 0)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(drawproc, nil, mainstacksize) < 0)
		sysfatal("proccreate drawproc: %r");
	return 0;
}
