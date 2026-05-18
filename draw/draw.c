#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "fs.h"
#include "drw.h"
#include "view.h"
#include "ui.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "layout.h"

RNode *rnodes, *vnodes;
REdge *redges;
RLine *rlines;
ioff *v2id;

Drawing drawing = {
	.flags = DFstalelen | DFrecalclen | DFnoray,
	.length = {
		.min = 0x7fffffff,
		.max = 0,
	},
	.nodesz = Nodesz,
	.fatness = Ptsz,
};
Box promptbox, selbox;
Channel *rendc, *ctlc;
int drawstate;

static Channel *drawc;
static RLine raxes[6], rselbox[4];
static RWLock drawlock;
static QLock raylock;

static void
initstatic(void)
{
	RLine *r, a = {.pos1 = {0.0f, 0.0f, 0.0f, 1.0f}};

	r = raxes;
	a.pos2[0] = 200.0f * view.right.X;
	a.pos2[1] = 200.0f * view.right.Y;
	a.pos2[2] = 200.0f * view.right.Z;
	a.pos2[3] = 1.0f;
	setcolor(a.col1, theme[Cxaxis]);
	setcolor(a.col2, theme[Cxaxis]);
	*r++ = a;
	a.pos2[0] = 200.0f * view.up.X;
	a.pos2[1] = 200.0f * view.up.Y;
	a.pos2[2] = 200.0f * view.up.Z;
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
	*r++ = a;
	setcolor(r->col1, theme[Cxaxis]);
	setcolor(r->col2, theme[Cxaxis]);
	r->pos1[3] = r->pos2[3] = 1.0f;
	r++;
	setcolor(r->col1, theme[Cyaxis]);
	setcolor(r->col2, theme[Cyaxis]);
	r->pos1[3] = r->pos2[3] = 1.0f;
	r++;
	setcolor(r->col1, theme[Czaxis]);
	setcolor(r->col2, theme[Czaxis]);
	r->pos1[3] = r->pos2[3] = 1.0f;
	r++;
	assert(r == raxes + nelem(raxes));
	for(r=rselbox; r<rselbox+nelem(rselbox); r++){
		r->pos1[2] = r->pos2[2] = view.center.Z;
		r->pos1[3] = r->pos2[3] = 1.0f;
		setcolor(r->col1, theme[Chigh]);
		setcolor(r->col2, theme[Chigh]);
	}
}

static inline void
setrcol(RNode *r, u32int col, int high)
{
	setcolor(r->col, setdefalpha(col));	/* FIXME: set alpha elsewhere? */
	if(high)
		highlightnode(r);
}

static inline void
setrlen(RNode *r, vlong n, double min, double max, double k)
{
	if((n -= drawing.length.min) < 0)
		n = 0;
	r->len = min + max * (exp(k * n) - 1.0);	/* / (exp(k * Δ) - 1) == 1 */
}

static void
initrnodes(void)
{
	ioff i, n;
	double min, max, k, tot;
	RNode *r;
	Node *u, *ue;

	n = dylen(nodes);
	dyresize(rnodes, n);
	min = drawing.rlen.min;
	max = drawing.rlen.max;
	k = drawing.k;
	tot = 0.0;
	for(i=0, r=rnodes, u=nodes, ue=u+n; u<ue; u++, r++, i++){
		setrlen(r, u->length, min, max, k);
		tot += r->len;
	}
	drawing.meansz = tot / n;
}

/* FIXME: reduce/increase jitter amount? */
/* length set in bulk in resetlengths */
void
spawnrnode(RNode *r, RNode *from, voff id)
{
	float z;

	r->pos[0] = from->pos[0] + 0.5f - xfrand();
	r->pos[1] = from->pos[1] + 0.5f - xfrand();
	if(drawing.flags & DF3d)
		r->pos[2] = from->pos[2] + 0.5f - xfrand();
	else{
		z = -0.5f + (float)id / nnodes;
		r->pos[2] = (drawing.flags & DFnodepth) == 0
			? 0.1f * z : 0.00001f * z;
	}
}

