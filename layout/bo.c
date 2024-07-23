#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	Length = 512,
};

typedef struct P P;
typedef struct D D;
struct P{
	Vertex xyz;
	Vertex *pos;
	Vertex *dir;
	ioff i;
	ioff e;
	int nin;
	int nout;
};
struct D{
	P *ptab;
	ioff *etab;
	float k;
	int nth;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)

static void *
new(Graph *g)
{
	uint min, max;
	ioff i, ie, n, *e, *ee, *etab;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = nil;
	min = -1U;
	max = 0;
	for(i=0, ie=dylen(g->nodes), n=0; i<ie; i++, n++){
		u = g->nodes + i;
		u->layid = dylen(ptab);
		p.i = i;
		p.pos = &u->pos;
		p.dir = &u->dir;
		dypush(ptab, p);
		if(u->pos0.x < min)
			min = u->pos0.x;
		else if(u->pos0.x > max)
			max = u->pos0.x;
	}
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		u = g->nodes + pp->i;
		pp->xyz = u->pos0;
		pp->xyz.x -= min + (max - min) / 2;
		pp->xyz.x *= Nodesz + 32;
		pp->xyz.y = -32 + nrand(32);
		*pp->pos = pp->xyz;
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
	aux->k = 1 * sqrt((float)(Length * Length) / dylen(ptab));
	aux->nth = nlaythreads;
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
	int c, nth;
	P *pp, *p0, *p1, *u, *v;
	double dt;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ioff *e, *ee;
	D *d;

	d = arg;
	pp = d->ptab;
	k = d->k;
	nth = d->nth;
	t = 1.0f;
	p0 = pp + i;
	p1 = pp + dylen(pp);
	if(p1 > pp + dylen(pp))
		p1 = pp + dylen(pp);
	for(c=0;;c++){
		Δr = 0;
		for(u=p0; u<p1; u+=nth){
			if((*stat & LFstop) != 0)
				return 0;
			x = u->xyz.x;
			y = u->xyz.y;
			Δx = Δy = 0;
			for(v=pp; v<pp+dylen(pp); v++){
				if(u == v)
					continue;
				δx = x - v->xyz.x;
				δy = y - v->xyz.y;
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
				δx = x - v->xyz.x;
				δy = y - v->xyz.y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=u->nin; e<ee; e++){
				v = pp + *e;
				δx = v->xyz.x - x;
				δy = v->xyz.y - y;
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
			u->xyz.x = x;
			u->xyz.y = y;
			*u->pos = u->xyz;
			SETDIR(*u->dir, δx, δy);
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 1.0f)
			return 0;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 4000.0);
		t = 1.0 - dt * dt * dt * dt;
	}
}

static Shitkicker ll = {
	.name = "bo",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
regbo(void)
{
	return &ll;
}
