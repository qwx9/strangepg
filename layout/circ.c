#include "strpg.h"
#include "layout.h"
#include "drw.h"
#include "graph.h"

enum{
	W = 256,
	L = 256,
	Area = W * L,
};

typedef struct P P;
typedef struct D D;
struct P{
	ioff e;
	int nin;
	int nout;
	char fixed;
	char dafuq;
};
struct D{
	P *ptab;
	ioff *etab;
	float k;
};

#define Fa(x, k)	((x) * (x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.00001)
#define	cool(t)	((t) - 0.0001f > 0 ? (t) - 0.0001f : 0.0f)

static void *
new(Graph *g)
{
	uint min, max;
	ioff i, iv, ie, *e, *ee, *etab;
	float n, ρ, ρ1, θ, x, y;
	Node *u, *v, *ue;
	RNode *r, *re;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = nil;
	max = 0;
	min = ~0;

	for(i=0, r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++, i++){
		p.e = dylen(etab);
		p.nout = dylen(u->out);
		p.nin = dylen(u->in);
		for(e=u->out, ee=e+p.nout; e<ee; e++){
			iv = g->edges[*e].v >> 1;
			dypush(etab, iv);
		}
		for(e=u->in, ee=e+p.nin; e<ee; e++){
			iv = g->edges[*e].u >> 1;
			dypush(etab, iv);
		}
		/* FIXME: bug in data prod? */
		if(p.e == dylen(etab)){
			p.dafuq = 1;
			goto skip;
		}
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			x = u->pos0.x;
			y = u->pos0.y;
			if(max < x)
				max = x;
			if(min > x)
				min = x;
			if((u->flags & FNfixed) != 0)
				p.fixed = 1;
			else
				p.fixed = 2;
		}
		r->pos[0] = x;
		r->pos[1] = y;
skip:
		dypush(ptab, p);
	}
	/* FIXME */
	//ρ1 = Vdefw / 1.5f - Nodesz / 2;
	ρ1 = (max - min) / 4;
	n = 0;
	for(pp=ptab, r=rnodes, re=r+dylen(r); r<re; r++, pp++){
		if(!pp->fixed)
			continue;
		n++;
		ρ = ρ1;
		ρ -= (n / (max - min)) * Nodesz * 4;
		θ = n * Nodesz / ρ;
		if(pp->fixed == 2)
			ρ = 0.0f;
		r->pos[0] = ρ * cos(θ);
		r->pos[1] = ρ * -sin(θ);
	}
	aux = emalloc(sizeof *aux);
	aux->ptab = ptab;
	aux->etab = etab;
	aux->k = 1 * sqrt((float)Area / dylen(ptab));
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

static int
compute(void *arg, volatile int *stat, int i)
{
	int c, Δ;
	double dt;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ioff *e, *ee, *fp, *f0, *f1;
	RNode *r0, *r1, *r, *v;
	P *pp, *vp, *u, *p0, *p1;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	pp = d->ptab;
	k = d->k;
	t = 1.0;
	p0 = pp + i;
	p1 = pp + dylen(pp);
	if(p1 > pp + dylen(pp))
		p1 = pp + dylen(pp);
	for(c=0;; c++){
		Δr = 0;
		for(u=p0; u<p1; u+=nlaythreads){
			if((*stat & LFstop) != 0)
				return 0;
			u = pp + i;
			if(u->fixed)
				continue;
			r = rnodes + i;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0;
			for(v=rnodes, vp=pp; v<r1; v++, vp++){
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
			for(e=d->etab+u->e, ee=e+u->nout; e<ee; e++){
				v = rnodes + *e;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=u->nin; e<ee; e++){
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
			x += δx / δ;
			y += δy / δ;
			r->pos[0] = x;
			r->pos[1] = y;
			if(δx != 0.0f){	/* FIXME */
				r->dir[0] = δx;
				r->dir[1] = δy;
			}
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 1.0)
			break;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 100000.0);
		t = 1.0 - dt * dt * dt * dt;
		CLK1(clk);
	}
	return 0;
}

static Shitkicker ll = {
	.name = "circ",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
regcirc(void)
{
	return &ll;
}
