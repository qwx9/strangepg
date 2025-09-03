#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "fs.h"
#include "drw.h"
#include "view.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "layout.h"

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
Box promptbox, selbox;
Channel *rendc, *ctlc;
int drawstate;

static Channel *drawc;
static RLine raxes[3], rselbox[4];

static void
initstatic(void)
{
	RLine *r, a = {.pos1 = {0.0f, 0.0f, 0.0f, 1.0f}};

	r = raxes;
	a.pos2[0] = 200.0f * view.up.X;
	a.pos2[1] = 200.0f * view.up.Y;
	a.pos2[2] = 200.0f * view.up.Z;
	a.pos2[3] = 1.0f;
	setcolor(a.col1, theme[Cxaxis]);
	setcolor(a.col2, theme[Cxaxis]);
	*r++ = a;
	a.pos2[0] = 200.0f * view.right.X;
	a.pos2[1] = 200.0f * view.right.Y;
	a.pos2[2] = 200.0f * view.right.Z;
	a.pos2[3] = 1.0f;
	setcolor(a.col1, theme[Cyaxis]);
	setcolor(a.col2, theme[Cyaxis]);
	*r++ = a;
	a.pos2[0] = 200.0f * view.front.X;
	a.pos2[1] = 200.0f * view.front.Y;
	a.pos2[2] = 200.0f * view.front.Z;
	a.pos2[3] = 1.0f;
	setcolor(a.col1, theme[Czaxis]);
	setcolor(a.col2, theme[Czaxis]);
	*r = a;
	for(r=rselbox; r<rselbox+nelem(rselbox); r++){
		r->pos1[2] = r->pos2[2] = view.center.Z;
		r->pos1[3] = r->pos2[3] = 1.0f;
		setcolor(r->col1, theme[Chigh]);
		setcolor(r->col2, theme[Chigh]);
	}
}

/* FIXME: completely redundant with what the node pipeline is doing,
 * we should either do everything cpu-side or in the shaders; can one
 * shader use the results of another? a doesn't depend at all on b;
 * we could pass the same input as the node pipeline maybe or reuse state? */
static inline void
drawedge(REdge *r, RNode *u, RNode *v, int urev, int vrev)
{
	float l;
	HMM_Vec3 g, a, b;
	HMM_Quat q;

	g = HMM_V3(1.0f, 0.0f, 0.0f);
	a = HMM_V3(u->pos[0], u->pos[1], u->pos[2]);
	l = u->len * drawing.nodesz * 0.5f;
	if(!urev)
		l = -l;
	q = HMM_Q(u->dir[0], u->dir[1], u->dir[2], u->dir[3]);
	a = HMM_AddV3(HMM_RotateV3Q(HMM_MulV3(g, HMM_V3(l, 1.0f, 1.0f)), q), a);
	b = HMM_V3(v->pos[0], v->pos[1], v->pos[2]);
	l = v->len * drawing.nodesz * 0.5f;
	if(vrev)
		l = -l;
	q = HMM_Q(v->dir[0], v->dir[1], v->dir[2], v->dir[3]);
	b = HMM_AddV3(HMM_RotateV3Q(HMM_MulV3(g, HMM_V3(l, 1.0f, 1.0f)), q), b);
	r->pos1[0] = a.X;
	r->pos1[1] = a.Y;
	r->pos1[2] = a.Z;
	r->pos2[0] = b.X;
	r->pos2[1] = b.Y;
	r->pos2[2] = b.Z;
}

static int
drawaxes(int n)
{
	dygrow(rlines, n + nelem(raxes));
	memcpy(rlines + n, raxes, sizeof raxes);
	return nelem(raxes);
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
	vlong n;
	double l, k, Δ, min, max;
	RNode *r, *re;
	Node *u, *ue;

	if((drawing.flags & DFstalelen) == 0)
		return;
	if(drawing.flags & DFrecalclen){
		min = max = 0.0;
		for(u=nodes, ue=u+dylen(u); u<ue; u++){
			n = u->length;
			if(min == 0.0 || min > n)
				min = n;
			if(max < n)
				max = n;
		}
		drawing.length = (Range){min, max};
	}
	drawing.flags &= ~(DFstalelen | DFrecalclen);
	if(drawing.length.min < 1.0)
		drawing.length.min = 1.0;
	if(drawing.length.max < drawing.length.min)
		drawing.length.max = drawing.length.min;
	if((Δ = drawing.length.max - drawing.length.min) < 1.0){
		Δ = 1.0;
		min = max = drawing.nodesz;
	}else{
		min = Minsz;
		max = Maxsz;
	}
	k = Minsz * log(2) / Δ;
	for(u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		l = u->length - drawing.length.min;
		if(l < 1.0)
			l = 1.0;
		r->len = min + (max - min) * ((exp(k * l) - 1) / (exp(k * Δ) - 1));
	}
}

