#include "strpg.h"
#include "layout.h"
#include "graph.h"
#include "drw.h"

/* fruchterman and reingold 91, with small modifications */

enum{
	Nrep = 5000,
	W = 256,
	L = 256,
	Area = W * L,
};

typedef struct P P;
struct P{
	Graph *g;
	float x;
	float y;
	float Δx;
	float Δy;
	ioff i;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.00001)
#define	cool(t)	((t) * ((Nrep - 1.0) / Nrep))
//#define	cool(t)	((t) - 0.001f > 0.0f ? (t) - 0.001f : 0.0f)

static void *
new(Graph *g)
{
	ioff i, ie;
	Node *u;
	P *ptab, p = {0};
	RNode *r;

	ptab = nil;
	for(i=0, ie=dylen(g->nodes); i<ie; i++){
		u = g->nodes + i;
		p.i = i;
		p.x = -W/4 + nrand(W/2);
		p.y = -L/4 + nrand(L/2);
		dypush(ptab, p);
		r = rnodes + i;
		r->pos[0] = p.x;
		r->pos[1] = p.y;
	}
	ptab->g = g;
	return ptab;
}

static void
cleanup(void *p)
{
	dyfree(p);
	USED(p);	/* shut compiler up */
}

static int
compute(void *arg, volatile int *stat, int idx)
{
	ioff i, ie;
	float k, t, f, x, y, rx, ry, Δx, Δy, Δr, δx, δy, δ;
	P *ptab, *u, *v;
	Node *nu, *nv;
	RNode *r, *r1, *r2, *re;
	Edge *e;
	Graph *g;

	if(idx > 0)	/* single thread */
		return 0;
	ptab = arg;
	g = ptab->g;
	k = 1 * sqrt((float)Area / dylen(ptab));
	t = 1.0;
	for(;;){
		Δr = 0;
		for(u=ptab; u<ptab+dylen(ptab); u++){
			if((*stat & LFstop) != 0)
				return 0;
			Δx = Δy = 0;
			for(v=ptab; v<ptab+dylen(ptab); v++){
				if(u == v)
					continue;
				δx = u->x - v->x;
				δy = u->y - v->y;
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
			}
			u->Δx = Δx;
			u->Δy = Δy;
		}
		for(i=0, ie=dylen(g->edges); i<ie; i++){
			e = g->edges + i;
			r1 = rnodes + (e->u >> 1);
			r2 = rnodes + (e->v >> 1);
			if(r1 == r2)
				continue;
			δx = r1->pos[0] - r2->pos[0];
			δy = r1->pos[1] - r2->pos[1];
			δ = Δ(δx, δy);
			f = Fa(δ, k);
			rx = f * δx / δ;
			ry = f * δy / δ;
			u = ptab + (e->u >> 1);
			v = ptab + (e->v >> 1);
			u->Δx -= rx;
			u->Δy -= ry;
			v->Δx += rx;
			v->Δy += ry;
		}
		if((*stat & LFstop) != 0)
			break;
		for(u=ptab, r=rnodes, re=r+dylen(r); r<re; r++, u++){
			δx = u->Δx;
			δy = u->Δy;
			δ = Δ(δx, δy);
			x = u->x + δx / δ * t;
			y = u->y + δy / δ * t;
			r->pos[0] = u->x = x;
			r->pos[1] = u->y = y;
			if(δx != 0.0f){	/* FIXME */
				r->dir[0] = δx;
				r->dir[1] = δy;
			}
			if(Δr < δ)
				Δr = δ;
		}
		t = cool(t);
		if(Δr < 1)
			break;
	}
	return 0;
}

static Shitkicker ll = {
	.name = "fr",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
regfr(void)
{
	return &ll;
}
