#include "strpg.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include <draw.h>

struct Color{
	u32int col;
	Image *i;
	Image *shad;
};

static Point panmax, ΔZP;
static Rectangle viewr, statr;
static Image *viewfb, *selfb;
static Channel *drawc, *ticc;
static QLock ticker;

#define v2p(v)	Pt((v).x, (v).y)

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
	if(col == DWhite)
		c->i = c->shad = display->white;
	else if(col == DBlack)
		c->i = c->shad = display->black;
	else{
		c->i = eallocimage(Rect(0,0,1,1), (view.flags & VFhaxx0rz) != 0 ? screen->chan : ARGB32, 1, setalpha(col, 0x9f));
		c->shad = eallocimage(Rect(0,0,1,1), XRGB32, 1, col);
	}
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

ioff
mousepick(int x, int y)
{
	Rectangle r;
	union { uchar u[8]; ioff v; } u;	/* libdraw bug */

	if(selfb == nil)
		return 0;
	r = Rect(x, y, x+1, y+1);
	r = rectaddpt(r, ΔZP);
	if(unloadimage(selfb, r, u.u, sizeof u.u) < 0)
		warn("mousepick: %r\n");
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
drawselected(void)
{
	ioff id;
	char s[128];
	Edge *e;
	Node *n;
	Graph *g;

	g = graphs;
	if(selected == -1)
		return;
	else if((selected & (1<<31)) == 0){
		id = (uint)selected;
		n = g->nodes + id;
		snprint(s, sizeof s, "V[%zx]", id);
		USED(n);
	}else{
		id = (uint)selected & ~(1<<31);
		e = g->edges + id;
		snprint(s, sizeof s, "E[%zx] %zx,%zx", id, e->u, e->v);
	}
	string(screen, statr.min, color(theme[Ctext])->i, ZP, font, s);
}

void
zoomdraw(float)
{
}

void
pandraw(float, float)
{
}

void
renderui(void)
{
}

int
drawlabel(Node *n, ioff idx, Color *c)
{
	Vertex v;
	Point p;
	char lab[128];

	v = centerscalev(n->pos);
	p = Pt(v.x + Ptsz, v.y + Ptsz);
	snprint(lab, sizeof lab, "%zx", idx);
	string(viewfb, p, c->i, ZP, font, lab);
	return 0;
}

/* FIXME: no prior screen intersection test */

int
drawquad(Vertex pos, Vertex dir, ioff idx, Color *c)
{
	float θ, cθ, sθ;
	Point p[5];

	θ = atan2(dir.y, dir.x);
	cθ = cos(θ);
	sθ = sin(θ);
	p[0] = v2p(centerscalev(addv(pos, zrotv(V(-Nodesz/2.0f, -Nodesz/4.0f, 0.0f), cθ, sθ))));
	p[1] = v2p(centerscalev(addv(pos, zrotv(V(+Nodesz/2.0f, -Nodesz/4.0f, 0.0f), cθ, sθ))));
	p[2] = v2p(centerscalev(addv(pos, zrotv(V(+Nodesz/2.0f, +Nodesz/4.0f, 0.0f), cθ, sθ))));
	p[3] = v2p(centerscalev(addv(pos, zrotv(V(-Nodesz/2.0f, +Nodesz/4.0f, 0.0f), cθ, sθ))));
	p[4] = p[0];
	polyop(viewfb, p, nelem(p), 0, 0, 1, c->shad, ZP, SatopD);
	fillpoly(viewfb, p, nelem(p), ~0, c->i, ZP);
	if(idx >= 0){
		poly(selfb, p, nelem(p), 0, 0, 1, i2c(idx), ZP);
		fillpoly(selfb, p, nelem(p), ~0, i2c(idx), ZP);
	}
	return 0;
}

int
drawbezier(Vertex a, Vertex b, ioff idx, Color *c)
{
	int w;
	float Δx, Δy;
	double θ;
	Point p[4];

	Δx = b.x - a.x;
	Δy = b.y - a.y;
	//θ = atan2(a.x - b.x, a.y - b.y);
	θ = atan2(Δy, Δx);
	p[0] = v2p(centerscalev(a));
	p[3] = v2p(centerscalev(b));
	/* FIXME: doesn't take into account opposite direction */
	if(Δx > 0.f)
		p[1] = addpt(p[0], mulpt(Pt(Nodesz,Nodesz), θ));
	else
		p[1] = subpt(p[0], mulpt(Pt(Nodesz,Nodesz), θ));
	if(Δy > 0.f)
		p[2] = addpt(p[3], mulpt(Pt(Nodesz,Nodesz), θ));
	else
		p[2] = subpt(p[3], mulpt(Pt(Nodesz,Nodesz), θ));
	//w = MAX(0., view.zoom/5);
	w = 0;
	bezier(viewfb, p[0], p[1], p[2], p[3], Endsquare,
		(view.flags & VFdrawarrows) != 0 ? Endarrow : Endsquare, w, c->i, ZP);
	bezier(viewfb, p[0], p[1], p[2], p[3], Endsquare,
		(view.flags & VFdrawarrows) != 0 ? Endarrow : Endsquare, w+1, c->shad, ZP);
	if(idx >= 0)
		bezier(selfb, p[0], p[1], p[2], p[3], Endsquare,
			(view.flags & VFdrawarrows) != 0 ? Endarrow : Endsquare, w, i2c(idx), ZP);
	return 0;
}

int
drawline(Vertex a, Vertex b, double w, int emph, ioff idx, Color *c)
{
	Point p[2];

	p[0] = v2p(centerscalev(a));
	p[1] = v2p(centerscalev(b));
	line(viewfb, p[0], p[1], Endsquare,
		(view.flags & VFdrawarrows) != 0 || emph ? Endarrow : Endsquare,
		w, c->i, ZP);
	if(idx >= 0)
		line(selfb, p[0], p[1], Endsquare,
			(view.flags & VFdrawarrows) != 0 || emph ? Endarrow : Endsquare,
			w, i2c(idx), ZP);
	return 0;
}

static void
flushdraw(void)
{
	drawop(screen, screen->r, viewfb, nil, ΔZP, SoverD);
	drawui();
	renderui();
	flushimage(display, 1);
}

static void
resetdraw(void)
{
	view.w = Dx(screen->r);
	view.h = Dy(screen->r);
	ΔZP = Pt(-view.w/2, -view.h/2);
	viewr = rectaddpt(rectsubpt(screen->r, screen->r.min), ΔZP);
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
			resetui();
		if((req & Reqrefresh) != 0 || (req & Reqshape) != 0){
			if(!redraw((req & Reqrefresh) != 0 || (req & Reqshape) != 0))
				stopdrawclock();
		}
		/* FIXME: long lock */
		/* FIXME: better to put these in every single drawquad/bezier call? */
		if(req != Reqshallowdraw){
			lockdisplay(display);
			if(!redraw((req & (Reqrefresh|Reqshape)) != 0))
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
