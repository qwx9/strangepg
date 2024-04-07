#include "strpg.h"
#include "layout.h"
#include "threads.h"

/* lilu dallas moolti modified fruchterman and reingold */

enum{
	//Nrep = 100,
	W = 256,
	L = 256,
	Area = W * L,
	Nthread = 4,
};

typedef struct P P;
typedef struct D D;
struct P{
	double x;
	double y;
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
	double t;
	double k;
	double Δr;
	Thread *th[Nthread];
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	Δ(x, y)	(sqrt((x) * (x) + (y) * (y)) + 0.0001)
//#define	cool(t)	((t) * ((Nrep - 1.0) / Nrep))
#define	cool(t)	((t) > 0 ? (t) - 0.001 : 0)

static void
init(Graph *)
{
}

static D *
scan(Graph *g, int new)
{
	ssize i, *e, *ee, *etab;
	Node *u, *v;
	P p = {0}, *ptab, *pp;
	D *data;

	ptab = nil;
	etab = nil;
	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		u->layid = dylen(ptab);
		p.i = i;
		p.nx = &u->vrect.o.x;
		p.ny = &u->vrect.o.y;
		if(new){
			p.x = -W/4 + nrand(W/2);
			p.y = -L/4 + nrand(L/2);
		}
		*p.nx = p.x;
		*p.ny = p.y;
		u->vrect.v = ZV;
		dypush(ptab, p);
	}
	for(pp=ptab; pp<ptab+dylen(ptab); pp++){
		u = g->nodes + pp->i;
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
	data = emalloc(sizeof *data);
	data->ptab = ptab;
	data->etab = etab;
	//data->k = 1 * ceil(sqrt((double)Area / dylen(ptab)));
	data->k = 1 * sqrt((double)Area / dylen(ptab));
	data->t = 1.0;
	data->Δr = 1.0;
	if(threadstore(data) == nil)
		sysfatal("threadstore: %s", error());
	g->layout.f |= LFarmed;
	return data;
}

static void
werks(void *arg)
{
	P *pp, *p0, *p1, *u, *v;
	vlong τ, τ1;
	double k, f, x, y, Δx, Δy, δx, δy, δ, rx, ry, Δr;
	ssize i, *e, *ee;
	D *d;

	i = (intptr)arg;
	if((d = threadstore(nil)) == nil){
		warn("werker: no data\n");
		return;
	}
	pp = d->ptab;
	k = d->k;
	p0 = pp + i;
	p1 = pp + dylen(pp);
	if(p1 > pp + dylen(pp))
		p1 = pp + dylen(pp);
	τ = μsec();
	for(;;){
		Δr = 0;
		/* not the best way to do this, but skipping through the array
		 * approximately gives a view over the entire graph, not just
		 * a slice, for eg. termination */
		for(u=p0; u<p1; u+=nelem(d->th)){
			x = u->x;
			y = u->y;
			Δx = Δy = 0;
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
			x += d->t * δx / δ;
			y += d->t * δy / δ;
			*u->nx = x;
			*u->ny = y;
			u->x = x;
			u->y = y;
			if(Δr < δ)
				Δr = δ;
			yield();
		}
		if(Δr > d->Δr)
			d->Δr = Δr;
		if(Δr < 1.0)
			break;
		d->t = cool(d->t);
		τ1 = μsec();
		//warn("[%zd] %lld μs\n", i, τ1 - τ);
		τ = τ1;
	}
	d->th[i] = nil;
}

static void
cleanup(void *pdata)
{
	D *d;
	Thread **th, **te;

	warn("cleanup %#p\n", pdata);
	d = pdata;
	for(th=d->th, te=th+nelem(d->th); th<te; th++)
		if(*th != nil)
			killthread(*th);
	dyfree(d->ptab);
	dyfree(d->etab);
	free(d);
}

static void
compute(Graph *g)
{
	ssize i;
	Layouting *l;
	D *d;

	l = &g->layout;
	if((d = scan(g, (l->f & LFarmed) == 0)) == nil)
		sysfatal("scan: %s", error());
	if(dylen(g->edges) < 1){
		warn("no links to hand\n");
		return;
	}
	for(i=0; i<nelem(d->th); i++){
		if((d->th[i] = newthread(werks, nil, (void *)i, d, "swine", mainstacksize)) == nil)
			sysfatal("newthread: %s", error());
	}
	for(;;){
		sleep(100);
		//d->t = cool(d->t);
		//yield();
	}
}

static Layout ll = {
	.name = "pfr",
	.init = init,
	.compute = compute,
	.cleanup = cleanup,
};

Layout *
regpfr(void)
{
	return &ll;
}
