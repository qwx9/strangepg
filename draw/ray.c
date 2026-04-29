#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"
#include "threads.h"

typedef struct Cull Cull;

enum{
	Maxhits = 5,
};
#define	Eps	0.0001f

/* FIXME: more shit */
struct Cull{
	Tree *tp;
	AABB px;
	Vec3 eye;
	float w[3];
};

static inline int
pushleaf(ioff idx)
{
	Node *u;

	u = nodes + idx;
	if(u->dflags & FNDvis)
		return 0;
	u->dflags |= FNDvis;
	return 1;
}

/* FIXME: parallel to axis edge case */
static inline int
intersect1(AABB *bp, Vec3 *o, Vec3 *d)
{
	float t0, t1, tc, tf;

	if(d->p[0] >= 0){
		t0 = (bp->tl.p[0] - o->p[0]) / (d->p[0] + Eps);
		t1 = (bp->br.p[0] - o->p[0]) / (d->p[0] + Eps);
	}else{
		t1 = (bp->tl.p[0] - o->p[0]) / (d->p[0] + Eps);
		t0 = (bp->br.p[0] - o->p[0]) / (d->p[0] + Eps);
	}
	if(isinf(t1))
		warn("xinf %.2f-%.2f %.2f %.2f\n",
			bp->tl.p[0], bp->br.p[0], o->p[0], d->p[0]);
	tc = t0;
	tf = t1;
	if(d->p[1] >= 0){
		t0 = (bp->tl.p[1] - o->p[1]) / (d->p[1] + Eps);
		t1 = (bp->br.p[1] - o->p[1]) / (d->p[1] + Eps);
	}else{
		t1 = (bp->tl.p[1] - o->p[1]) / (d->p[1] + Eps);
		t0 = (bp->br.p[1] - o->p[1]) / (d->p[1] + Eps);
	}
	if(isinf(t1))
		warn("yinf %.2f-%.2f %.2f %.2f\n",
			bp->tl.p[0], bp->br.p[0], o->p[1], d->p[1]);
	if(tc < t0)
		tc = t0;
	if(tf > t1)
		tf = t1;
	if(d->p[2] >= 0){
		t0 = (bp->tl.p[2] - o->p[2]) / (d->p[2] + Eps);
		t1 = (bp->br.p[2] - o->p[2]) / (d->p[2] + Eps);
	}else{
		t1 = (bp->tl.p[2] - o->p[2]) / (d->p[2] + Eps);
		t0 = (bp->br.p[2] - o->p[2]) / (d->p[2] + Eps);
	}
	if(isinf(t1))
		warn("zinf %.2f-%.2f %.2f %.2f\n",
			bp->tl.p[0], bp->br.p[0], o->p[2], d->p[2]);
	if(tc < t0)
		tc = t0;
	if(tf > t1)
		tf = t1;
	return tc <= tf && tc >= 0;
}

/* FIXME: parallel to axis edge case */
static inline int
intersect1w(float p[3], float w, Vec3 *o, Vec3 *d)
{
	float l, h, t0, t1, tc, tf;

	w *= 0.5f;
	l = p[0] - w;
	h = p[0] + w;
	if(d->p[0] >= 0){
		t0 = (l - o->p[0]) / (d->p[0] + Eps);
		t1 = (h - o->p[0]) / (d->p[0] + Eps);
	}else{
		t1 = (l - o->p[0]) / (d->p[0] + Eps);
		t0 = (h - o->p[0]) / (d->p[0] + Eps);
	}
	if(isinf(t1))
		warn("xinf %.2f-%.2f %.2f %.2f\n", l, h, o->p[0], d->p[0]);
	tc = t0;
	tf = t1;
	l = p[1] - w;
	h = p[1] + w;
	if(d->p[1] >= 0){
		t0 = (l - o->p[1]) / (d->p[1] + Eps);
		t1 = (h - o->p[1]) / (d->p[1] + Eps);
	}else{
		t1 = (l - o->p[1]) / (d->p[1] + Eps);
		t0 = (h - o->p[1]) / (d->p[1] + Eps);
	}
	if(isinf(t1))
		warn("yinf %.2f-%.2f %.2f %.2f\n", l, h, o->p[1], d->p[1]);
	if(tc < t0)
		tc = t0;
	if(tf > t1)
		tf = t1;
	l = h = p[2];
	if(drawing.flags & DF3d){
		l -= w;
		h += w;
	}
	if(d->p[2] >= 0){
		t0 = (l - o->p[2]) / (d->p[2] + Eps);
		t1 = (h - o->p[2]) / (d->p[2] + Eps);
	}else{
		t1 = (l - o->p[2]) / (d->p[2] + Eps);
		t0 = (h - o->p[2]) / (d->p[2] + Eps);
	}
	if(isinf(t1))
		warn("zinf %.2f-%.2f %.2f %.2f\n", l, h, o->p[2], d->p[2]);
	if(tc < t0)
		tc = t0;
	if(tf > t1)
		tf = t1;
	return tc <= tf && tc >= 0;
}

