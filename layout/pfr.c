#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	Length = 256,
};
#define C	0.1

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
	double k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	cool(t)	(0.995 * (t))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)

static void *
new(Graph *g)
{
	int n;
	ioff iv, *e, *ee, *etab;
	double k;
	Node *u, *ue;
	RNode *r, *re;
	P p, *ptab;
	D *aux;

	ptab = nil;
	etab = nil;
	n = 2;
	k = C * sqrt((double)(Length * Length) / dylen(rnodes));
	for(u=g->nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		if((u->flags & FNinitx) != 0)
			r->pos[0] = u->pos0.x;
		else
			r->pos[0] = nrand(2 * n) - n;
		if((u->flags & FNinity) != 0)
			r->pos[1] = u->pos0.y;
		else
			r->pos[1] = nrand(2 * n) - n;
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
	int Δ;
	ioff *e, *ee;
	double t, tol, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	RNode *r0, *r1, *r, *v;
	P *pp, *p0;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = k;
	tol = 0.01 * k;
	p0 = d->ptab + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(rnodes);
	Δ = nlaythreads;
	for(;;){
		CLK0(clk);
		Δr = 0;
		for(pp=p0, r=r0; r<r1; r+=Δ, pp+=Δ){
			if((*stat & LFstop) != 0)
				return 0;
			if((pp->flags & FNfixed) == FNfixed)
				continue;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+pp->e, ee=e+pp->ne; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx += rx;
				Δy += ry;
			}
			δx = t * Δx;
			δy = t * Δy;
			δ = Δ(δx, δy);
			if((pp->flags & FNfixedx) == 0){
				x += δx / δ;
				r->pos[0] = x;
			}
			if((pp->flags & FNfixedy) == 0){
				y += δy / δ;
				r->pos[1] = y;
			}
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

static Shitkicker ll = {
	.name = "pfr",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
regpfr(void)
{
	return &ll;
}
