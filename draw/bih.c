#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"
#include "threads.h"

/* based on Wächter and Keller, Instant Ray Tracing: The Bounding
 * Interval Hierarchy, 2006 */
/* note: this again races against layouting */

/* FIXME: implement dump and check the tree */
/* FIXME: profile, figure out what is slow and if the preproc is
 * helping at all */
/* FIXME: might make sense to use doubles at least in some places */
/* FIXME: some nodes show up as not visible even at the start, maybe b/c
 * of layouting not being done yet → mc -T 7000000, minigraph */

typedef struct BA BA;
typedef struct Grid Grid;
typedef struct BIH BIH;

#define Gridf	(1.0 / 5)	/* FIXME: tune */
#define	Giveup	0.01		/* FIXME: tune */

enum{
	Nmaxobj = 32,			/* FIXME: tune */
};

struct BA{		/* cell split */
	ioff *l;
	ioff *r;
};
struct Grid{
	AABB b;		/* total aabb */
	ioff off;
	ioff n;
};
struct BIH{
	AABB b;		/* bounding box aligned to other two axes */
	Tree t;
	Grid *g;
	int w;
	int h;
	int ax;		/* grid front axis */
	BA *ql;		/* index stacks */	/* FIXME: sucks */
	BA *qr;
};

static uint	split(BIH*, AABB*, BA**, ioff);

static inline int
longest(AABB b)
{
	float w, h, d;

	w = b.br.x - b.tl.x;
	h = b.br.y - b.tl.y;
	d = b.br.z - b.tl.z;
	if(w >= h){
		if(w >= d)
			return Fxaxis;
		else
			return Fzaxis;
	}else if(h >= d)
		return Fyaxis;
	else
		return Fzaxis;
}

static inline int
shortest(AABB b)
{
	float w, h, d;

	w = b.br.x - b.tl.x;
	h = b.br.y - b.tl.y;
	d = b.br.z - b.tl.z;
	if(w < h){
		if(w < d)
			return Fxaxis;
		else
			return Fzaxis;
	}else if(h < d)
		return Fyaxis;
	else
		return Fzaxis;
}

static void
dump(BIH *bih)
{
	DPRINT(Debugbih, "tree size: %zd grid %dx%d",
		dylen(bih->t.b), bih->w, bih->h);
	/* FIXME: check tree: overflowing indices, etc. */
}

static inline void
swap(ioff *to, ioff *from)
{
	ioff t;

	if(to != from){
		t = *to;
		*to = *from;
		*from = t;
	}
}

/* avoids queues for sorting but does that incur a performance penalty? */
static void
splitcell(BIH *bih, BA *Q, int i, float bmin, float bmax, B *bl, B *br, double m, int *nl, int *nr)
{
	float Δl, Δr, min, max, d;
	ioff *p, *r0, *l0, *l, *r, *lr, *rl;
	RNode *u;
	BA q;

	l = lr = l0 = Q->l;	/* overwritten unless passed by value */
	r = rl = r0 = Q->r;
	DPRINTN(Debugbih, "splitcell [%.3f %.3f]: ", bmin, bmax);
	for(p=l; p<rl;){
		u = rnodes + *p;
		d = u->len * 0.5f;
		min = max = u->pos[i];
		if(i != Fzaxis || drawing.flags & DF3d){
			min -= d;
			max += d;
		}
		Δl = m - min;
		Δr = max - m;
		if(min < bmin)
			min = bmin;
		else if(min > bmax)
			min = bmax;
		if(max > bmax)
			max = bmax;
		else if(max < bmin)
			max = bmin;
		/* FIXME: can be hit b/c layouting */
		assert(min <= max);
		if(Δr < 0){			/* non-overlapping l */
			swap(l++, p++);
			lr++;	/* swapping p either with lr, or with itself */
			if(bl->l > min)
				bl->l = min;
			if(bl->r < max)
				bl->r = max;
			DPRINTN(Debugbih, "[L l=%.2f %.2f,%.2f,%.2f %.2f,%.2f]",
				d, u->pos[0], u->pos[1], u->pos[2], min, max);
		}else if(Δl < 0){	/* non-overlapping r */
			swap(--r, p);
			if(rl <= r)		/* non-empty rl */
				swap(rl-1, p);
			rl--;
			if(br->l > min)
				br->l = min;
			if(br->r < max)
				br->r = max;
			DPRINTN(Debugbih, "[R l=%.2f %.2f,%.2f,%.2f %.2f,%.2f]",
				d, u->pos[0], u->pos[1], u->pos[2], min, max);
		}else if(Δl >= Δr){	/* overlapping l */
			p++;	/* already at the lr border */
			lr++;
			if(bl->l > min)
				bl->l = min;
			if(bl->r < max)
				bl->r = max;
			DPRINTN(Debugbih, "[LR l=%.2f %.2f,%.2f,%.2f %.2f,%.2f]",
				d, u->pos[0], u->pos[1], u->pos[2], min, max);
		}else{				/* overlapping r */
			swap(--rl, p);
			if(br->l > min)
				br->l = min;
			if(br->r < max)
				br->r = max;
			DPRINTN(Debugbih, "[RL l=%.2f %.2f,%.2f,%.2f %.2f,%.2f]",
				d, u->pos[0], u->pos[1], u->pos[2], min, max);
		}
	}
	if(lr > l0){
		q.l = l0;
		q.r = rl;
		dypush(bih->ql, q);
		(*nl)++;
	}
	if(rl < r0){
		q.l = rl;
		q.r = r0;
		dypush(bih->qr, q);
		(*nr)++;
	}
}