static inline ioff
intersectnodes(Vec3 *o, Vec3 *d, ioff *ip, ioff n, int m)
{
	ioff tot, *e;
	RNode *r;

	for(tot=0, e=ip+n; ip<e && m>0; ip++){
		r = rnodes + *ip;
		if(!intersect1w(r->pos, r->len, o, d) || !pushleaf(*ip))
			continue;
		tot++;
		m--;
	}
	return tot;
}

/* FIXME: order by distance from camera (here?) */
static inline ioff
pushleavesif(AABB *bp, float w[3], Vec3 *o, ioff *ip, ioff n)
{
	int h;
	ioff *p, *e, tot;
	float x, y;
	RNode *r;
	Node *u;
	HMM_Vec3 v;
	Vec3 d;

	e = ip + n;
	for(tot=0, y=bp->tl.p[1]+w[1]*0.5f; y<bp->br.p[1]; y+=w[1]){
		for(x=bp->tl.p[0]+w[0]*0.5f; x<bp->br.p[0]; x+=w[0]){
			v = HMM_NormV3(HMM_SubV3(HMM_V3(x, y, 0.0f), view.eye));
			d.x = v.X;
			d.y = v.Y;
			d.z = v.Z;
			for(h=0, p=ip; p<e && h<Maxhits; p++){
				u = nodes + *p;
				if(u->dflags & FNDvis){	/* FIXME: sucks */
					h++;
					continue;
				}
				r = rnodes + *p;
				if(intersect1w(r->pos, r->len, o, &d)){
					if(pushleaf(*p))
						tot++;
					h++;
				}
			}
		}
	}
	return tot;
}

