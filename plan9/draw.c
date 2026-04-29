#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"
#include "ui.h"
#include "threads.h"
#include <draw.h>
#include "lib/khashl.h"

extern Channel *rendc;

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

typedef struct Color Color;

KHASHL_MAP_INIT(KH_LOCAL, Cmap, cm, u32int, Color*, kh_hash_uint32, kh_eq_generic)

struct Color{
	u32int col;
	Image *i;
	Image *shad;
};
Cmap *cmap;

static Point panmax, ΔZP;
static Rectangle viewr, statr;
static Image *viewfb, *selfb;
static Channel *ticc;
static QLock ticker;
static ioff selected;	/* FIXME */

static float *nodev;
static int nnodev;

#define v2p(v)	Pt((v).X, (v).Y)

#define FCOL(x) ((u8int)((x)[0] * 255.0) << 24 |\
	(u8int)((x)[1] * 255.0) << 16 |\
	(u8int)((x)[2] * 255.0) << 8 |\
	(u8int)((x)[3] * 255.0))

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

static Color *
colorimage(u32int col)
{
	int abs;
	khint_t k;
	Color *c;

	k = cm_get(cmap, col);
	if(k != kh_end(cmap))
		return kh_val(cmap, k);
	c = alloccolor(col);
	k = cm_put(cmap, col, &abs);
	assert(abs);
	kh_val(cmap, k) = c;
	return c;
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

/* FIXME: generalize/portable code */
void
setnodeshape(int arrow)
{
	float quadv3d[] = {
		-0.5f * drawing.nodesz,	+0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
	}, arrowv3d[] = {
		-0.1f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.0f * drawing.fatness, +0.0f * drawing.fatness, /* tip */
	}, quadv[] = {
		-0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
		-0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
	}, arrowv[] = {
		+0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	-0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	+0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	-1.5f * drawing.fatness,
		-0.5f * drawing.nodesz,	-0.0f * drawing.fatness,
		-0.0f * drawing.nodesz,	+1.5f * drawing.fatness,
	};

	if(drawing.flags & DF3d){
		if(arrow){
			nnodev = nelem(arrowv3d);
			free(nodev);
			nodev = emalloc(sizeof *arrowv3d);
			memcpy(nodev, arrowv3d, sizeof *arrowv3d);
		}else{
			nnodev = nelem(quadv3d);
			free(nodev);
			nodev = emalloc(sizeof *quadv3d);
			memcpy(nodev, quadv3d, sizeof *quadv3d);
		}
	}else{
		if(arrow){
			nnodev = nelem(arrowv);
			free(nodev);
			nodev = emalloc(sizeof *arrowv);
			memcpy(nodev, arrowv, sizeof *arrowv);
		}else{
			nnodev = nelem(quadv);
			free(nodev);
			nodev = emalloc(sizeof *quadv);
			memcpy(nodev, quadv, sizeof *quadv);
		}
	}
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
	string(screen, statr.min, colorimage(theme[Ctext])->i, ZP, font, s);
}

static void
drawlabels(void)
{
	Point p;
	char lab[128];
	Color *c;
	FVertex v;
	RNode *r, *e;

	c = colorimage(theme[Ctext]);
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
drawquad(RNode *v)
{
	ioff i;
	float *cp, *ce;
	Point p[5];	/* FIXME: arrows */
	Color *c;
	HMM_Vec3 g, pos;
	HMM_Vec4 m;
	HMM_Quat q;

	/* FIXME: 3d version */
	if(drawing.flags & DF3d)
		return -1;
	q = HMM_Q(v->dir[0], v->dir[1], v->dir[2], v->dir[3]);
	pos = HMM_V3(v->pos[0], v->pos[1], v->pos[2]);
	for(i=0, cp=nodev, ce=cp+nnodev; cp<ce; cp+=2, i++){
		g = HMM_V3(cp[0] * v->len, cp[1], 0.0f);
		g = HMM_RotateV3Q(g, q);
		g = HMM_AddV3(g, pos);
		m = HMM_MulM4V4(view.mvp, HMM_V4(g.X, g.Y, g.Z, 1.0f));
		/* FIXME: w2s, this doesn't work */
		g = HMM_SubV3(m.XYZ, view.center);
		g = HMM_RotateV3Q(g, HMM_InvQ(view.rot));
		g.X /= view.zoom * view.ar * view.fov;
		g.Y /= view.zoom * view.tfov;
		g.X += 1.0f;
		g.Y += 1.0f;
		g.X /= 2.0f / view.w;
		g.Y /= 2.0f / view.h;
		p[i].x = g.X;
		p[i].y = g.Y;
	}
	p[4] = p[0];
	c = colorimage(FCOL(v->col));
	/*
	pos.X = v->pos[0];
	pos.Y = v->pos[1];
	dir.X = v->dir[0];
	dir.Y = v->dir[1];
	cθ = dir.X;
	sθ = dir.Y;
	p[0] = v2p(centerscalev(addv(pos, zrotv(V(-drawing.nodesz/2.0f, -drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[1] = v2p(centerscalev(addv(pos, zrotv(V(+drawing.nodesz/2.0f, -drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[2] = v2p(centerscalev(addv(pos, zrotv(V(+drawing.nodesz/2.0f, +drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[3] = v2p(centerscalev(addv(pos, zrotv(V(-drawing.nodesz/2.0f, +drawing.nodesz/4.0f, 0.0f), cθ, sθ))));
	p[4] = p[0];
	*/
	polyop(viewfb, p, nelem(p), 0, 0, 1, c->shad, ZP, SatopD);
	fillpoly(viewfb, p, nelem(p), ~0, c->i, ZP);
	i = v - rnodes;
	poly(selfb, p, nelem(p), 0, 0, 1, i2c(i), ZP);
	fillpoly(selfb, p, nelem(p), ~0, i2c(i), ZP);
	return 0;
}

/* FIXME: angling */
int
drawbezier(REdge *r)
{
	int w;
	ioff i;
	float Δx, Δy;
	double θ;
	Point p[4];
	FVertex a, b;
	Color *c;

	a.X = r->pos1[0];
	a.Y = r->pos1[1];
	b.X = r->pos2[0];
	b.Y = r->pos2[1];
	Δx = b.X - a.X;
	Δy = b.Y - a.Y;
	//θ = atan2(a.x - b.X, a.Y - b.Y);
	c = colorimage(theme[Cedge]);
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
	i = r - redges;
	bezier(selfb, p[0], p[1], p[2], p[3], Endsquare,
		(drawing.flags & DFdrawarrows) != 0 ? Endarrow : Endsquare, w, i2c(i), ZP);
	return 0;
}

/* FIXME: 3d */
static void
renderscene(void)
{
	Color *c;
	RNode *r, *re;
	REdge *e, *ee;
	RLine *l, *le;

	c = colorimage(theme[Cbg]);
	draw(viewfb, viewfb->r, c->i, nil, ZP);
	/*
	for(e=redges, ee=e+dylen(e); e<ee; e++)
		drawbezier(e);
	*/
	for(r=rnodes, re=r+dylen(r); r<re; r++)
		drawquad(r);
	l = le = nil;
	USED(l, le);
	/* FIXME
	for(l=rlines; nl>0; l++, nl--)
		drawline(l);
	*/
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

void
resizedraw(void)
{
	int w, h;

	w = Dx(screen->r);
	h = Dy(screen->r);
	initscreen(w, h);
	ΔZP = Pt(-w/2, -h/2);
	viewr = rectaddpt(rectsubpt(screen->r, screen->r.min), ΔZP);
	DPRINT(Debugdraw, "resetdraw %R", viewr);
	freeimage(viewfb);
	freeimage(selfb);
	viewfb = eallocimage(viewr, (drawing.flags & DFhaxx0rz) != 0 ? screen->chan : XRGB32, 0, DNofill);
	selfb = eallocimage(viewr, XRGB32, 0, DBlack);
	statr.min = addpt(screen->r.min, Pt(0, viewr.max.y - font->height));
	statr.max = viewr.max;
}

void
renderframe(ulong, int snooze)
{
	/*
	if(redraw() >= 0)
		renderscene();
	else
		warn("redraw: %s\n", error());
	*/
	renderscene();
	if(graph.flags & GFdrawme)
		reqdraw(Reqrefresh);
	if(snooze){
		//drawing.flags |= DFnorend;	/* for coarsening */
		sendul(ticc, 1);
	}
	flush();
}

void
wakedrawup(void)
{
	if(ticc == nil)
		return;
	nbsendul(ticc, 0);
}

static void
rendproc(void *)
{
	ulong r;

	for(;;){
		r = recvul(rendc);	/* FIXME */
		reqdraw(r);
		frame();
	}
}

/* FIXME: only if shit is happening; see also: wakethefup,
 * use lock or rendez instead of chanop */
static void
ticproc(void *)
{
	ulong stop;
	double t0, step;
	vlong t, Δt;

	t0 = μsec();
	step = Nsec / 60.;
	stop = 1;
	for(;;){
		if(nbrecv(ticc, &stop) < 0)
			break;
		while(stop)
			if(recv(ticc, &stop) < 0)
				threadexits(nil);
		t = μsec();
		Δt = t - t0;
		t0 += step * (1 + Δt / step);
		if(Δt < step)
			sleep((step - Δt) / Nmsec);
		reqdraw(Reqshallowdraw);
	}
}

void
initp9draw(void)
{
	cmap = cm_init();
	if((ticc = chancreate(sizeof(ulong), 1)) == nil)
		sysfatal("chancreate: %r");
	if(initdraw(nil, nil, "strpg") < 0)
		sysfatal("initdraw: %r");
	initscreen(Dx(screen->r), Dy(screen->r));
	reqdraw(Reqresetview);
	resizedraw();
	setnodeshape(0);
	newthread(ticproc, nil, nil, nil, "tic", mainstacksize);
	newthread(rendproc, nil, nil, nil, "draw", mainstacksize);
	updateview();
}
