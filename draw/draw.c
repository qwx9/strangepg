#include "strpg.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "layout.h"
#include "../lib/HandmadeMath.h"

View view;
RNode *rnodes;
REdge *redges;
RLine *rlines;
ioff *vedges;	/* FIXME: check if we could store id's in robj with SG_VERTEXFORMAT_UBYTE4N instead */
ssize ndnodes, ndedges, ndlines;
Drawing drawing = {
	.length = {0.0f, 0.0f},
	.xbound = {0.0f, 0.0f},
	.ybound = {0.0f, 0.0f},
	.zbound = {0.0f, 0.0f},
	.nodesz = Nodesz,
	.fatness = Ptsz,
};
Box selbox;
Channel *rendc, *ctlc;

static Channel *drawc;
static RLine rselbox[4];

static inline void
drawedge(REdge *r, RNode *u, RNode *v, int urev, int vrev)
{
	Vertex p1, p2, du, dv;

	du.x = u->dir[0];
	du.y = u->dir[1];
	du.z = 0.0f;
	p1.x = u->pos[0];
	p1.y = u->pos[1];
	p1.z = u->pos[2];
	dv.x = v->dir[0];
	dv.y = v->dir[1];
	dv.z = 0.0f;
	p2.x = v->pos[0];
	p2.y = v->pos[1];
	p2.z = v->pos[2];
	du = mulv(du, u->len * drawing.nodesz / 2.0f);
	du = urev ? addv(p1, du) : subv(p1, du);
	dv = mulv(dv, v->len * drawing.nodesz / 2.0f);
	dv = vrev ? subv(p2, dv) : addv(p2, dv);
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos1[2] = du.z;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
	r->pos2[2] = dv.z;
}

static intptr
drawedges(void)
{
	ioff id, eid, aid, x, *e, *ee;
	Node *n, *ne;
	RNode *u, *v;
	REdge *r;

	r = redges;
	x = dylen(redges);
	dyresize(vedges, x);
	for(id=eid=0, u=rnodes, n=nodes, ne=n+dylen(n); n<ne; n++, u++, id++){
		for(e=edges+n->eoff, ee=e+n->nedges; e<ee; e++){
			x = *e;
			aid = x >> 2;
			if(id > aid || id == aid && (x & 1) == 1)
				continue;
			v = rnodes + (x >> 2);
			drawedge(r, u, v, x & 1, x & 2);
			r++;
			vedges[eid++] = e - edges;
		}
	}
	return r - redges;
}

void
resizenodes(void)
{
	double l, k, Δ, min, max;
	RNode *r, *re;

	if((drawing.flags & DFstalelen) == 0)
		return;
	min = Minsz;
	max = Maxsz;
	drawing.flags &= ~DFstalelen;
	if(drawing.length.min < 1.0)
		drawing.length.min = 1.0;
	if(drawing.length.max < drawing.length.min)
		drawing.length.max = drawing.length.min;
	if((Δ = drawing.length.max - drawing.length.min) < 1.0){
		Δ = 1.0;
		min = max = drawing.nodesz;
	}
	k = Minsz * log(2) / Δ;
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		l = r->len - drawing.length.min;
		if(l < 1.0)
			l = 1.0;
		r->len = min + (max - min) * ((exp(k * l) - 1) / (exp(k * Δ) - 1));
	}
}

static inline void
faceyourfears(RNode *ru, Node *u)
{
	float x, y, z, Δ, Δx, Δy, Δz;
	float θ, c, s, t;
	ioff *i, *ie;
	u32int e;
	RNode *rv;

	if(u->nedges == 0)
		return;
	x = ru->pos[0];
	y = ru->pos[1];
	z = ru->pos[2];
	c = s = t = 0.0f;
	for(i=edges+u->eoff, ie=i+u->nedges; i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		if((e & 1) != 0){
			Δx = rv->pos[0] - x;
			Δy = rv->pos[1] - y;
			Δz = rv->pos[2] - z;
		}else{
			Δx = x - rv->pos[0];
			Δy = y - rv->pos[1];
			Δz = z - rv->pos[2];
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy + Δz * Δz);
		c += Δx / Δ;
		s += Δy / Δ;
		t += Δz / Δ;
	}
	θ = fmodf(atan2f(s, c), 2.0f * (float)PI);
	ru->dir[0] = cosf(θ);
	ru->dir[1] = sinf(θ);
	θ = sqrtf(c * c + s * s);
	θ = fmodf(atan2f(θ, t), 2.0f * (float)PI);
	ru->dir[2] = cosf(θ);
	ru->dir[3] = sinf(θ);
}

static intptr
drawnodes(void)
{
	Node *n, *e;
	RNode *r;

	for(r=rnodes, n=nodes, e=n+dylen(n); n<e; n++, r++)
		faceyourfears(r, n);
	return r - rnodes;
}

/* FIXME */
#define scr2world(x,y)	do{ \
	(x) = 2.0f * (((x) - view.w * 0.5f) / view.w) * view.Δeye.z * view.ar * view.tfov; \
	(y) = 2.0f * ((-(y) + view.h * 0.5f) / view.h) * view.Δeye.z * view.tfov; \
}while(0)