/* FIXME: combine with dfs above into bfs */
static ioff
traceray(B *b, int i, AABB *bp, Vec3 *o, Vec3 *d, ioff hits, Tree *tp)
{
	int li, ri;
	B *bl, *br;
	AABB bb;

	if(d->p[i] >= 0){	/* FIXME: anything for the == 0 (or ±Eps/Giveup) case? */
		bl = b;
		br = b + 1;
	}else{
		bl = b + 1;
		br = b;
	}
	bb = *bp;
	DPRINTS(Debugray, "trace: %d hits, axis %d: l node %zd: ",
		hits, i, bl - tp->b);
	if((li = bl->child & 3) == Fleaf){
		if(bl->n == 0)
			DPRINTN(Debugray, "empty leaf; ");
		else{
			hits += intersectnodes(o, d, tp->idx + bl->off, bl->n, Maxhits - hits);
			DPRINTN(Debugray, "%d new hits; ", hits);
			if(hits >= Maxhits){
				DPRINTN(Debugray, "we\'re done here\n");
				return hits;
			}
		}
		bl = nil;
	/* FIXME: can we get away with only doing one of these at a time? */
	}else{
		DPRINTN(Debugray, "l %.2f r %.2f o %.2f d %.2f; ",
			bl->l, bl->r, o->p[i], d->p[i]);
		if(bb.tl.p[i] < bl->l)
			bb.tl.p[i] = bl->l;
		if(bb.br.p[i] > bl->r)
			bb.br.p[i] = bl->r;
		if(!intersect1(&bb, o, d)){
			DPRINTN(Debugray, "not intersecting; ");
			bl = nil;
		}
	}
	DPRINTN(Debugray, "r node %zd: ", br - tp->b);
	if((ri = br->child & 3) == Fleaf){
		if(br->n == 0)
			DPRINTN(Debugray, "empty leaf; ");
		else{
			hits += intersectnodes(o, d, tp->idx + br->off, br->n, Maxhits - hits);
			DPRINTN(Debugray, "%d new hits; ", hits);
			if(hits >= Maxhits){
				DPRINTN(Debugray, "we\'re done here\n");
				return hits;
			}
		}
		br = nil;
	}else{
		DPRINTN(Debugray, "l %.2f r %.2f o %.2f d %.2f; ",
			br->l, br->r, o->p[i], d->p[i]);
		bb.tl = bp->tl;
		bb.br = bp->br;
		if(bb.tl.p[i] < br->l)
			bb.tl.p[i] = br->l;
		if(bb.br.p[i] > br->r)
			bb.br.p[i] = br->r;
		if(!intersect1(&bb, o, d)){
			DPRINTN(Debugray, "not intersecting; ");
			br = nil;
		}
	}
	DPRINTN(Debugray, "descending: %d %d\n",
		bl != nil ? (bl->child >> 2 << 1) : -1,
		br != nil ? (br->child >> 2 << 1) : -1);
	if(bl != nil){
		bb.tl = bp->tl;
		bb.br = bp->br;
		if(bb.tl.p[i] < bl->l)
			bb.tl.p[i] = bl->l;
		if(bb.br.p[i] > bl->r)
			bb.br.p[i] = bl->r;
		bl = tp->b + (bl->child >> 2 << 1);
		hits += traceray(bl, li, &bb, o, d, hits, tp);
	}
	if(br != nil){
		bb.tl = bp->tl;
		bb.br = bp->br;
		if(bb.tl.p[i] < br->l)
			bb.tl.p[i] = br->l;
		if(bb.br.p[i] > br->r)
			bb.br.p[i] = br->r;
		br = tp->b + (br->child >> 2 << 1);
		hits += traceray(br, ri, &bb, o, d, hits, tp);
	}
	return hits;
}

/* FIXME: 3d, from behind */
static ioff
dumbraytrace(Tree *tp)
{
	ioff tot;
	int x, y, w, h;
	Vec3 o, r;
	HMM_Vec3 v;

	w = view.w;
	h = view.h;
	o.x = view.eye.X;
	o.y = view.eye.Y;
	o.z = view.eye.Z;
	for(tot=0, y=0; y<h; y++){
		for(x=0; x<w; x++){
			//DPRINT(Debugray, "new ray: %d,%d", x, y);
			v = d2w(HMM_V2(x + 0.5f, y + 0.5f));
			r.x = v.X;
			r.y = v.Y;
			r.z = v.Z;
			tot += traceray(tp->b, tp->root.child & 3, &view.bb, &o, &r, 0, tp);
		}
	}
	return tot;
}

static ioff
dumberraytrace(Tree *)
{
	ioff i, tot;
	int x, y, w, h;
	Vec3 o, d;
	HMM_Vec3 v;
	Node *u;
	RNode *r, *re;

	w = view.w;
	h = view.h;
	o.x = view.eye.X;
	o.y = view.eye.Y;
	o.z = view.eye.Z;
	for(tot=0, y=0; y<h; y++){
		for(x=0; x<w; x++){
			//DPRINT(Debugray, "new ray: %d,%d", x, y);
			v = d2w(HMM_V2(x + 0.5f, y + 0.5f));
			d.x = v.X;
			d.y = v.Y;
			d.z = v.Z;
			for(i=0, u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++, i++){
				if(!intersect1w(r->pos, r->len, &o, &d))
					continue;
				if(!pushleaf(i))
					continue;
				setcolor(r->col, theme[Cemph]);
				DPRINT(Debugray, "[%d] %.2f,%.2f,%.2f len %.2f intersect %d,%d",
					i, r->pos[0], r->pos[1], r->pos[2], r->len*0.5f, x, y);
				tot++;
			}
		}
	}
	return tot;
}

/* FIXME: perform a bfs on the bih, so that we process it layer
 * by layer
 * - push objects that pass to the tail of the list: that way
 *   everything is automatically sorted by depth
 * - problem: if we do this instead of raycasting, we're screwed
 *	 with qball rotation, maybe change axis/orientation after all?
 * - if we can make bih construction faster, we can just
 *   recompute it even with qball rotation (we don't need to in
 *   2d mode)
 */