/* FIXME: it being unordered doesn't help us much */
/* FIXME: order? */
/* special case: tons of superimposed geometry close together,
 * we won't be able to separate it and just run out of stack;
 * won't even attempt to sort in any way */
static inline uint
slice(BIH *bih, BA *Q, AABB *bp, ioff nq)
{
	ioff o, op;
	B b, c;
	BA *q, *qe;

	o = op = dylen(bih->t.b);
	b.child = Fleaf;
	c.l = bp->tl.z;
	c.r = bp->br.z;
	for(qe=Q+dylen(Q), q=qe-nq; q<qe; q++){
		b.off = q->l - bih->t.idx;
		b.n = q->r - q->l;
		dypush(bih->t.b, b);
		if(q == qe - 1){
			b.off = b.n = 0;
			dypush(bih->t.b, b);
			break;
		}
		o += 2;
		c.child = (uint)o >> 1 << 2 | Fzaxis;
		dypush(bih->t.b, c);
	}
	assert(dylen(Q) >= nq);
	_dylen(Q) -= nq;	/* FIXME */
	return (uint)op >> 1 << 2 | Fzaxis;
}

/* try our damnest not to overflow the stack on endless splits while
 * layouting bunches shit together */
static inline int
pushchild(BIH *bih, AABB *bp, BA **Q, ioff nq, ioff o)
{
	ioff c;
	B *b;
	BA *q;

	b = bih->t.b + o;
	q = *Q;
	q += dylen(q) - nq;
	if(nq == 0){
		b->child = Fleaf;
		b->off = b->n = 0;
		DPRINT(Debugbih, "pushch [%d,%d] → empty leaf", o, nq);
	}else if(bp->br.x - bp->tl.x + bp->br.y - bp->tl.y + bp->br.z - bp->tl.z < Giveup){
		DPRINT(Debugbih, "pushch [%d,%d] → slice [%.2f %.2f %.2f] [%.2f %.2f %.2f]",
			o, nq, bp->tl.x, bp->tl.y, bp->tl.z, bp->br.x, bp->br.y, bp->br.z);
		if((c = slice(bih, *Q, bp, nq)) < 0)
			return -1;
		b = bih->t.b + o;
		b->child = c;
	}else if(nq == 1 && q->r - q->l <= Nmaxobj){
		q = dypop(*Q);
		assert(q != nil);
		b->off = q->l - bih->t.idx;
		b->n = q->r - q->l;
		b->child = Fleaf;
		DPRINT(Debugbih, "pushch [%d,%d] → leaf off=%d n=%d",
			o, nq, b->off, b->n);
	}else{
		DPRINT(Debugbih, "pushch [%d,%d] → split [%.2f %.2f %.2f] [%.2f %.2f %.2f] ql %d",
			o, nq, bp->tl.x, bp->tl.y, bp->tl.z, bp->br.x, bp->br.y, bp->br.z,
			nq);
		if((c = split(bih, bp, Q, nq)) < 0)
			return -1;
		b = bih->t.b + o;	/* could be invalidated if set earlier */
		b->child = c;
	}
	return 0;
}

