#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	W = 1024,
	H = 1024,
	Area = W * H,
};

#define	C	((float)Nodesz / (Maxsz - Minsz) * 0.7f)
#define Tolerance	0.001f

typedef struct P P;
typedef struct D D;
struct P{
	ioff e;
	short ne;
	short flags;
};
struct D{
	P *ptab;
	ioff *etab;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	cool(t)	(0.99985f * (t))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.0001f)
#define	Δ3(x, y, z)	(sqrtf((x) * (x) + (y) * (y) + (z) * (z)) + 0.0001f)

static void *
new_(Graph *g, int is3d)
{
	int orphans;
	ioff *e, *ee, *etab;
	double z;
	float k;
	Node *u, *ue;
	RNode *r, *re;
	P p, *ptab;
	D *aux;

	ptab = nil;
	etab = nil;
	k = C * sqrtf((float)Area / dylen(rnodes));
	for(u=g->nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		if((u->attr.flags & FNinitx) != 0)
			r->pos[0] = u->attr.pos0.x;
		else
			r->pos[0] = (float)(W / 2 - nrand(W)) / (W / 2);
		if((u->attr.flags & FNinity) != 0)
			r->pos[1] = u->attr.pos0.y;
		else
			r->pos[1] = (float)(H / 2 - nrand(H)) / (H / 2);
		if(!is3d){
			z = (double)(dylen(rnodes) - (r - rnodes)) / dylen(rnodes);
			r->pos[2] = (view.flags & VFnodepth) == 0
				? 0.8 * (0.5 - z)
				: 0.00001 * z;
		}else
			r->pos[2] = (float)(W / 2 - nrand(W)) / (W / 2);
	}
	orphans = 0;
	for(r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++){
		p.e = dylen(etab);
		p.ne = 0;
		p.flags = u->attr.flags & FNfixed;
		/* FIXME: have to look at twice as many edges because it's not
		 * global... */
		if((u->attr.flags & FNfixed) != FNfixed){
			for(e=u->out, ee=e+dylen(e); e<ee; e++, p.ne++)
				dypush(etab, *e >> 2);
			for(e=u->in, ee=e+dylen(e); e<ee; e++, p.ne++)
				dypush(etab, *e >> 2);
		}
		if(p.ne == 0)
			orphans++;
		dypush(ptab, p);
	}
	if(orphans > 1)
		logmsg(va("layout: ignoring %d nodes with no adjacencies", orphans));
	aux = emalloc(sizeof *aux);
	aux->ptab = ptab;
	aux->etab = etab;
	aux->k = k;
	return aux;
}

static void *
new(Graph *g)
{
	return new_(g, 0);
}

static void *
new3d(Graph *g)
{
	return new_(g, 1);
}

static void
cleanup(void *p)
{
	D *aux;

	aux = p;
	dyfree(aux->ptab);
	dyfree(aux->etab);
	free(aux);
}

/* FIXME: find more intelligent way to merge the two */
static int
compute3d(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *e, *ee;
	float t, tol, k, f, x, y, z, Δx, Δy, Δz, δx, δy, δz, δ, w, uw, vw, Δr;
	RNode *r0, *r1, *r, *v;
	P *pp, *p0;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = k;
	tol = Tolerance * k;
	p0 = d->ptab + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(rnodes);
	skip = nlaythreads;
	for(;;){
		CLK0(clk);
		Δr = 0;
		for(pp=p0, r=r0; r<r1; r+=skip, pp+=skip){
			if((*stat & LFstop) != 0)
				return 0;
			fixed = pp->flags & FNfixed;
			if(fixed == FNfixed || pp->ne == 0)
				continue;
			uw = r->len;
			x = r->pos[0];
			y = r->pos[1];
			z = r->pos[2];
			Δx = Δy = Δz = 0.0f;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δz = z - v->pos[2];
				δ = Δ3(δx, δy, δz);
				f = Fr(δ, k);
				vw = v->len;
				w = C * MIN(uw, vw);
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
				Δz += w * f * δz / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+pp->e, ee=e+pp->ne; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δz = v->pos[2] - z;
				δ = Δ3(δx, δy, δz);
				f = Fa(δ, k);
				vw = v->len;
				if(uw < vw)
					w = uw / vw;
				else
					w = vw / uw;
				w *= C;
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
				Δz += w * f * δz / δ;
			}
			δ = Δ3(Δx, Δy, Δz);
			if((fixed & FNfixedx) == 0){
				f = MIN(t, fabsf(Δx));
				Δx = f * Δx / δ;
				x += Δx;
				r->pos[0] = x;
				if(Δr < Δx)
					Δr = Δx;
			}
			if((fixed & FNfixedy) == 0){
				f = MIN(t, fabsf(Δy));
				Δy = f * Δy / δ;
				y += Δy;
				r->pos[1] = y;
				if(Δr < Δy)
					Δr = Δy;
			}
			f = MIN(t, fabs(Δz));
			Δz = f * Δz / δ;
			z += Δz;
			r->pos[2] = z;
			if(Δr < Δz)
				Δr = Δz;
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

/* skipping through nodes ensures that each thread works on a more
 * "global" part of the graph, otherwise the quality of the layout
 * is horrible; preparing the data in contiguous chunks in some
 * manner would be better but more complicated, as it stands,
 * performance doesn't really suffer because of this, possibly
 * because other threads would be accessing what was prefetched
 * anyway */
static int
compute(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *e, *ee;
	float t, tol, k, f, x, y, Δx, Δy, δx, δy, δ, w, uw, vw, Δr;
	RNode *r0, *r1, *r, *v;
	P *pp, *p0;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = k;
	tol = Tolerance * k;
	p0 = d->ptab + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(rnodes);
	skip = nlaythreads;
	for(;;){
		CLK0(clk);
		Δr = 0;
		for(pp=p0, r=r0; r<r1; r+=skip, pp+=skip){
			if((*stat & LFstop) != 0)
				return 0;
			fixed = pp->flags & FNfixed;
			if(fixed == FNfixed)
				continue;
			uw = r->len;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0.0f;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				vw = v->len;
				w = C * MIN(uw, vw);
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+pp->e, ee=e+pp->ne; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				vw = v->len;
				if(uw < vw)
					w = uw / vw;
				else
					w = vw / uw;
				w *= C;
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			δ = Δ(Δx, Δy);
			/* limiting layouting area doesn't work well for long linear graphs */
			if((fixed & FNfixedx) == 0){
				f = MIN(t, fabsf(Δx));
				Δx = f * Δx / δ;
				x += Δx;
				r->pos[0] = x;
				if(Δr < Δx)
					Δr = Δx;
			}
			if((fixed & FNfixedy) == 0){
				f = MIN(t, fabsf(Δy));
				Δy = f * Δy / δ;
				y += Δy;
				r->pos[1] = y;
				if(Δr < Δy)
					Δr = Δy;
			}
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

static Target ll = {
	.name = "pfr",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

static Target ll3d = {
	.name = "pfr3d",
	.new = new3d,
	.cleanup = cleanup,
	.compute = compute3d,
};

Target *
regpfr3d(void)
{
	return &ll3d;
}
Target *
regpfr(void)
{
	return &ll;
}
