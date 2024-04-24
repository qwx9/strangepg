#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	//Nrep = 100,
	Length = 200,
};

typedef struct P P;
typedef struct D D;
struct P{
	float x;
	float y;
	float *nx;
	float *ny;
	ssize i;
	ssize e;
	int nin;
	int nout;
};
struct D{
	P *ptab;
	ssize *etab;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)
#define	cool(t)	((t) - 0.0001f > 0.0001f ? (t) - 0.0001f : (t))
//#define	cool(t,n)	((t) * ((n - 1.0) / n))

static void *
new(Graph *g)
{
	float n;
	ssize i, *e, *ee, *etab;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = nil;
	for(i=g->node0.next, n=0.0; i>=0; i=u->next){
		u = g->nodes + i;
		u->layid = dylen(ptab);
		p.i = i;
		p.nx = &u->pos.x;
		p.ny = &u->pos.y;
		dypush(ptab, p);
		n += 1.0;
	}
	n = Nodesz * log(n);
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		*pp->nx = pp->x = -n + nrand(2*n);
		*pp->ny = pp->y = -n + nrand(2*n);
		u = g->nodes + pp->i;
		pp->e = dylen(etab);
		for(e=u->out, ee=e+dylen(e), i=0; e<ee; e++, i++){
			v = getnode(g, g->edges[*e].v >> 1);
			assert(v != nil);
			dypush(etab, v->layid);
			assert(v->layid >= 0 && v->layid < dylen(ptab));
		}
		pp->nout = i;
		for(e=u->in, ee=e+dylen(e), i=0; e<ee; e++, i++){
			v = getnode(g, g->edges[*e].u >> 1);
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

/* what takes the most time (O(n²)) is the all-to-all node repulsion loop,
 * the rest is fast, but to avoid waiting, just do all three steps in each
 * worker */
static int
compute(void *arg, volatile int *stat, int i)
{
	P *pp, *p0, *p1, *u, *v;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ssize *e, *ee;
	D *d;

	d = arg;
	pp = d->ptab;
	k = d->k;
	t = 1.0;
	p0 = pp + i;
	p1 = pp + dylen(pp);
	if(p1 > pp + dylen(pp))
		p1 = pp + dylen(pp);
	for(;;){
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
			return 0;
		if(t == 0.0){
			warn("fuck %f\n", Δr);
			return 0;
		}
		t = cool(t);
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