static uint
split(BIH *bih, AABB *bp, BA **Q, ioff nq)
{
	int i;
	ioff o, coff, nl, nr, t, h, hh;
	float bmin, bmax;
	double m;
	B bl, br;
	BA *q;
	AABB bb;

	assert(nq > 0);
	i = longest(*bp);
	bmin = bp->tl.p[i];
	bmax = bp->br.p[i];
	m = (bmin + bmax) / 2.0;
	bl.l = m;
	bl.r = bmin;
	br.l = bmax;
	br.r = m;
	DPRINT(Debugbih, "split [%d] ax=%d %.3f [%.3f] %.3f",
		nq, i, bmin, m, bmax);
	nl = nr = 0;
	for(h=dylen(*Q), t=h-nq, o=t; o<h; o++){
		q = *Q + o;
		DPRINTS(Debugbih, "off [%zd-%zd] ", q->l-bih->t.idx, q->r-bih->t.idx);
		splitcell(bih, q, i, bmin, bmax, &bl, &br, m, &nl, &nr);
		DPRINTN(Debugbih, "\n");
	}
	q = *Q;
	hh = dylen(q) - h;
	if(hh > 0)
		memmove(q+t, q+h, hh * sizeof *q);
	_dylen(q) -= nq;
	o = dylen(bih->t.b);
	dypush(bih->t.b, bl);
	dypush(bih->t.b, br);
	coff = -1;
	DPRINT(Debugbih, "L[%d] %.2f,%.2f R[%d] %.2f,%.2f orig %.2f,%.2f",
		nl, bl.l, bl.r, nr, br.l, br.r, bmin, bmax);
	bb = *bp;
	bb.br.p[i] = m;
	if(pushchild(bih, &bb, &bih->ql, nl, o) < 0)
		goto err;
	bb.tl.p[i] = m;
	bb.br.p[i] = bmax;
	if(pushchild(bih, &bb, &bih->qr, nr, o+1) < 0)
		goto err;
	coff = (uint)o >> 1 << 2 | i;
err:
	return coff;
}

static int
splitroot(BIH *bih)
{
	int i, r, nl, nr;
	float mt, min, max, Δl, Δr, bmin, bmax;
	double m;
	ioff o;
	Grid *c, *e;
	B bl, br;
	BA q;
	AABB bb;

	bb = bih->b;
	i = longest(bb);
	bmin = bb.tl.p[i];
	bmax = bb.br.p[i];
	m = (bmin + bmax) / 2.0;
	bl.l = m;
	bl.r = bmin;
	br.l = bmax;
	br.r = m;
	DPRINT(Debugbih, "splitroot ax=%d %.3f,%.3f mid %.3f",
		i, bmin, bmax, m);
	dyprealloc(bih->ql, bih->w * bih->h);	/* FIXME: overkill? */
	dyprealloc(bih->qr, bih->w * bih->h);
	nl = nr = 0;
	for(c=bih->g, e=c+bih->w*bih->h; c<e; c++){
		min = c->b.tl.p[i];
		max = c->b.br.p[i];
		DPRINTS(Debugbih, "cell[%zd] %.3f %.3f: ", c-bih->g, min, max);
		if(min == max){	/* empty */
			DPRINTN(Debugbih, "empty\n");
			continue;
		}
		if(min > max){
			mt = min;
			min = max;
			max = mt;
		}
		Δl = m - min;
		Δr = max - m;
		if(Δr < 0){			/* non-overlapping l */
			if(bl.l > min)
				bl.l = min;
			if(bl.r < max)
				bl.r = max;
			q.l = bih->t.idx + c->off;
			q.r = q.l + c->n;
			dypush(bih->ql, q);
			nl++;
			DPRINTN(Debugbih, "[L]");
		}else if(Δl < 0){	/* non-overlapping r */
			if(br.l > min)
				br.l = min;
			if(br.r < max)
				br.r = max;
			q.l = bih->t.idx + c->off;
			q.r = q.l + c->n;
			dypush(bih->qr, q);
			nr++;
			DPRINTN(Debugbih, "[R]");
		}else{				/* overlapping r or l */
			q.l = bih->t.idx + c->off;
			q.r = q.l + c->n;
			splitcell(bih, &q, i, bmin, bmax, &bl, &br, m, &nl, &nr);
		}
		DPRINTN(Debugbih, ": L [%.3f,%.3f] R [%.3f,%.3f]\n",
			bl.l, bl.r, br.l, br.r);
	}
	DPRINT(Debugbih, "L[%d] %.2f,%.2f R[%d] %.2f,%.2f orig %.2f,%.2f",
		nl, bl.l, bl.r, nr, br.l, br.r, bmin, bmax);
	o = dylen(bih->t.b);
	dypush(bih->t.b, bl);
	dypush(bih->t.b, br);
	r = -1;
	bb.br.p[i] = m;
	if(pushchild(bih, &bb, &bih->ql, nl, o) < 0)
		goto err;
	bb.tl.p[i] = m;
	bb.br.p[i] = bmax;
	if(pushchild(bih, &bb, &bih->qr, nr, o+1) < 0)
		goto err;
	r = 0;
	DPRINT(Debugbih, "queue sizes: %zd %zd", _dysz(bih->ql), _dysz(bih->qr));
err:
	dyfree(bih->ql);
	dyfree(bih->qr);
	bih->t.root.child = (uint)o >> 1 << 2 | i;
	bih->t.root.l = bmin;
	bih->t.root.r = bmax;
	return r;
}

