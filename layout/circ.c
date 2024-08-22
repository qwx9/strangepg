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

#define Fa(x, k)	((x) * (x) * (x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.00001)

static void *
new(Graph *g)
{
	uint min, max;
	ioff i, iv, *e, *ee, *etab;
	float n, ρ, ρ1, θ, x, y;
	Node *u, *ue;
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
		x = 0;
		y = 0;
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
	ρ1 = (max - min) / (Nodesz * Ptsz);
	n = 0;
	for(pp=ptab, r=rnodes, re=r+dylen(r); r<re; r++, pp++){
		if(!pp->fixed)
			continue;
		n++;
		/* FIXME */
		ρ = ρ1 + (n / (max - min)) * Nodesz * 192;
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
	float f, x, y, Δx, Δy;
	double dt, t, k, δx, δy, δ, rx, ry, Δr;
	ioff *e, *ee;
	RNode *u, *v;
	P *p, *p0, *p1;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = 1.0;
	p0 = d->ptab + i;
	p1 = d->ptab + dylen(d->ptab);
	Δ = nlaythreads;
	for(c=0;;c++){
		CLK0(clk);
		Δr = 0;
		for(u=rnodes, p=p0; p<p1; p+=Δ, u++){
			if((*stat & LFstop) != 0)
				return 0;
			x = u->pos[0];
			y = u->pos[1];
			Δx = Δy = 0.0f;
			/*
			for(v=rnodes, vp=pp; v<r1; v++, vp++){
				// movable nodes don't repulse each other
				if(r == v || vp->fixed == 1)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx += rx;
				Δy += ry;
			}
			*/
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+p->e, ee=e+p->nout; e<ee; e++){
				v = rnodes + *e;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				//rx = f * δx / δ;
				//ry = f * δy / δ;
				rx = f * δx;
				ry = f * δy;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=p->nin; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				//rx = f * δx / δ;
				//ry = f * δy / δ;
				rx = f * δx;
				ry = f * δy;
				Δx += rx;
				Δy += ry;
			}
			δx = t * Δx;
			δy = t * Δy;
			δ = Δ(δx, δy);
			x += δx / δ;
			y += δy / δ;
			u->pos[0] = x;
			u->pos[1] = y;
			if(δx != 0.0f){	/* FIXME */
				u->dir[0] = δx;
				u->dir[1] = δy;
			}
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr <= 1.0)
			break;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 1000000.0);
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