/* FIXME: alternative: perform a *dfs* on the bih to get stuff
 * at the back first; let's solve it for 2d first without qballs */
/* FIXME: fixed front axis */
static ioff
dfs(B *l, int i, AABB *bp, Cull *c)
{
	int j, k;
	ioff tot;
	float x, y;
	B *r;
	HMM_Vec3 v;
	Vec3 d;
	AABB bb;

	/* FIXME: bfs probably makes more sense, might resolve blinking
	 *	we should be adding no more than Max objects *per pixel*,
	 *	here we're adding way more (or are we??) */
	/*
	FIXME: we don't really need a bfs if we can get the checks right
		. we can know if there's an overlap between l and r at the split
		. we can also tell if the set of children is only intersecting
		  one pixel
		. if children intersect multiple pixels, we should be able to
		  "saturate" those, instead of all of these checks; so maybe
		  just keep some state about the number of hits per pixel?
		. the bih is not oriented toward the screen, that means that
		  in 3d, with qball rotation we need to parse it differently
		  ... or do we? solve for 2d first and verify 3d
	*/
	/* FIXME: print the tree (DOT) */

	/* FIXME: align all bb's to pixel grid, check that it isn't empty */

	/* FIXME: raytracing is expensive, we might be doing it too much */

	/* FIXME: 3d, and maybe too approximate */
	tot = 0;
	bb = *bp;
	if(bb.br.p[i] - bb.tl.p[i] <= c->w[i]){	/* longest axis */
		DPRINT(Debugray, "dfs: raytrace %.2f,%.2f,%.2f %.2f,%.2f,%.2f",
			bb.tl.x, bb.tl.y, bb.tl.z, bb.br.x, bb.br.y, bb.br.z);
		for(y=bb.tl.p[1]; y<bb.br.p[1]; y+=c->w[1]){
			for(x=bb.tl.p[0]; x<bb.br.p[0]; x+=c->w[0]){
				//DPRINT(Debugray, "new ray: %.2f,%.2f", x, y);
				//v = d2w(HMM_V2(x + 0.5f, y + 0.5f));
				//v = HMM_V3(x + 0.5f, y + 0.5f, 0.0f)
				/* FIXME: 3d: ray is fucked up */
				v = HMM_V3(x, y, 0.0f);
				v = HMM_NormV3(HMM_SubV3(v, view.eye));
				d.x = v.X;
				d.y = v.Y;
				d.z = v.Z;
				tot += traceray(l, i, &bb, &c->eye, &d, 0, c->tp);
			}
		}
		return tot;
	}
	r = l + 1;
	DPRINTS(Debugray, "dfs: l node %zd axis %d: ", l - c->tp->b, i);
	if((j = l->child & 3) == Fleaf){
		if(l->n > 0){
			DPRINTN(Debugray, "leaf [%d:%d]\n", l->off, l->n);
			tot += pushleavesif(&bb, c->w, &c->eye, c->tp->idx + l->off, l->n);
		}else
			DPRINTN(Debugray, "empty leaf\n");
		l = nil;
	}else if(l->l >= bb.br.p[i] || l->r < bb.tl.p[i]){
		DPRINTN(Debugray, "not visible\n");
		l = nil;
	}
	DPRINTS(Debugray, "dfs: r node %zd axis %d: ", r - c->tp->b, i);
	if((k = r->child & 3) == Fleaf){
		if(r->n > 0){
			DPRINTN(Debugray, "leaf [%d:%d]\n", r->off, r->n);
			tot += pushleavesif(&bb, c->w, &c->eye, c->tp->idx + r->off, r->n);
		}else
			DPRINTN(Debugray, "empty leaf\n");
		r = nil;
	}else if(r->l >= bb.br.p[i] || r->r < bb.tl.p[i]){
		DPRINTN(Debugray, "not visible\n");
		r = nil;
	}

	if(l != nil){
		if(bb.tl.p[i] < l->l)
			bb.tl.p[i] = l->l;
		if(bb.br.p[i] > l->r)
			bb.br.p[i] = l->r;
		l = c->tp->b + (l->child >> 2 << 1);
		DPRINT(Debugray, "l children %zd", l - c->tp->b);
		tot += dfs(l, j, &bb, c);
	}
	if(r != nil){
		if(l != nil){
			bb.tl.p[i] = bp->tl.p[i];
			bb.br.p[i] = bp->br.p[i];
		}
		if(bb.tl.p[i] < r->l)
			bb.tl.p[i] = r->l;
		if(bb.br.p[i] > r->r)
			bb.br.p[i] = r->r;
		r = c->tp->b + (r->child >> 2 << 1);
		DPRINT(Debugray, "r children %zd", r - c->tp->b);
		tot += dfs(r, k, &bb, c);
	}
	return tot;
}