static void
recolornodes(void)
{
	RNode *r;
	Node *u, *ue;

	for(r=rnodes, u=nodes, ue=u+dylen(u); u<ue; u++, r++)
		setrcol(r, getcnodecolor(u->id), 0);
}

/* FIXME: compute min/max as we go, has to be at least 1 */
static void
resetlengths(void)
{
	ioff n;
	vlong m, min, max;
	double k, rmin, rmax, tot;
	Node *u, *ue;
	RNode *r, *re;

	if((n = dylen(nodes)) == 0)
		return;
	ue = nodes + n;
	min = drawing.length.min;
	max = drawing.length.max;
	if(drawing.flags & DFrecalclen){
		/* FIXME: do this as we go, not here */
		for(u=nodes; u<ue; u++){
			m = u->length;
			if(u > nodes){
				if(min > m)
					min = m;
				if(max < m)
					max = m;
			}else
				min = max = m;
		}
		drawing.length.min = min;
		drawing.length.max = max;
	}
	if(min < 1)
		min = 1;
	if(max < min)
		max = min;
	if((m = max - min) <= 0)
		m = 1;
	k = log(2) / m;
	if(m > Maxsz - Minsz){
		rmin = Minsz;
		rmax = Maxsz;
	}else{
		if(min >= Maxsz || max >= Maxsz){
			rmax = Maxsz;
			rmin = rmax - m;
		}else if(min < Minsz || max < Minsz || max < min){
			rmin = Minsz;
			rmax = rmin + m;
		}else{
			rmax = max;
			rmin = min;
		}
	}
	if(rmax - rmin <= 0.0)
		rmax = rmin + 1;
	if(drawing.nodesz != 1.0){
		rmin *= drawing.nodesz;
		rmax *= drawing.nodesz;
	}
	rmax -= rmin;
	drawing.rlen.min = rmin;
	drawing.rlen.max = rmax;
	drawing.k = k;
	drawing.flags &= ~(DFstalelen | DFrecalclen);
	if(rnodes == nil)
		return;
	tot = 0.0;
	n = dylen(rnodes);
	for(u=nodes, r=rnodes, re=r+n; r<re; r++, u++){
		setrlen(r, u->length, rmin, rmax, k);
		tot += r->len;
	}
	drawing.meansz = tot / n;
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

static RLine *
drawaxes(RLine *rp)
{
	int n;
	float o[4];
	RLine *r;
	HMM_Vec3 v;

	n = dylen(rp);
	dyresize(rp, n + nelem(raxes));
	r = rp + n;
	memcpy(r, raxes, sizeof raxes);
	r += 3;
	o[0] = view.center.X;
	o[1] = view.center.Y;
	o[2] = view.center.Z;
	o[3] = 1.0f;
	memcpy(r->pos1, o, sizeof o);
	v = HMM_AddV3(view.center, HMM_MulV3F(view.right, 20.0f));
	r->pos2[0] = v.X;
	r->pos2[1] = v.Y;
	r->pos2[2] = v.Z;
	r++;
	memcpy(r->pos1, o, sizeof o);
	v = HMM_AddV3(view.center, HMM_MulV3F(view.up, 20.0f));
	r->pos2[0] = v.X;
	r->pos2[1] = v.Y;
	r->pos2[2] = v.Z;
	r++;
	memcpy(r->pos1, o, sizeof o);
	v = HMM_AddV3(view.center, HMM_MulV3F(view.front, 20.0f));
	r->pos2[0] = v.X;
	r->pos2[1] = v.Y;
	r->pos2[2] = v.Z;
	return rp;
}

static void
drawedges(void)
{
	ioff i, j, x, *e, *ee;
	ssize n;
	Node *u, *v;
	RNode *ru, *re, *rv;

	for(n=0, i=0, ru=rnodes, re=ru+dylen(ru); ru<re; ru++, i++){
		u = nodes + i;
		if((u->dflags & FNDvis) == 0)
			continue;
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			if(i > j || i == j && (x & 1) == 1)
				continue;
			v = nodes + j;
			if((v->dflags & FNDvis) == 0)
				continue;
			rv = rnodes + j;
			dygrow(redges, n);
			drawedge(redges + n, ru, rv, x & 1, x & 2);
			n++;
		}
	}
}