/* sort objects into buckets by the top left corner of their 2d aabb */
static int
presort(BIH *bih)
{
	int ax, i, j, x, y, w, h;
	float f, l, fw, fh;
	ioff o, t, *idx, *ids, *ip, *ie;
	Vec3 p;
	RNode *u, *ue;
	AABB *bp, bb;
	Grid *g, *c;

	switch(ax = shortest(bih->b)){
	case Fxaxis: i = 1; j = 2; break;
	case Fyaxis: i = 2; j = 0; break;
	case Fzaxis: i = 0; j = 1; break;
	default: sysfatal("presort: invalid axis %d", ax);
	}
	f = Gridf / drawing.meansz;
	bp = &bih->b;
	fw = bp->br.p[i] - bp->tl.p[i];
	fh = bp->br.p[j] - bp->tl.p[j];
	if(fw <= 0 || fh <= 0){
		werrstr("empty/invalid bounding box: %.2f,%.2f,%.2f %.2f,%.2f,%.2f",
			bp->tl.x, bp->tl.y, bp->tl.z, bp->br.x, bp->br.y, bp->br.z);
		return -1;
	}
	ids = nil;
	w = ceilf(fw * f);
	h = ceilf(fh * f);
	DPRINT(Debugbih, "presort: %d x %d grid front=%d f %f msz %f gridf %f, size %zd, dim %f,%f",
		w, h, ax, f, drawing.meansz, Gridf, w*h*sizeof(*g), fw, fh);
	assert(w > 0 && h > 0);
	g = emalloc(w * h * sizeof *g);
	for(u=rnodes, ue=u+dylen(u); u<ue; u++){
		p.x = u->pos[0];
		p.y = u->pos[1];
		p.z = u->pos[2];
		/* FIXME: must be recomputed on next frame in this case */
		/* we're racing against layouting here at startup */
		if(isnan(p.p[i]) || isnan(p.p[j]))
			continue;
		l = u->len * 0.5f;
		/* FIXME: shouldn't be setting this flag ourselves */
		bb.tl.x = p.x - l;
		bb.tl.y = p.y - l;
		bb.tl.z = p.z;
		bb.br.x = p.x + l;
		bb.br.y = p.y + l;
		bb.br.z = p.z;
		if(drawing.flags & DF3d){
			bb.tl.z -= l;
			bb.br.z += l;
		}
		DPRINTS(Debugbih, "[%.2f,%.2f len=%f] ⇒ [%.2f,%.2f %.2f,%.2f]",
			p.p[i], p.p[j], l, bb.tl.p[i], bb.tl.p[j], bb.br.p[i], bb.br.p[j]);
		fw = bb.tl.p[i] - bp->tl.p[i];
		fh = bb.tl.p[j] - bp->tl.p[j];
		x = floorf(fw * f);
		y = floorf(fh * f);
		/* FIXME: racing against layout */
		if(x < 0 || x >= w || y < 0 || y >= h){
			DPRINTN(Debugbih, " oob! ignoring\n");
			continue;
		}
		o = w * y + x;
		DPRINTN(Debugbih, " o %d xy %d,%d\n", o, x, y);
		c = g + o;
		dypush(ids, o);
		assert(c >= g && c < g + w*h);
		o = u - rnodes;
		dypush(ids, o);
		if(c->n++ > 0)
			expandbb(&c->b, &bb);
		else
			c->b = bb;
	}
	for(o=0, c=g; c<g+w*h; c++){
		t = c->n;
		c->off = o + t;
		o += t;
	}
	if(o == 0){
		werrstr("no visible nodes");
		free(g);
		return -1;
	}
	DPRINT(Debugbih, "ids: %zd bytes, %zd elements", 8+8+dylen(ids)*sizeof(*ids), dylen(ids));
	DPRINT(Debugbih, "idx: %zd bytes", o*sizeof(*idx));
	DPRINT(Debugbih, "%d/%zd nodes in view box", o, dylen(rnodes));
	idx = emalloc(o * sizeof *idx);
	bih->t.idx = idx;
	for(ip=ids, ie=ip+dylen(ip); ip<ie;){
		o = *ip++;
		c = g + o;
		idx[--c->off] = *ip++;
	}
	dyfree(ids);
	bih->g = g;
	bih->w = w;
	bih->h = h;
	bih->ax = ax;
	return 0;
}

