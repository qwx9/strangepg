#include "strpg.h"
#include "layout.h"
#include "graph.h"
#include "drw.h"

/* fruchterman and reingold 91, with small modifications */

enum{
	Nrep = 1000,
	W = 1024,
	L = 1024,
	Area = W * L,
};
#define	C	((float)Nodesz / (Maxsz - Minsz) * 0.7f)

typedef struct Aux Aux;
typedef struct Δpos Δpos;
struct Δpos{
	float x;
	float y;
};
struct Aux{
	Δpos *Δtab;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.00001f)
#define	cool(t)	((t) * ((Nrep - 1.0f) / Nrep))

static void *
init(void)
{
	Aux *aux;

	aux = emalloc(sizeof *aux);
	aux->k = C * sqrtf((float)Area / dylen(rnodes));
	aux->Δtab = emalloc(dylen(rnodes) * sizeof *aux->Δtab);
	return aux;
}

static void *
new(void)
{
	ioff n;
	double z;
	RNode *r, *re;
	Node *u;

	n = dylen(rnodes);
	for(u=nodes, r=rnodes, re=r+n; r<re; r++, u++){
		r->pos[0] = (float)(W / 2 - xfrand() * W) / (W / 2);
		r->pos[1] = (float)(L / 2 - xfrand() * L) / (L / 2);
		z = (double)(n - (r - rnodes)) / n;
		r->pos[2] = (drawing.flags & DFnodepth) == 0
			? 0.8 * (0.5 - z)
			: 0.00001 * z;
	}
	return init();
}

static void
cleanup(void *aux)
{
	free(aux);
}

static int
compute(void *arg, volatile int *stat, int idx)
{
	ioff i, *e, *ee;
	float k, t, f, x, y, w, uw, vw, Δx, Δy, Δr, δx, δy, δ;
	RNode *u, *v, *re;
	Node *n;
	Δpos *Δtab, *Δu, *Δv;
	Aux *aux;

	if(idx > 0)	/* single thread */
		return 0;
	aux = arg;
	re = rnodes + dylen(nodes);
	Δtab = aux->Δtab;
	k = aux->k;
	t = 1.0f;
	for(;;){
		Δr = 0;
		for(Δu=Δtab, u=rnodes; u<re; u++, Δu++){
			if((*stat & LFstop) != 0)
				return 0;
			uw = u->len;
			x = u->pos[0];
			y = u->pos[1];
			Δx = Δy = 0.0f;
			for(v=rnodes; v<re; v++){
				if(u == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				vw = v->len;
				w = C * MIN(uw, vw);
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			Δu->x = Δx;
			Δu->y = Δy;
		}
		for(Δu=Δtab, n=nodes, u=rnodes; u<re; u++, n++, Δu++){
			x = u->pos[0];
			y = u->pos[1];
			uw = u->len;
			for(e=edges+n->eoff, ee=e+n->nedges-n->nin; e<ee; e++){
				i = *e >> 2;
				v = rnodes + i;
				Δv = Δtab + i;
				if(u == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				vw = v->len;
				if(uw < vw)
					w = uw / vw;
				else
					w = vw / uw;
				w *= C;
				Δx = w * f * δx / δ;
				Δy = w * f * δy / δ;
				Δu->x -= Δx;
				Δu->y -= Δy;
				Δv->x += Δx;
				Δv->y += Δy;
			}
		}
		if((*stat & LFstop) != 0)
			break;
		for(Δu=Δtab, u=rnodes; u<re; u++, Δu++){
			δx = Δu->x;
			δy = Δu->y;
			δ = Δ(δx, δy);
			x = u->pos[0] + δx / δ * t;
			y = u->pos[1] + δy / δ * t;
			u->pos[0] = x;
			u->pos[1] = y;
			δ *= t;
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 0.1f)
			break;
		t = cool(t);
	}
	return 0;
}

static Target ll = {
	.name = "fr",
	.init = init,
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Target *
regfr(void)
{
	return &ll;
}