static inline void
faceyourfearsin3d(RNode *ru, Node *u)
{
	float x, y, z, Δ, Δx, Δy, Δz;
	float c, s, t;
	ioff *i, *ie;
	u32int e;
	RNode *rv;
	HMM_Vec3 a = {{0.0f}}, b = {{0.0f}};
	HMM_Quat q;

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
		Δx /= Δ;
		Δy /= Δ;
		Δz /= Δ;
		c += Δx;
		s += Δy;
		t += Δz;
		if((e & 1) == 0){
			b.X += Δx;
			b.Y += Δy;
			b.Z += Δz;
		}
	}
	a = HMM_V3(1.0f, 0.0f, 0.0f);
	if(c != 0.0f || s != 0.0f || t != 0.0f)	/* edge case: choose one side */
		b = HMM_V3(c, s, t);
	b = HMM_NormV3(b);
	q = HMM_QFromNormPair(a, b);
	ru->dir[0] = q.X;
	ru->dir[1] = q.Y;
	ru->dir[2] = q.Z;
	ru->dir[3] = q.W;
}

static inline void
faceyourfears(RNode *ru, Node *u)
{
	float x, y, Δ, Δx, Δy;
	float c, s;
	ioff *i, *ie;
	u32int e;
	RNode *rv;
	HMM_Vec3 a = {{0.0f}}, b = {{0.0f}};
	HMM_Quat q;

	if(u->nedges == 0)
		return;
	x = ru->pos[0];
	y = ru->pos[1];
	c = s = 0.0f;
	for(i=edges+u->eoff, ie=i+u->nedges; i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		if((e & 1) != 0){
			Δx = rv->pos[0] - x;
			Δy = rv->pos[1] - y;
		}else{
			Δx = x - rv->pos[0];
			Δy = y - rv->pos[1];
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy);
		Δx /= Δ;
		Δy /= Δ;
		c += Δx;
		s += Δy;
		if((e & 1) == 0){
			b.X += Δx;
			b.Y += Δy;
		}
	}
	a = HMM_V3(1.0f, 0.0f, 0.0f);
	if(c != 0.0f || s != 0.0f)	/* edge case: choose one side */
		b = HMM_V3(c, s, 0.0f);
	b = HMM_NormV3(b);
	q = HMM_QFromNormPair(a, b);
	ru->dir[0] = q.X;
	ru->dir[1] = q.Y;
	ru->dir[2] = q.Z;
	ru->dir[3] = q.W;
}

static intptr
drawnodes(void)
{
	Node *n, *e;
	RNode *r;

	if(drawing.flags & DF3d)
		for(r=rnodes, n=nodes, e=n+dylen(n); n<e; n++, r++)
			faceyourfearsin3d(r, n);
	else
		for(r=rnodes, n=nodes, e=n+dylen(n); n<e; n++, r++)
			faceyourfears(r, n);
	return r - rnodes;
}

/* FIXME: clean up; note that view.zoom is NOT Δeye.Z, this problem is different */
static inline HMM_Vec3
scr2world(HMM_Vec3 v)
{
	v.X = 2.0f * ((v.X - view.w * 0.5f) / view.w) * view.zoom * view.ar * view.tfov;
	v.Y = 2.0f * ((-v.Y + view.h * 0.5f) / view.h) * view.zoom * view.tfov;
	return v;
}

