#include "strpg.h"
#include "layout.h"
#include "graph.h"

/* FIXME: just set constant link length between fixed and movable? */
/* FIXME: chrona/other layout */

enum{
	W = 256,
	L = 256,
	Area = W * L,
};

typedef struct P P;
typedef struct D D;
struct P{
	Vertex xyz;
	Vertex *pos;
	Vertex *dir;
	Vertex *rot;
	ssize i;
	ssize e;
	int nin;
	int nout;
};
struct D{
	P *ptab;
	ssize *etab;
	ssize *ftab;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)

static void *
new(Graph *g)
{
	ssize nf, maxx, i, *e, *ee, *etab, *ftab;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *aux;

	ptab = nil;
	etab = ftab = nil;
	for(i=g->node0.next, nf=maxx=0; i>=0; i=u->next){
		u = g->nodes + i;
		p.i = i;
		p.pos = &u->pos;
		p.rot = &u->rot;
		p.dir = &u->dir;
		if((u->flags & (FNfixed|FNinitpos)) != 0){
			p.xyz.x = u->fixpos.x;
			p.xyz.y = u->fixpos.y;
			if((u->flags & FNfixed) != 0){
				if(maxx < p.xyz.x)
					maxx = p.xyz.x;
			}else{
				nf++;
				p.xyz.y = -128 + nrand(256);
			}
		}else{
			p.xyz.x = g->nnodes / 2;
			p.xyz.y = -32 + nrand(64);
		}
		*p.pos = p.xyz;
		u->layid = dylen(ptab);
		if((u->flags & FNfixed) == 0)
			dypush(ftab, u->layid);
		dypush(ptab, p);
	}
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		u = g->nodes + pp->i;
		if((u->flags & FNfixed) != 0){
			pp->i = -1;
			pp->xyz.x -= maxx / 2;
			pp->pos->x = pp->xyz.x;
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
	P *pp, *u, *v;
	int c;
	double dt;
	float t, k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ssize *e, *ee, *fp, *f0, *f1;
	D *d;

	d = arg;
	fp = d->ftab;
	pp = d->ptab;
	k = d->k;
	t = 1.0f;
	f0 = fp + i;
	f1 = fp + dylen(fp);
	if(f1 > fp + dylen(fp))
		f1 = fp + dylen(fp);
	for(c=0;;c++){
		Δr = 0;
		/* not the best way to do this, but skipping through the array
		 * approximately gives a view over the entire graph, not just
		 * a slice, for eg. termination */
		for(fp=f0; fp<f1; fp+=nlaythreads){
			if((*stat & LFstop) != 0)
				return 0;
			u = pp + *fp;
			x = u->xyz.x;
			y = u->xyz.y;
			Δx = Δy = 0;
			if(u->i < 0)
				continue;

			/* FIXME: removing this is not really a fix
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
			*/

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
			ROTATENODE(u->rot, u->dir, δx, δy);
			if(Δr < δ)
				Δr = δ;
		}
		if(Δr < 1.0)
			break;
		/* y = 1 - (x/Nrep)^4 */
		dt = c * (1.0 / 100000.0);
		t = 1.0 - dt * dt * dt * dt;
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
