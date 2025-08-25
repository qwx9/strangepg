#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"
#include "ui.h"
#include "threads.h"
#include <draw.h>

typedef struct FVertex FVertex;	/* FIXME: temp, interop with hmm */
struct FVertex{
	float X;
	float Y;
	float Z;
};

#define	V(x,y,z)	((FVertex){(x), (y), (z)})
#define	addv(u,v)	((FVertex){(u).X+(v).X, (u).Y+(v).Y, (u).Z+(v).Z})
#define	subv(u,v)	((FVertex){(u).X-(v).X, (u).Y-(v).Y, (u).Z-(v).Z})
#define	mulv(u,s)	((FVertex){(u).X*(s), (u).Y*(s), (u).Z*(s)})
#define zrotv(v,cosθ,sinθ)	V((v).X * (cosθ) - (v).Y * (sinθ), (v).X * (sinθ) + (v).Y * (cosθ), 0.0f)
#define centerscalev(v)	addv(subv(mulv((v), view.zoom), view.Δeye), view.center)

extern Channel *rendc;

struct Color{
	u32int col;
	Image *i;
	Image *shad;
};

static Point panmax, ΔZP;
static Rectangle viewr, statr;
static Image *viewfb, *selfb;
static Channel *ticc, *framec;
static QLock ticker;
static ioff selected;	/* FIXME */

#define v2p(v)	Pt((v).X, (v).Y)

static Image *
eallocimage(Rectangle r, uint chan, int repl, uint col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
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
		c->i = eallocimage(Rect(0,0,1,1), (drawing.flags & DFhaxx0rz) != 0 ? screen->chan : ARGB32, 1, setalpha(col, col & 0xff));
		c->shad = eallocimage(Rect(0,0,1,1), XRGB32, 1, col);
	}
	return c;
}

void
setcolor(float *col, u32int v)
{
	col[0] = v >> 24 & 0xff;
	col[1] = v >> 16 & 0xff;
	col[2] = v >> 8 & 0xff;
	col[3] = v & 0xff;
}

static Color *
v2col(float *col)
{
	u32int u;

	u = (int)col[0] << 24;
	u |= (int)col[1] << 16;
	u |= (int)col[2] << 8;
	u |= (int)col[3];
	return color(u);
}

Color *
newcolor(u32int v)
{
	return alloccolor(v);
}

void
endmove(void)
{
}

u32int
mousepick(int x, int y)
{
	Rectangle r;
	union { uchar u[8]; u32int v; } u;	/* libdraw bug */

	if(selfb == nil)
		return 0;
	r = Rect(x, y, x+1, y+1);
	r = rectaddpt(r, ΔZP);
	if(unloadimage(selfb, r, u.u, sizeof u.u) < 0)
		warn("mousepick: %r\n");
	return u.v;
}

static Image *
i2c(s32int i)
{
	static Image *im;
	union { uchar u[4]; u32int v; } u;

	u.v = (u32int)i + 1;
	if(i == 0)
		im = eallocimage(Rect(0,0,1,1), XRGB32, 1, u.v);
	else
		loadimage(im, im->r, u.u, sizeof u.u);
	return im;
}

void
drawselected(void)
{
	ioff id;
	char s[128];
	Node *n;

	if(selected == -1)
		return;
	else if((selected & (1<<31)) == 0){
		id = (uint)selected;
		n = nodes + id;
		snprint(s, sizeof s, "V[%x]", id);
		USED(n);
	}else{
		//id = (uint)selected & ~(1<<31);
		// FIXME: update
		//e = edges + id;
		//snprint(s, sizeof s, "E[%x] %x,%x", id, e->u, e->v);
	}
	string(screen, statr.min, color(theme[Ctext])->i, ZP, font, s);
}

void
zoomdraw(float, float, float)
{
}

void
pandraw(float, float)
{
}

void
rotdraw(Vertex)
{
}

static void
drawlabels(void)
{
	Point p;
	char lab[128];
	Color *c;
	FVertex v;
	RNode *r, *e;

	c = color(theme[Ctext]);
	for(r=rnodes, e=r+dylen(r); r<e; r++){
		v.X = r->pos[0];
		v.Y = r->pos[1];
		v = centerscalev(v);
		p = Pt(v.X + drawing.fatness, v.Y + drawing.fatness);
		snprint(lab, sizeof lab, "%zx", r - rnodes);
		string(viewfb, p, c->i, ZP, font, lab);
	}
}

