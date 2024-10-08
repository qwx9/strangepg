#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	W = 1024,
	H = 1024,
	Area = W * H,
};

#define C	0.065f
#define Tolerance	3.0f

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
#define	cool(t)	(0.9995f * (t))
#define	Δ(x, y, z)	(sqrtf((x) * (x) + (y) * (y) + (z) * (z)) + 0.0001f)

static void *
new(Graph *g)
{
	ioff iv, *e, *ee, *etab;
	double z;
	float k;
	Node *u, *ue;
	RNode *r, *re;
	P p, *ptab;
	D *aux;

	ptab = nil;
	etab = nil;
	k = C * sqrtf((float)Area / dylen(rnodes));
	/* force shrink edges for smaller graphs */
	if(dylen(rnodes) < 1000)
		k /= log10(dylen(redges));
	for(u=g->nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		if((u->flags & FNinitx) != 0)
			r->pos[0] = u->pos0.x;
		else
			r->pos[0] = (float)(W / 2 - nrand(W)) / (W / 2);
		if((u->flags & FNinity) != 0)
			r->pos[1] = u->pos0.y;
		else
			r->pos[1] = (float)(H / 2 - nrand(H)) / (H / 2);
		z = (double)(dylen(rnodes) - (r - rnodes)) / dylen(rnodes);
		r->pos[2] = 0.8 * (0.5 - z);
	}
	for(r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++){
		p.e = dylen(etab);
		p.ne = 0;
		p.flags = u->flags & FNfixed;
		if((u->flags & FNfixed) != FNfixed){
			for(e=u->out, ee=e+dylen(e); e<ee; e++, p.ne++){
				iv = g->edges[*e].v >> 1;
				dypush(etab, iv);
			}
			for(e=u->in, ee=e+dylen(e); e<ee; e++, p.ne++){
				iv = g->edges[*e].u >> 1;
				dypush(etab, iv);
			}
		}
		dypush(ptab, p);
	}
	aux = emalloc(sizeof *aux);
	aux->ptab = ptab;
	aux->etab = etab;
	aux->k = k;
	return aux;
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
	int skip, fixed;
	ioff *e, *ee;
	float t, tol, k, f, x, y, z, Δx, Δy, Δz, δx, δy, δz, δ, w, Δr;
	RNode *r0, *r1, *r, *v;
	P *pp, *p0;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = k;
	tol = Tolerance * k;
	/* heuristic: converge faster with smaller graphs */
	if(dylen(rnodes) <= 1000)
		tol /= 2.0f;
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
			w = r->len;
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
				δ = Δ(δx, δy, δz);
				f = Fr(δ, k);
				Δx += (w / v->len) * f * δx / δ;
				Δy += (w / v->len) * f * δy / δ;
				Δz += (w / v->len) * f * δz / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+pp->e, ee=e+pp->ne; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δz = v->pos[2] - z;
				δ = Δ(δx, δy, δz);
				f = Fa(δ, k);
				Δx += (w / v->len) * f * δx / δ;
				Δy += (w / v->len) * f * δy / δ;
				Δz += (w / v->len) * f * δz / δ;
			}
			δ = Δ(Δx, Δy, Δz);
			if((fixed & FNfixedx) == 0){
				f = MIN(t, fabs(Δx));
				x += f * Δx / δ;
				r->pos[0] = x;
			}
			if((fixed & FNfixedy) == 0){
				f = MIN(t, fabs(Δy));
				y += f * Δy / δ;
				r->pos[1] = y;
			}
			f = MIN(t, fabs(Δz));
			z += f * Δz / δ;
			r->pos[2] = z;
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

static Target ll = {
	.name = "pfr3d",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Target *
regpfr3d(void)
{
	return &ll;
}