static inline void
faceyourfearsin3d(RNode *ru, Node *u)
{
	int zilch;
	float x, y, z, Δ, Δx, Δy, Δz;
	double c, s, t;
	ioff *i, *ie;
	u32int e;
	RNode *rv;
	HMM_Vec3 a = {{0.0f}}, b = {{0.0f}};
	HMM_Quat q;

	assert(u->nedges > 0);
	x = ru->pos[0];
	y = ru->pos[1];
	z = ru->pos[2];
	c = s = t = 0.0f;
	zilch = 1;
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
		if(Δ == 0.0f)
			continue;
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
			zilch = 0;
		}
	}
	a = HMM_V3(1.0f, 0.0f, 0.0f);
	if(c != 0.0 || s != 0.0 || t != 0.0)	/* edge case: choose one side */
		b = HMM_V3(c, s, t);
	else if(zilch)
		b = HMM_V3(1.0f, 1.0f, 1.0f);
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
	int zilch;
	float x, y, Δ, Δx, Δy;
	double c, s;
	ioff *i, *ie;
	u32int e;
	RNode *rv;
	HMM_Vec3 a = {{0.0f}}, b = {{0.0f}};
	HMM_Quat q;

	assert(u->nedges > 0);
	x = ru->pos[0];
	y = ru->pos[1];
	c = s = 0.0f;
	zilch = 1;
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
		if(Δ == 0.0f)
			continue;
		Δx /= Δ;
		Δy /= Δ;
		c += Δx;
		s += Δy;
		if((e & 1) == 0){
			b.X += Δx;
			b.Y += Δy;
			zilch = 0;
		}
	}
	a = HMM_V3(1.0f, 0.0f, 0.0f);
	if(c != 0.0 || s != 0.0)	/* edge case: choose one side */
		b = HMM_V3(c, s, 0.0f);
	else if(zilch)
		b = HMM_V3(1.0f, 1.0f, 0.0f);
	b = HMM_NormV3(b);
	q = HMM_QFromNormPair(a, b);
	ru->dir[0] = q.X;
	ru->dir[1] = q.Y;
	ru->dir[2] = q.Z;
	ru->dir[3] = q.W;
}

static void
drawnodes(void)
{
	int is3d;
	ioff id;
	Node *u;
	RNode *r, *re;

	is3d = drawing.flags & DF3d;
	for(id=0, u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++, id++){
		if(drawing.flags & DFnoray)	/* FIXME */
			u->dflags |= FNDvis;
		else if((u->dflags & FNDvis) == 0)	/* FIXME: what about edges? */
			continue;
		if(u->uflags & FNUhigh){
			highlightnode(r);
			u->uflags &= ~FNUhigh;
		}
		if(u->nedges > 0){
			if(is3d)
				faceyourfearsin3d(r, u);
			else
				faceyourfears(r, u);
		}
		if((drawing.flags & DFnoray) == 0){
			dypush(vnodes, *r);
			dypush(v2id, id);
		}
	}
}

/* FIXME: redundant with s2w! */
static inline HMM_Vec3
scr2world(HMM_Vec3 v)
{
	v.X = 2.0f * ((v.X - view.w * 0.5f) / view.w) * view.zoom * view.ar * view.tfov;
	v.Y = 2.0f * ((-v.Y + view.h * 0.5f) / view.h) * view.zoom * view.tfov;
	return v;
}

static RLine *
drawselbox(RLine *rp)
{
	int n;
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

	if(b.x1 == b.x2 || b.y1 == b.y2)
		return rp;

	fx = 0.5f * (b.x2 - b.x1);
	fy = 0.5f * (b.y2 - b.y1);
	p[0] = HMM_V3(-fx, -fy, 0.0f);
	p[1] = HMM_V3(-fx,  fy, 0.0f);
	p[2] = HMM_V3( fx,  fy, 0.0f);
	p[3] = HMM_V3( fx, -fy, 0.0f);
	v = HMM_V3(b.x1, b.y1, 0.0f);
	/* FIXME: use helpers above */
	p[0] = scr2world(p[0]);
	p[1] = scr2world(p[1]);
	p[2] = scr2world(p[2]);
	p[3] = scr2world(p[3]);
	v = scr2world(v);
	p[0] = HMM_RotateV3Q(p[0], view.rot);
	p[1] = HMM_RotateV3Q(p[1], view.rot);
	p[2] = HMM_RotateV3Q(p[2], view.rot);
	p[3] = HMM_RotateV3Q(p[3], view.rot);
	v = HMM_RotateV3Q(v, view.rot);
	v = HMM_AddV3(v, view.center);
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
	n = dylen(rp);
	dyresize(rp, n + nelem(rselbox));
	memcpy(rp + n, rselbox, sizeof rselbox);
	return rp;
}

