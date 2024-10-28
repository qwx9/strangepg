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

typedef struct P P;
struct P{
	float w;
	float x;
	float y;
	float Δx;
	float Δy;
};
typedef struct D D;
struct D{
	P *ptab;
	Node *nodes;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.00001f)
#define	cool(t)	((t) * ((Nrep - 1.0f) / Nrep))

static void *
new(Graph *g)
{
	ioff i;
	P *ptab, p = {0};
	RNode *r;
	Node *n, *ne;
	D *aux;

	ptab = nil;
	for(i=0, n=g->nodes, ne=n+dylen(g->nodes); n<ne; n++, i++){
		p.x = (float)(W / 2 - nrand(W)) / (W / 2);
		p.y = (float)(L / 2 - nrand(L)) / (L / 2);
		r = rnodes + i;
		r->pos[0] = p.x;
		r->pos[1] = p.y;
		p.w = r->len;
		dypush(ptab, p);
	}
	aux = emalloc(sizeof *aux);
	aux->ptab = ptab;
	aux->nodes = g->nodes;
	return aux;
}

static void
cleanup(void *p)
{
	D *aux;

	aux = p;
	dyfree(aux->ptab);
	free(aux);
}

static int
compute(void *arg, volatile int *stat, int idx)
{
	ioff *ep, *ee;
	float k, t, f, x, y, w, uw, vw, rx, ry, Δx, Δy, Δr, δx, δy, δ;
	P *ptab, *u, *v;
	RNode *r, *re;
	Node *n, *ne;
	D *d;

	if(idx > 0)	/* single thread */
		return 0;
	d = arg;
	ptab = d->ptab;
	k = C * sqrtf((float)Area / dylen(ptab));
	t = 1.0f;
	for(;;){
		Δr = 0;
		for(n=d->nodes, u=ptab; u<ptab+dylen(ptab); u++, n++){
			if((*stat & LFstop) != 0)
				return 0;
			Δx = Δy = 0.0f;
			uw = u->w;
			for(v=ptab; v<ptab+dylen(ptab); v++){
				if(u == v)
					continue;
				δx = u->x - v->x;
				δy = u->y - v->y;
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				vw = v->w;
				w = (uw + vw);
				w = C * MIN(uw, vw);
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			u->Δx = Δx;
			u->Δy = Δy;
		}
		for(u=ptab, n=d->nodes, ne=n+dylen(n); n<ne; n++, u++){
			uw = u->w;
			for(ep=n->out, ee=ep+dylen(ep); ep<ee; ep++){
				v = ptab + (*ep >> 2);
				if(u == v)
					continue;
				δx = u->x - v->x;
				δy = u->y - v->y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				vw = v->w;
				if(uw < vw)
					w = uw / vw;
				else
					w = vw / uw;
				w *= C;
				rx = w * f * δx / δ;
				ry = w * f * δy / δ;
				u->Δx -= rx;
				u->Δy -= ry;
				v->Δx += rx;
				v->Δy += ry;
			}
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
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Target *
regfr(void)
{
	return &ll;
}
