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
	uchar fixed;
	float x;
	float y;
	float *nx;
	float *ny;
	ioff i;
	ioff e;
	int nin;
	int nout;
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
	int fx;
	uint min, max;
	ioff nf, nm, i, ie, *e, *ee, *etab;
	float n, r, r1, θ;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = nil;
	max = 0;
	min = ~0;
	for(i=0, ie=dylen(g->nodes), nf=nm=0; i<ie; i++){
		u = g->nodes + i;
		u->layid = dylen(ptab);
		/* FIXME: bug in data prod? */
		if(dylen(u->in) == 0 && dylen(u->out) == 0)
			continue;
		p.i = i;
		p.nx = &u->pos.x;
		p.ny = &u->pos.y;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			p.x = u->pos0.x;
			p.y = u->pos0.y;
			if((u->flags & FNfixed) != 0)
				nf++;
			else
				nm++;
			if(max < p.x)
				max = p.x;
			if(min > p.x)
				min = p.x;
		}
		dypush(ptab, p);
	}
	/* FIXME */
	//r1 = Vdefw / 1.5f - Nodesz / 2;
	r1 = (max - min) / 4;
	n = 0;
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		u = g->nodes + pp->i;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			fx = (u->flags & FNfixed);
			n++;
			r = r1;
			r -= (n / (max - min)) * Nodesz * 4;
			θ = n * Nodesz / r;
			if(!fx)
				r = 0.0f;
			*pp->nx = pp->x = r * cos(θ);
			*pp->ny = pp->y = r * -sin(θ);
			if(fx){
				pp->i = -1;
				continue;
			}
		}
		pp->e = dylen(etab);
		for(e=u->out, ee=e+dylen(e), i=0; e<ee; e++, i++){
			v = g->nodes + (g->edges[*e].v >> 1);
			assert(v != nil);
			dypush(etab, v->layid);
			assert(v->layid >= 0 && v->layid < dylen(ptab));
		}
		pp->nout = i;
		for(e=u->in, ee=e+dylen(e), i=0; e<ee; e++, i++){
			v = g->nodes + (g->edges[*e].u >> 1);
			assert(v != nil);
			dypush(etab, v->layid);
			assert(v->layid >= 0 && v->layid < dylen(ptab));
		}
		pp->nin = i;
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
	P *pp, *p0, *p1, *u, *v;
	int c;
	double dt;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ioff *e, *ee;
	D *d;

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
		/* not the best way to do this, but skipping through the array
		 * approximately gives a view over the entire graph, not just
		 * a slice, for eg. termination */
		for(u=p0; u<p1; u+=nlaythreads){
			if((*stat & LFstop) != 0)
				return 0;
			x = u->x;
			y = u->y;
			Δx = Δy = 0;
			if(u->i < 0)
				continue;
			/* FIXME */
			for(v=pp; v<pp+dylen(pp); v++){
				if(u == v)
					continue;
				δx = x - v->x;
				δy = y - v->y;
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			e = d->etab + u->e;
			for(ee=e+u->nout; e<ee; e++){
				v = pp + *e;
				δx = x - v->x;
				δy = y - v->y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=u->nin; e<ee; e++){
				v = pp + *e;
				δx = v->x - x;
				δy = v->y - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx += rx;
				Δy += ry;
			}
			δx = Δx;
			δy = Δy;
			δ = Δ(δx, δy);
			x += t * δx / δ;
			y += t * δy / δ;
			*u->nx = x;
			*u->ny = y;
			u->x = x;
			u->y = y;
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 1.0)
			break;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 100000.0);
		t = 1.0 - dt * dt * dt * dt;
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
