#include "strpg.h"
#include "layout.h"

/* FIXME: just set constant link length between fixed and movable? */
/* FIXME: chrona/other layout */

enum{
	//Nrep = 100,
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
	double *nx;
	double *ny;
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
#define	cool(t)	((t) > 0 ? (t) - 0.0001 : 0)

static void *
new(Graph *g)
{
	ssize nf, maxx, i, *e, *ee, *etab;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = nil;
	for(i=g->node0.next, nf=maxx=0; i>=0; i=u->next){
		u = g->nodes + i;
		u->layid = dylen(ptab);
		p.i = i;
		p.nx = &u->vrect.o.x;
		p.ny = &u->vrect.o.y;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			p.x = u->fixed.x;
			p.y = u->fixed.y;
			if((u->flags & FNfixed) != 0){
				nf++;
				if(maxx < p.x)
					maxx = p.x;
			}else{
				p.y = -128 + nrand(256);
			}
		}else{
			p.x = g->nnodes / 2;
			p.y = -32 + nrand(64);
		}
		*p.nx = p.x;
		*p.ny = p.y;
		u->vrect.v = ZV;
		dypush(ptab, p);
	}
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		u = g->nodes + pp->i;
		if((u->flags & FNfixed) != 0){
			pp->i = -1;
			pp->x -= maxx / 2;
			*pp->nx = pp->x;
			continue;
		}
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

/* FIXME: compensate for the fact that movable nodes move towards fixed ones,
 * rather than both toward each other */
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
			if(u->i < 0)
				continue;
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
		t = cool(t);
	}
	return 0;
}

static Shitkicker ll = {
	.name = "pline",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
regpline(void)
{
	return &ll;
}