static inline AABB
pixelbox(void)
{
	AABB bb;
	HMM_Vec3 a, b;

	a = s2w(HMM_V2(view.w * 0.5f + 0.0f, view.h * 0.5f - 0.0f));
	b = s2w(HMM_V2(view.w * 0.5f + 1.0f, view.h * 0.5f - 1.0f));
	if(a.X <= b.X){
		bb.tl.x = a.X;
		bb.br.x = b.X;
	}else{
		bb.tl.x = b.X;
		bb.br.x = a.X;
	}
	if(a.Y <= b.Y){
		bb.tl.y = a.Y;
		bb.br.y = b.Y;
	}else{
		bb.tl.y = b.Y;
		bb.br.y = a.Y;
	}
	if(a.Z <= b.Z){
		bb.tl.z = a.Z;
		bb.br.z = b.Z;
	}else{
		bb.tl.z = b.Z;
		bb.br.z = a.Z;
	}
	return bb;
}

/* FIXME: dfs still won't give us one bottom-up list, but
 * several */
static ioff
dfstrace(Tree *tp, int i)
{
	Cull c;

	c.px = pixelbox();
	c.w[0] = c.px.br.p[0] - c.px.tl.p[0];
	c.w[1] = c.px.br.p[1] - c.px.tl.p[1];
	c.w[2] = c.px.br.p[2] - c.px.tl.p[2];
	c.tp = tp;
	c.eye.x = view.eye.X;
	c.eye.y = view.eye.Y;
	c.eye.z = view.eye.Z;
	DPRINT(Debugray, "px %.2f,%.2f,%.2f %.2f,%.2f,%.2f",
		c.px.tl.x, c.px.tl.y, c.px.tl.z, c.px.br.x, c.px.br.y, c.px.br.z);
	return dfs(tp->b, i, &view.bb, &c);
}

static inline int
visible(RNode *r, AABB *b)
{
	float l;

	l = r->len * 0.5f;
	return r->pos[0] + l >= b->tl.x && r->pos[0] - l < b->br.x
		&& r->pos[1] + l >= b->tl.y && r->pos[1] - l < b->br.y
		&& r->pos[2] + l >= b->tl.z && r->pos[2] - l < b->br.z;
}

static ioff
noreject(void)
{
	ioff i, tot;
	AABB *vb;
	RNode *r, *re;
	Node *u;

	vb = &view.bb;
	for(tot=0, i=0, u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++, i++){
		u->dflags &= ~FNDvis;
		if(visible(r, vb))
			tot += pushleaf(i);
	}
	return tot;
}

ioff
raytrace(Tree *tp)
{
	int i;
	ioff tot;
	vlong t;
	Node *u, *ue;

	if(tp->b == nil){
		DPRINT(Debugray, "cullsort: no tree to hand");
		return noreject();
	}
	if((i = tp->root.child & 3) == Fleaf)
		die("cullsortnodes: bug: root is a leaf node");
	t = μsec();
	/* FIXME: kludge */
	for(u=nodes, ue=u+dylen(u); u<ue; u++)
		u->dflags &= ~FNDvis;
	TIME("raytrace", "kludge", t);
	//tot = dumbraytrace(tp);
	//tot = dumberraytrace(tp);
	tot = dfstrace(tp, i);
	TIME("raytrace", "trace", t);
	DPRINT(Debugray, "raytrace: %zd/%zd rnodes", dylen(rnodes), dylen(nodes));
	return tot;
}