/* FIXME: no prior screen intersection test */
int
drawquad(ioff i)
{
	float cθ, sθ;
	Point p[5];
	Color *c;
	FVertex pos, dir;
	RNode *v;

	v = rnodes + i;
	pos.X = v->pos[0];
	pos.Y = v->pos[1];
	dir.X = v->dir[0];
	dir.Y = v->dir[1];
	c = v2col(v->col);
	cθ = dir.X;
	sθ = dir.Y;
	p[0] = v2p(centerscalev(addv(pos, zrotv(V(-drawing.nodesz/2.0f, -drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[1] = v2p(centerscalev(addv(pos, zrotv(V(+drawing.nodesz/2.0f, -drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[2] = v2p(centerscalev(addv(pos, zrotv(V(+drawing.nodesz/2.0f, +drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[3] = v2p(centerscalev(addv(pos, zrotv(V(-drawing.nodesz/2.0f, +drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[4] = p[0];
	polyop(viewfb, p, nelem(p), 0, 0, 1, c->shad, ZP, SatopD);
	fillpoly(viewfb, p, nelem(p), ~0, c->i, ZP);
	// FIXME: ???
	if(i >= 0){
		poly(selfb, p, nelem(p), 0, 0, 1, i2c(i), ZP);
		fillpoly(selfb, p, nelem(p), ~0, i2c(i), ZP);
	}
	return 0;
}

/* FIXME: angling */
int
drawbezier(ioff i)
{
	int w;
	float Δx, Δy;
	double θ;
	Point p[4];
	FVertex a, b;
	Color *c;
	REdge *r;

	r = redges + i;
	a.X = r->pos1[0];
	a.Y = r->pos1[1];
	b.X = r->pos2[0];
	b.Y = r->pos2[1];
	Δx = b.X - a.X;
	Δy = b.Y - a.Y;
	//θ = atan2(a.x - b.X, a.Y - b.Y);
	c = color(theme[Cedge]);
	θ = atan2(Δy, Δx);
	p[0] = v2p(centerscalev(a));
	p[3] = v2p(centerscalev(b));
	/* FIXME: doesn't take into account opposite direction */
	if(Δx > 0.f)
		p[1] = addpt(p[0], mulpt(Pt(drawing.nodesz,drawing.nodesz), θ));
	else
		p[1] = subpt(p[0], mulpt(Pt(drawing.nodesz,drawing.nodesz), θ));
	if(Δy > 0.f)
		p[2] = addpt(p[3], mulpt(Pt(drawing.nodesz,drawing.nodesz), θ));
	else
		p[2] = subpt(p[3], mulpt(Pt(drawing.nodesz,drawing.nodesz), θ));
	//w = MAX(0., view.zoom/5);
	w = 0;
	bezier(viewfb, p[0], p[1], p[2], p[3], Endsquare,
		(drawing.flags & DFdrawarrows) != 0 ? Endarrow : Endsquare, w, c->i, ZP);
	bezier(viewfb, p[0], p[1], p[2], p[3], Endsquare,
		(drawing.flags & DFdrawarrows) != 0 ? Endarrow : Endsquare, w+1, c->shad, ZP);
	/* FIXME: ??? */
	if(i >= 0)
		bezier(selfb, p[0], p[1], p[2], p[3], Endsquare,
			(drawing.flags & DFdrawarrows) != 0 ? Endarrow : Endsquare, w, i2c(i), ZP);
	return 0;
}

static void
render(void)
{
	ioff i;
	Color *c;
	RNode *r, *re;
	REdge *e, *ee;

	c = color(theme[Cbg]);
	draw(viewfb, viewfb->r, c->i, nil, ZP);
	for(i=0,e=redges,ee=e+dylen(e); e<ee; e++, i++)
		drawbezier(i);
	for(i=0,r=rnodes,re=r+dylen(r); r<re; r++, i++)
		drawquad(i);
}

static void
drawui(void)
{
	if((drawing.flags & DFdrawlabels) != 0)
		drawlabels();
	drawselected();
}

static void
flush(void)
{
	drawop(screen, screen->r, viewfb, nil, ΔZP, SoverD);
	drawui();
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
	viewfb = eallocimage(viewr, (drawing.flags & DFhaxx0rz) != 0 ? screen->chan : XRGB32, 0, DNofill);
	selfb = eallocimage(viewr, XRGB32, 0, DBlack);
	statr.min = addpt(screen->r.min, Pt(0, viewr.max.y - font->height));
	statr.max = viewr.max;
}

static void
rendproc(void *)
{
	for(;;){
		if(recvul(framec) == 0)
			break;
		lockdisplay(display);
		render();
		flush();
		unlockdisplay(display);
	}
}

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
		nbsendul(framec, Reqrefresh);
	}
}

/* FIXME */
static void
rendproc(void *)
{
	int req, stop;

	/* FIXME */
	while(recvul(rendc) != Reqredraw)
		;
	resetdraw();
	newthread(rendproc, nil, nil, nil, "render", mainstacksize);
	sendul(ticc, 0);
	stop = 0;
	for(;;){
		if((req = recvul(rendc)) == 0)
			break;
		if((req & Reqredraw) != 0)
			sendul(ticc, 0);
		if((req & Reqresetdraw) != 0){
			lockdisplay(display);
			resetdraw();
			unlockdisplay(display);
		}
		if((req & Reqresetview) != 0)
			resetview();
		if(req != Reqshallowdraw && (stop = !redraw(stop)))
			sendul(ticc, 1);
		nbsendul(framec, Reqrefresh);
	}
}

void
initp9draw(void)
{
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	display->locking = 1;
	unlockdisplay(display);
	view.w = Dx(screen->r);
	view.h = Dy(screen->r);
	if((ticc = chancreate(sizeof(ulong), 0)) == nil
	|| (framec = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	newthread(ticproc, nil, nil, nil, "tic", mainstacksize);
	newthread(rendproc, nil, nil, nil, "draw", mainstacksize);
}
