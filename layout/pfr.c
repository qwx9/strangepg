#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"

enum{
	Length = 256,
};

typedef struct P P;
typedef struct D D;
struct P{
	Vertex xyz;
	Vertex *pos;
	Vertex *dir;
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
		p.pos = &u->pos;
		p.dir = &u->dir;
		dypush(ptab, p);
		n += 1.0;
	}
	n = Nodesz * log(n);
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		pp->xyz.x = -n + nrand(2*n);
		pp->xyz.y = -n + nrand(2*n);
		*pp->pos = pp->xyz;
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
	int c;
	P *pp, *p0, *p1, *u, *v;
	double dt;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ssize *e, *ee;
	D *d;

	d = arg;
	pp = d->ptab;
	k = d->k;
	t = 1.0f;
	p0 = pp + i;
	p1 = pp + dylen(pp);
	if(p1 > pp + dylen(pp))
		p1 = pp + dylen(pp);
	for(c=0;;c++){
		Δr = 0;
		/* not the best way to do this, but skipping through the array
		 * approximately gives a view over the entire graph, not just
		 * a slice, for eg. termination */
		for(u=p0; u<p1; u+=nlaythreads){
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
