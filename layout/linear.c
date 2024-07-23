#include "strpg.h"
#include "layout.h"
#include "graph.h"
#include "drw.h"

/* FIXME: NOTE: returning stuff by copy to feed graphs etc means pointers
 * will be invalidated as soon as those arrays are resized, so just malloc
 * each or keep indices... */

/* fruchterman and reingold 91, with small modifications */

enum{
	Nrep = 10000,
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
#define	cool(t)	((t) > 0 ? (t) - 0.001f : 0.0f)

static void *
new(Graph *g)
{
	ioff i, ie;
	Node *u;
	P *ptab, p = {0};

	ptab = nil;
	for(i=0, ie=dylen(g->nodes); i<ie; i++){
		u = g->nodes + i;
		p.i = i;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			p.x = u->pos.x = u->pos0.x;
			p.y = u->pos.y = u->pos0.y;
			if((u->flags & FNfixed) != 0)
				p.i = -1;
		}else{
			p.x = dylen(g->nodes) / 2;
			p.y = -32 + nrand(64);
		}
		u->layid = dylen(ptab);
		dypush(ptab, p);
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
	Edge *e;
	Graph *g;

	if(idx > 0)	/* single thread */
		return 0;
	ptab = arg;
	g = ptab->g;
	k = 1 * sqrt((float)Area / dylen(g->nodes));
	t = 1.0;
	for(;;){
		Δr = 0;
		for(u=ptab; u<ptab+dylen(ptab); u++){
			if((*stat & LFstop) != 0)
				return 0;
			Δx = Δy = 0;
			if(u->i < 0)
				continue;
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
			nu = g->nodes + (e->u >> 1);
			nv = g->nodes + (e->v >> 1);
			if(nu == nv)
				continue;
			assert(nu != nil && nv != nil);
			u = ptab + nu->layid;
			v = ptab + nv->layid;
			δx = u->x - v->x;
			δy = u->y - v->y;
			δ = Δ(δx, δy);
			f = Fa(δ, k);
			rx = f * δx / δ;
			ry = f * δy / δ;
			u->Δx -= rx;
			u->Δy -= ry;
			v->Δx += rx;
			v->Δy += ry;
		}
		if((*stat & LFstop) != 0)
			break;
		for(u=ptab; u<ptab+dylen(ptab); u++){
			if(u->i < 0)
				continue;
			δx = u->Δx;
			δy = u->Δy;
			δ = Δ(δx, δy);
			x = u->x + δx / δ * t;
			y = u->y + δy / δ * t;
			u->x = x;
			u->y = y;
			nu = g->nodes + u->i;
			nu->pos.x = x;
			nu->pos.y = y;
			SETDIR(nu->dir, δx, δy);
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
	.name = "linear",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Shitkicker *
reglinear(void)
{
	return &ll;
}
