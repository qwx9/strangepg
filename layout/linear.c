#include "strpg.h"
#include "layout.h"
#include "graph.h"
#include "drw.h"

enum{
	W = 256,
	L = 256,
	Area = W * L,
};

typedef struct P P;
typedef struct D D;
struct P{
	ioff e;
	int nin;
	int nout;
	char fixed;
};
struct D{
	P *ptab;
	ioff *etab;
	ioff *ftab;
	float k;
};

#define Fa(x, k)	((x) * (x) * (x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)

static void *
new(Graph *g)
{
	ioff i, iv, *e, *ee, *etab, *ftab;
	float x, y, min, max;
	Node *u, *ue;
	RNode *r, *re;
	P p, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = ftab = nil;
	max = 0;
	min = 3.40282347e+38f;
	for(i=0, r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++, i++){
		p.e = dylen(etab);
		p.nout = dylen(u->out);
		p.nin = dylen(u->in);
		p.fixed = 0;
		for(e=u->out, ee=e+p.nout; e<ee; e++){
			iv = g->edges[*e].v >> 1;
			dypush(etab, iv);
		}
		for(e=u->in, ee=e+p.nin; e<ee; e++){
			iv = g->edges[*e].u >> 1;
			dypush(etab, iv);
		}
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			x = u->pos0.x * Nodesz;
			y = u->pos0.y * Nodesz * Ptsz;
			if((u->flags & FNfixed) != 0)
				p.fixed = 1;
			else
				p.fixed = 2;
		}else
			x = y = 0.0f;
		if(max < x)
			max = x;
		if(min > x)
			min = x;
		r->pos[0] = x;
		r->pos[1] = y;
		if(p.fixed != 1)
			dypush(ftab, i);
		dypush(ptab, p);
	}
	for(pp=ptab, r=rnodes, re=r+dylen(r); r<re; r++, pp++){
		if(pp->fixed)
			r->pos[0] -= (max - min) / 2.0f;
	}
	aux = emalloc(sizeof *aux);
	aux->ptab = ptab;
	aux->etab = etab;
	aux->ftab = ftab;
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
	dyfree(aux->ftab);
	free(aux);
}

/* FIXME: compensate for the fact that movable nodes move towards fixed ones,
 * rather than both toward each other */
static int
compute(void *arg, volatile int *stat, int i)
{
	int c, Δ;
	float f, x, y, Δx, Δy;
	double dt, t, k, δx, δy, δ, rx, ry, Δr;
	ioff n, *e, *ee, *fp, *f0, *f1;
	RNode *r, *v;
	P *pp, *u;
	D *d;
	Clk clk = {.lab = "layiter"};

	d = arg;
	fp = d->ftab;
	pp = d->ptab;
	k = d->k;
	t = 1.0f;
	f0 = fp + i;
	f1 = fp + dylen(fp);
	if(f1 > fp + dylen(fp))
		f1 = fp + dylen(fp);
	Δ = nlaythreads;
	for(c=0;;c++){
		CLK0(clk);
		Δr = 0;
		for(fp=f0; fp<f1; fp+=Δ){
			if((*stat & LFstop) != 0)
				return 0;
			n = *fp;
			u = pp + n;
			r = rnodes + n;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0.0f;
			/*
			for(v=rnodes, vp=pp; v<rnodes+dylen(rnodes); v++, vp++){
				// movable nodes don't repulse each other
				if(r == v || vp->fixed == 1)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				rx = f * δx / δ;
				ry = f * δy / δ;
				Δx += rx;
				Δy += ry;
			}
			*/
			if((*stat & LFstop) != 0)
				return 0;
			for(e=d->etab+u->e, ee=e+u->nout; e<ee; e++){
				v = rnodes + *e;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				//rx = f * δx / δ;
				//ry = f * δy / δ;
				rx = f * δx;
				ry = f * δy;
				Δx -= rx;
				Δy -= ry;
			}
			for(ee+=u->nin; e<ee; e++){
				v = rnodes + *e;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				//rx = f * δx / δ;
				//ry = f * δy / δ;
				rx = f * δx;
				ry = f * δy;
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
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr <= 1.0)
			break;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 1000000.0);
		t = 1.0 - dt * dt * dt * dt;
		CLK1(clk);
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
