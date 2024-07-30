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
	ioff e;
	int nin;
	int nout;
};
struct D{
	P *ptab;
	ioff *etab;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)

static void *
new(Graph *g)
{
	int n;
	ioff i, iv, ie, *e, *ee, *etab;
	Node *u, *ue;
	RNode *r, *re;
	P p, *pp, *ptab;
	D *aux;

	ptab = nil;
	etab = nil;
	for(u=g->nodes, ue=u+dylen(u); u<ue; u++){
		p.e = dylen(etab);
		p.nout = dylen(u->out);
		p.nin = dylen(u->in);
		for(e=u->out, ee=e+p.nout; e<ee; e++){
			iv = g->edges[*e].v >> 1;
			dypush(etab, iv);
		}
		for(e=u->in, ee=e+p.nin; e<ee; e++){
			iv = g->edges[*e].u >> 1;
			dypush(etab, iv);
		}
		dypush(ptab, p);
	}
	n = Nodesz * Ptsz * dylen(ptab);
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		r->pos[0] = nrand(2 * n) - n;
		r->pos[1] = nrand(2 * n) - n;
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

/* skipping through nodes ensures that each thread works on a more
 * "global" part of the graph, otherwise the quality of the layout
 * is horrible; preparing the data in contiguous chunks in some
 * manner would be better but more complicated, as it stands,
 * performance doesn't really suffer because of this, possibly
 * because other threads would be accessing what was prefetched
 * anyway */
static int
compute(void *arg, volatile int *stat, int i)
{
	int c, Δ;
	ioff *e, *ee;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	double dt;
	RNode *r0, *r1, *r, *v;
	P *pp, *p0;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	k = d->k;
	t = 1.0f;
	p0 = d->ptab + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(rnodes);
	Δ = nlaythreads;
	for(c=0;;c++){
		CLK0(clk);
		Δr = 0;
		for(pp=p0, r=r0; r<r1; r+=Δ, pp+=Δ){
			if((*stat & LFstop) != 0)
				return 0;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+pp->e, ee=e+pp->nout; e<ee; e++){
				v = rnodes + *e;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=pp->nin; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
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
			r->pos[0] = x;
			r->pos[1] = y;
			if(δx != 0.0f){	/* FIXME */
				r->dir[0] = δx;
				r->dir[1] = δy;
			}
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 1.0f)
			return 0;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 4000.0);
		t = 1.0 - dt * dt * dt * dt;
		CLK1(clk);
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