static RLine *
drawlines(void)
{
	RLine *rp;

	rp = rlines;
	dyclear(rp);
	if(debug & Debugdraw)
		rp = drawaxes(rp);
	rp = drawselbox(rp);
	return rp;
}

/* FIXME: reuse buffers instead of reallocating if possible */
int
redraw(void)
{
	ioff nn;
	vlong t;

	t = μsec();
	TIME("redraw", "wait", t);
	nn = 0;
	if(drawing.flags & DFnoray){
		if(rnodes != nil){
			drawnodes();
			TIME("redraw", "drawnodes", t);
			drawedges();
			TIME("redraw", "drawedges", t);
		}
	}else{
		nn = raytrace(&drawing.tree);
		TIME("redraw", "raytrace", t);
		lockrend();
		//dyresize(vnodes, nn);
		dyclear(vnodes);
		dyclear(v2id);
		drawnodes();
		unlockrend();
		TIME("redraw", "drawnodes", t);
		lockrend();
		dyclear(redges);
		drawedges();
		unlockrend();
		TIME("redraw", "drawedges", t);
	}
	DPRINT(Debugdraw, "redraw %d/%zd/%zd/%zd nodes %zd edges %zd lines\n",
		nn, dylen(vnodes), dylen(rnodes), dylen(nodes), dylen(redges), dylen(rlines));
	return 0;
}

static int
partition(void)
{
	vlong t;
	Tree th;

	t = μsec();
	if(mktree(&th) < 0){
		DPRINT(Debugdraw, "mktree: %s", error());
		return 0;
	}
	TIME("draw", "partition", t);
	lockrend();
	nuketree(&drawing.tree);
	drawing.tree = th;
	unlockrend();
	TIME("draw", "swap", t);
	return 1;
}

static int
drawworld(int go)
{
	if(graph.layout == nil)
		return 0;
	if((graph.flags & GFdrawme) != 0 || go){
		if(drawing.flags & DFnoray || (go = partition()))
			redraw();
	}else
		go = 0;
	return go;
}

static void
drawback(void)
{
	vlong t;
	RLine *rp;

	t = μsec();
	rp = drawlines();
	rlines = rp;
	TIME("draw", "lines", t);
}

static void
drawproc(void *)
{
	int go;
	ulong f, r;

	initstatic();
	drawing.flags |= DFarmed;
	for(go=1;;){
		if((r = recvul(drawc)) == 0)
			break;
		while((f = nbrecvul(drawc)) != 0)
			r |= f;
		DPRINT(Debugdraw, "drawproc: %#lx", r);
		lockdraw();
		if(drawing.flags & DFstalelen)
			resetlengths();
		if(r & Reqredraw || r & Reqrefresh && drawing.flags & DF3d)
			go = 1;
		drawback();
		if(go)
			go = drawworld(go);
		unlockdraw();
		drawing.flags |= DFfiring;
		if(go)
			reqdraw(Reqpickbuf);
		else
			reqdraw(Reqsleep);
	}
}