static int
drawselbox(int rn)
{
	float fx, fy;
	Box b;
	RLine *r;
	HMM_Vec3 v, p[4];

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
	fx = 0.5f * (b.x2 - b.x1);
	fy = 0.5f * (b.y2 - b.y1);
	p[0] = HMM_V3(-fx, -fy, 0.0f);
	p[1] = HMM_V3(-fx,  fy, 0.0f);
	p[2] = HMM_V3( fx,  fy, 0.0f);
	p[3] = HMM_V3( fx, -fy, 0.0f);
	v = HMM_V3(b.x1, b.y1, 0.0f);
	p[0] = scr2world(p[0]);
	p[1] = scr2world(p[1]);
	p[2] = scr2world(p[2]);
	p[3] = scr2world(p[3]);
	v = scr2world(v);
	v = HMM_AddV3(v, view.center);
	p[0] = HMM_RotateV3Q(p[0], view.rot);
	p[1] = HMM_RotateV3Q(p[1], view.rot);
	p[2] = HMM_RotateV3Q(p[2], view.rot);
	p[3] = HMM_RotateV3Q(p[3], view.rot);
	v = HMM_RotateV3Q(v, view.rot);
	v = HMM_SubV3(v, p[0]);
	p[0] = HMM_AddV3(p[0], v);
	p[1] = HMM_AddV3(p[1], v);
	p[2] = HMM_AddV3(p[2], v);
	p[3] = HMM_AddV3(p[3], v);
	r->pos1[0] = p[0].X;
	r->pos1[1] = p[0].Y;
	r->pos1[2] = p[0].Z;
	r->pos2[0] = p[1].X;
	r->pos2[1] = p[1].Y;
	r->pos2[2] = p[1].Z;
	r++;
	r->pos1[0] = p[1].X;
	r->pos1[1] = p[1].Y;
	r->pos1[2] = p[1].Z;
	r->pos2[0] = p[2].X;
	r->pos2[1] = p[2].Y;
	r->pos2[2] = p[2].Z;
	r++;
	r->pos1[0] = p[2].X;
	r->pos1[1] = p[2].Y;
	r->pos1[2] = p[2].Z;
	r->pos2[0] = p[3].X;
	r->pos2[1] = p[3].Y;
	r->pos2[2] = p[3].Z;
	r++;
	r->pos1[0] = p[3].X;
	r->pos1[1] = p[3].Y;
	r->pos1[2] = p[3].Z;
	r->pos2[0] = p[0].X;
	r->pos2[1] = p[0].Y;
	r->pos2[2] = p[0].Z;
	dygrow(rlines, rn + nelem(rselbox));
	memcpy(rlines + rn, rselbox, sizeof rselbox);
	return nelem(rselbox);
}

static int
drawlines(void)
{
	int n;

	n = 0;
	if(debug & Debugdraw)
		n += drawaxes(n);
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
		drawing.frames++;	/* FIXME: remove */
	}
	DPRINT(Debugdraw, "drawworld %zd nodes %zd edges", ndnodes, ndedges);
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
	int f, r, go;

	resizenodes();
	initstatic();
	go = 1;
	for(;;){
		if((r = recvul(drawc)) == 0)
			break;
		while((f = nbrecvul(drawc)) != 0)
			r |= f;
		DPRINT(Debugdraw, "drawproc: %#x", r);
		switch(drawing.flags & (DFfreeze | DFnodraw)){
		case DFfreeze:
			drawing.flags |= DFnodraw;
			go = 0;
			/* wet floor */
		case DFfreeze | DFnodraw:
			DPRINT(Debugdraw, "can\'t draw, i was frozen today");
			ndnodes = ndedges = 0;
			continue;
		case DFnodraw:
			drawing.flags &= ~DFnodraw;
			go = 1;
			break;
		}
		if(r & Reqredraw)
			go = 1;
		if(!(go = redraw(go)))
			reqdraw(Reqsleep);
		else
			reqdraw(Reqpickbuf);
	}
}

void
freezeworld(void)
{
	drawing.flags |= DFfreeze;
	reqlayout(Lfreeze);
	reqdraw(Reqfreeze);
}

void
thawworld(void)
{
	if((drawing.flags & DFnorend) == 0)
		drawing.flags |= DFnorend;	/* FIXME: or we might miss resizebuf() */
	drawing.flags &= ~DFfreeze;
	resizenodes();	/* layout depends on lengths */
	reqdraw(Reqthaw);
	reqlayout(Lthaw);	/* FIXME: really, just unpause */
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

	DPRINT(Debugdraw, "reqdraw %#x df %#lx rf %#lx ", r, df, rf);
	switch(r){
	case Reqfreeze:
	case Reqthaw:
	case Reqrefresh:
	case Reqredraw:
		wakedrawup();
		df |= r;
		if(nbsendul(drawc, df) == 1)
			df = 0;
		/* wet floor */
	case Reqresetdraw:
	case Reqresetview:
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

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	drawing.flags |= DFstalelen | DFrecalclen;
	settheme();
	initcol();
	/* FIXME: this chan implementation SUCKS */
	if((drawc = chancreate(sizeof(ulong), 8)) == nil
	|| (rendc = chancreate(sizeof(ulong), 8)) == nil
	|| (ctlc = chancreate(sizeof(ulong), 1)) == nil)
		sysfatal("initdrw: chancreate");
}
