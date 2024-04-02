#include "strpg.h"
#include "layout.h"

/* fruchterman and reingold 91, with small modifications */

enum{
	Nrep = 5000,
	W = 256,
	L = 256,
	Area = W * L,
};

typedef struct P P;
struct P{
	double x;
	double y;
	double Δx;
	double Δy;
	ssize i;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.00001)
#define	cool(t)	((t) * ((Nrep - 1.0) / Nrep))

static void
init(Graph *)
{
}

static P *
scan(Graph *g, int new)
{
	ssize i;
	Node *u;
	Layouting *l;
	P *ρ, p = {0};

	ρ = nil;
	l = &g->layout;
	free(l->aux);
	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		p.i = i;
		if((u->flags & FNfixed) != 0){
			u->vrect.o = u->fixed;
			p.x = u->fixed.x;
			p.y = u->fixed.y;
			p.i = -1;
		}else if(new){
			p.x = -W/8 + nrand(W/4);
			p.y = -L/8 + nrand(L/4);
		}else{
			p.x = u->vrect.o.x;
			p.y = u->vrect.o.y;
		}
		u->layid = dylen(ρ);
		dypush(ρ, p);
	}
	l->aux = dyhdr(ρ);	/* ouch */
	l->f |= LFarmed;
	return ρ;
}

// FIXME: fixed point? fast fixed point square root?
// FIXME: fp errors here and in rend; still a double free or so

static void
compute(Graph *g)
{
	ssize i;
	double k, t, f, x, y, rx, ry, Δx, Δy, Δr, δx, δy, δ;
	P *ρ, *u, *v;
	Node *nu, *nv;
	Edge *e;
	Layouting *l;

	l = &g->layout;
	ρ = scan(g, (l->f & LFarmed) == 0);
	if(dylen(g->edges) < 1){
		warn("no links to hand\n");
		return;
	}
	k = 1 * ceil(sqrt((double)Area / dylen(g->nodes)));
	t = 1.0;
	for(;;){
		//sleep(10);
		Δr = 0;
		for(u=ρ; u<ρ+dylen(ρ); u++){
			Δx = Δy = 0;
			if(u->i < 0)
				continue;
			for(v=ρ; v<ρ+dylen(ρ); v++){
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
			u = ρ + nu->layid;
			v = ρ + nv->layid;
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
		for(u=ρ; u<ρ+dylen(ρ); u++){
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
			nu->vrect.o.x = u->x;
			nu->vrect.o.y = u->y;
			if(Δr < δ)
				Δr = δ;
		}
		t = cool(t);
		if(Δr < 1)
			break;
	}
	free(l->aux);
	l->aux = nil;
}

static Layout ll = {
	.name = "linear",
	.init = init,
	.compute = compute,
};

Layout *
reglinear(void)
{
	return &ll;
}