void
frame(void)	/* render thread */
{
	int n, snooze;
	ulong r, f;

	n = snooze = 0;
	r = 0;
	while((f = nbrecvul(rendc)) != 0){
		r |= f;
		n++;
	}
	if(n >= 8){
		DPRINT(Debugdraw, "frame: flushing additional pending events");
		reqdraw(Reqnone);
		while((f = nbrecvul(rendc)) != 0)
			r |= f;
	}
	DPRINT(Debugdraw, "frame: %#lx", r);
	if(r != 0){
		if(r & Reqsleep)
			snooze = 1;
		if(r & Reqresetdraw){
			resizedraw();
			updateview();
		}
		if(r & Reqresetview){
			resetview();
			updateview();
		}
		if(r & Reqfocus)
			focusobj();
		if(r & Reqshape){
			drawing.flags ^= DFdrawarrows;
			setnodeshape(drawing.flags & DFdrawarrows);
		}
	}else
		snooze = 1;
	renderframe(r, snooze);
}

void
freezeworld(void)
{
	DPRINT(Debugdraw, "freezeworld");
	reqlayout(Lfreeze);	/* stop layout before trying to claim lock */
	wlockdraw();
}

void
thawworld(int nn, int ne, RNode *extra)
{
	ioff nx;

	DPRINT(Debugdraw, "thawworld");
	if(rnodes == nil){
		assert(nn == dylen(nodes));
		initrnodes();
	}else
		dyresize(rnodes, nn);
	if((nx = dylen(extra)) > 0){
		memmove(rnodes + nn - nx, extra, nx * sizeof *extra);
		dyfree(extra);
	}
	dyresize(redges, ne);	/* FIXME: may be overestimated */
	dyclear(redges);
	if((drawing.flags & DFnoray) == 0)
		dyfree(vnodes);
	else
		vnodes = rnodes;
	wunlockdraw();
	reqlayout(Lthaw);
	recolornodes();
}

void
wlockdraw(void)
{
	wlock(&drawlock);
}

void
wunlockdraw(void)
{
	wunlock(&drawlock);
}

int
canlockdraw(void)
{
	return canrlock(&drawlock);
}

void
lockdraw(void)
{
	rlock(&drawlock);
}

void
unlockdraw(void)
{
	runlock(&drawlock);
}

void
lockrend(void)
{
	qlock(&raylock);
}

int
canlockrend(void)
{
	return canqlock(&raylock);
}

void
unlockrend(void)
{
	qunlock(&raylock);
}

/* FIXME: filtering here might not really make sense, maybe drawc/rendc
 * readers should all just ignore what they don't care about */
void
reqdraw(int r)
{
	static ulong df, rf, orf;

	r &= ~Reqnone;
	DPRINT(Debugdraw, "reqdraw %#x df %#lx rf %#lx draw %#x ",
		r, df, rf, drawing.flags);
	if((drawing.flags & DFarmed) == 0){
		if((r & Reqinit) == 0){
			if(r & Reqanydraw)
				df |= r & Reqanydraw;
			rf |= r;
			return;
		}else
			df |= r & (Reqanydraw | Reqinit);
	}else if(r & Reqanydraw)
		df |= r & Reqanydraw;
	/* FIXME: why is the chanavail shit necessary at all here? */
	if((orf & r) == 0 || chanavail(rendc) == 0){
		rf |= r;
		orf = rf;
	}
	if(df != 0 && nbsendul(drawc, df) == 1)
		df = 0;
	if(rf != 0){
		if(nbsendul(rendc, rf) == 1)
			rf = 0;
		if(drawing.flags & DFarmed)
			wakedrawup();
	}
}

/* wait until at least one file asks for a redraw and render starts up */
void
waitforit(void)
{
	ulong r, f;

	DPRINT(Debugdraw, "preparing to salt fries...");
	for(f=0; (f&Reqinit)!=Reqinit;){
		if((r = recvul(drawc)) == 0)
			sysfatal("recvul: %s", error());
		f |= r;
	}
	DPRINT(Debugdraw, "fries salted.");
	reqdraw(f & ~Reqinit | Reqredraw);
	newthread(drawproc, nil, nil, nil, "draw", mainstacksize);
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	initrwlock(&drawlock);
	initqlock(&raylock);
	settheme();
	/* FIXME: this chan implementation SUCKS */
	if((drawc = chancreate(sizeof(ulong), 8)) == nil
	|| (rendc = chancreate(sizeof(ulong), 8)) == nil
	|| (ctlc = chancreate(sizeof(ulong), 1)) == nil)
		sysfatal("initdrw: chancreate");
}