/* FIXME: this races against layout, we can't assume it's correct */
/* FIXME: hack 0: store previous bih's AABB, and just adjust it? */
/* FIXME: hack 2: compare against previous bih, skip if unchanged */
/* FIXME: hack 3: check a flag if moved, visible, etc.; note that we have
 * to rebuild the index and thus the tree, unless we switch to some other
 * method of storing shit in leaves, maybe just pointing to a new range
 * past the end or something; maybe a gencount */
/* FIXME: possible hack: early intersection test with vbox, we'll do one
 * during presort anyway */
/* FIXME: maybe we ought to cache aabb, not len? */
/* for convenience, the bb corners store the minima (tl) and maxima (br)
 * along the 3 axes, flipping the y and z axis; in 2d mode we flatten
 * the bb in the front dimension to get better and cheaper separation */
static void
scenebox(BIH *bih)
{
	int k;
	float l;
	RNode *r, *re;
	Node *u;
	Vec3 p;
	AABB b, ob;

	k = drawing.flags & DF3d;
	memset(&b, 0, sizeof b);
	for(u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		if(u->nedges == 0)	/* FIXME: for debug, artificially stretch bb */
			continue;
		l = r->len * 0.5f;
		p.x = r->pos[0];
		p.y = r->pos[1];
		p.z = r->pos[2];
		ob.tl.x = p.x - l;
		ob.tl.y = p.y - l;
		ob.tl.z = p.z;
		ob.br.x = p.x + l;
		ob.br.y = p.y + l;
		ob.br.z = p.z;
		if(k){
			ob.tl.z -= l;
			ob.br.z += l;
		}
		if(r != rnodes)
			expandbb(&b, &ob);
		else
			b = ob;
	}
	DPRINT(Debugbih, "box: %.2f,%.2f,%.2f %.2f,%.2f,%.2f ",
		b.tl.x, b.tl.y, b.tl.z, b.br.x, b.br.y, b.br.z);
	/* intersect done in render thread unless we recompute on view change
	//intersectbb(&b, &view.bb);
	DPRINTN(Debugbih, "⇒ %.2f,%.2f,%.2f %.2f,%.2f,%.2f\n",
		b.tl.x, b.tl.y, b.tl.z, b.br.x, b.br.y, b.br.z);
	*/
	bih->b = b;
}

void
nuketree(Tree *tp)
{
	free(tp->idx);
	dyfree(tp->b);
}

int
mktree(Tree *tp)
{
	int r;
	BIH h;
	vlong t;

	assert(view.ar != 0.0f);
	assert(drawing.meansz != 0.0);
	memset(&h, 0, sizeof h);
	t = μsec();
	scenebox(&h);
	TIME("bih", "scenebox", t);
	if(presort(&h) < 0)
		return -1;
	TIME("bih", "presort", t);
	r = splitroot(&h);
	free(h.g);
	if(r < 0){
		nuketree(&h.t);
		return -1;
	}
	h.g = nil;
	TIME("bih", "mktree", t);
	if(debug & Debugbih)
		dump(&h);
	*tp = h.t;
	return 0;
}