/* FIXME: wrong orientation/rotation wrt roll, etc. */
static int
drawselbox(int rn)
{
	Box b;
	RLine *r;

	b = selbox;
	r = rselbox;
	/* FIXME: after a deselect, we just don't draw, but the renderer still
	 * retains state up until the next re-render; if we just return, then
	 * next time we draw the box we'll first get one frame with the last
	 * one we drew */
	/*
	if((b.x1 == b.x2 || b.y1 == b.y2)
		return 0;
	*/
	scr2world(b.x1, b.y1);
	scr2world(b.x2, b.y2);
	b.x1 += view.eye.x;
	b.y1 += view.eye.y;
	b.x2 += view.eye.x;
	b.y2 += view.eye.y;
	r->pos1[0] = b.x1;
	r->pos1[1] = b.y1;
	r->pos1[2] = view.center.z;
	r->pos2[0] = b.x2;
	r->pos2[1] = b.y1;
	r->pos2[2] = view.center.z;
	r++;
	r->pos1[0] = b.x2;
	r->pos1[1] = b.y1;
	r->pos1[2] = view.center.z;
	r->pos2[0] = b.x2;
	r->pos2[1] = b.y2;
	r->pos2[2] = view.center.z;
	r++;
	r->pos1[0] = b.x1;
	r->pos1[1] = b.y2;
	r->pos1[2] = view.center.z;
	r->pos2[0] = b.x2;
	r->pos2[1] = b.y2;
	r->pos2[2] = view.center.z;
	r++;
	r->pos1[0] = b.x1;
	r->pos1[1] = b.y1;
	r->pos1[2] = view.center.z;
	r->pos2[0] = b.x1;
	r->pos2[1] = b.y2;
	r->pos2[2] = view.center.z;
	dygrow(rlines, rn + nelem(rselbox));
	memcpy(rlines + rn, rselbox, sizeof rselbox);
	return nelem(rselbox);
}

static int
drawlines(void)
{
	int n;

	n = debug & Debugdraw ? 3 : 0;	/* FIXME: fragile */
	n += drawselbox(n);
	return n;
}

static int
drawworld(int go)
{
	int r;

	if(graph.layout == nil)
		return 0;
	if((r = graph.flags & GFdrawme) == 0 && !go){
		ndnodes = dylen(rnodes);
		ndedges = dylen(redges);
	}else{
		ndnodes = drawnodes();
		ndedges = drawedges();
	}
	ndlines = drawlines();
	return r;
}

int
redraw(int go)
{
	static Clk clk = {.lab = "redraw"};

	CLK0(clk);
	go = drawworld(go);
	CLK1(clk);
	return go;
}

static void
drawproc(void *)
{
	int r, go;

	go = 1;
	for(;;){
		if((r = recvul(drawc)) == 0)
			break;
		if(r & Reqthaw){
			drawing.flags &= ~DFnodraw;
			reqdraw(Reqrefresh);
		}else if(r & Reqfreeze){
			sendul(ctlc, DFnodraw);
			drawing.flags |= DFnodraw;
		}
		if(drawing.flags & DFnodraw)
			continue;
		if(r & Reqstop){
			go = 0;
			continue;
		}else if(r & Reqredraw)
			go = 1;
		if(!(go = redraw(go)))
			reqdraw(Reqsleep);
		else
			reqdraw(Reqpickbuf);
	}
}

void
freezedraw(void)
{
	reqdraw(Reqfreeze);
	reqlayout(Lstop);
	recvul(ctlc);
	recvul(ctlc);
}

void
thawdraw(void)
{
	reqdraw(Reqrefresh);
	reqdraw(Reqthaw);
	reqlayout(Lstart);
}

void
waitforit(void)
{
	/* wait until at least one file asks for a redraw */
	while((recvul(drawc) & Reqredraw) == 0)
		;
	newthread(drawproc, nil, nil, nil, "draw", mainstacksize);
	reqdraw(Reqredraw);
}

void
reqdraw(int r)
{
	static ulong df, rf;

	DPRINT(Debugdraw, "reqdraw %#x df %#x rf %#x ", r, df, rf);
	switch(r){
	case Reqfreeze:
	case Reqthaw:
	case Reqstop:
	case Reqrefresh:
	case Reqredraw:
		wakedrawup();
		df |= r;
		if(nbsendul(drawc, df) == 1)
			df = 0;
		/* wet floor */
	case Reqresetdraw:
	case Reqresetui:
	case Reqfocus:
	case Reqpickbuf:
	case Reqshape:
	case Reqshallowdraw:
	case Reqsleep:
		rf |= r;
		if(nbsendul(rendc, rf) == 1)
			rf = 0;
		break;
	default:
		warn("reqdraw: unknown request %#x\n", r);
	}
}

static void
initselbox(void)
{
	RLine *r;

	for(r=rselbox; r<rselbox+nelem(rselbox); r++){
		r->pos1[2] = r->pos2[2] = view.center.z;
		r->pos1[3] = r->pos2[3] = 1.0f;
		setcolor(r->col1, theme[Chigh]);
		setcolor(r->col2, theme[Chigh]);
	}
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	drawing.flags |= DFstalelen;
	settheme();
	initcol();
	/* FIXME: this chan implementation SUCKS */
	if((drawc = chancreate(sizeof(ulong), 1)) == nil
	|| (rendc = chancreate(sizeof(ulong), 1)) == nil
	|| (ctlc = chancreate(sizeof(ulong), 2)) == nil)
		sysfatal("initdrw: chancreate");
	initselbox();
}
