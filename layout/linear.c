#include "strpg.h"
#include "layout.h"
#include "threads.h"

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
	double x;
	double y;
	double Δx;
	double Δy;
	ssize i;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.00001)
#define	cool(t)	((t) > 0 ? (t) - 0.001 : 0)

static void *
new(Graph *g)
{
	ssize i;
	Node *u;
	P *ptab, p = {0};

	ptab = nil;
	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		p.i = i;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			u->vrect.o = u->fixed;
			p.x = u->fixed.x;
			p.y = u->fixed.y;
			if((u->flags & FNfixed) != 0)
				p.i = -1;
		}else{
			p.x = g->nnodes / 2;
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
	ssize i;
	double k, t, f, x, y, rx, ry, Δx, Δy, Δr, δx, δy, δ;
	P *ptab, *u, *v;
	Node *nu, *nv;
	Edge *e;
	Graph *g;

	if(idx > 0)	/* single thread */
		return 0;
	ptab = arg;
	g = ptab->g;
	k = 1 * sqrt((double)Area / dylen(g->nodes));
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
		for(i=g->edge0.next; i>=0; i=e->next){
			e = g->edges + i;
			nu = getnode(g, e->u >> 1);
			nv = getnode(g, e->v >> 1);
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
			nu->vrect.o.x = x;
			nu->vrect.o.y = y;
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
