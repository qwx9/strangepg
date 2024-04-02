#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Length = 150,
	Nrep = 5000,
};

#define	ΔT	(1.0 - 1.0 / Nrep)		/* eg. δ *= 0.999 at every step */

static void
init(Graph *)
{
}

static double
diff(Vertex v)
{
	return sqrt(v.x * v.x + v.y * v.y) + 0.00001;
}

static double
attraction(double d, double k)
{
    return d * d / k;
}

static double
repulsion(double d, double k)
{
    return k * k / d;
}

static void
compute(Graph *g)
{
	int i, j, n;
	double K, δ, R, Δ, ε, l;
	ssize ui, vi, *ep, *ee;
	Vertex *Fu, dv;
	Node *u, *from, *v;
	Edge *e;

	if((g->layout.f & LFarmed) == 0){
		/* initial random placement, but in same scale as springs */
		for(ui=g->node0.next; ui>=0; ui=u->next){
			u = g->nodes + ui;
			putnode(u, nrand(Length), nrand(Length));
		}
		g->layout.f |= LFarmed;
	}
	if(dylen(g->edges) < 1){
		warn("no links to hand\n");
		// FIXME: error exit
		return;
	}
	l = Length;
	K = ceil(sqrt(l * l / dylen(g->nodes)));
	/* arbitrary displacement minimum function */
	ε = ceil(sqrt(l * l / dylen(g->edges)));
	δ = 1.0;
	Fu = emalloc(dylen(g->nodes) * sizeof *Fu);
	for(n=0; n<Nrep; n++){
		memset(Fu, 0, dylen(g->nodes) * sizeof *Fu);
		for(ui=g->node0.next, i=0; ui>=0; i++, ui=u->next){
			u = g->nodes + ui;
			for(vi=g->node0.next; vi>=0; vi=v->next){
				v = g->nodes + vi;
				if(v == u)
					continue;
				dv = subpt2(u->q1.o, v->vrect.o);
				Δ = diff(dv);
				Fu[i] = addpt2(Fu[i], mulpt2(divpt2(dv, Δ), v->weight * repulsion(Δ, K)));
			}
		}
		for(ui=g->node0.next, i=0; ui>=0; i++, ui=u->next){
			u = g->nodes + ui;
			for(ep=u->in,ee=ep+dylen(u->in); ep!=nil && ep<ee; ep++){
				if((e = getedge(g, *ep)) == nil)
					continue;
				if((from = getnode(g, e->u >> 1)) == nil)
					panic("phase error -- missing incident node");
				dv = subpt2(from->vrect.o, u->vrect.o);
				Δ = diff(dv);
				dv = mulpt2(divpt2(dv, Δ), attraction(Δ, K));
				// FIXME: just a scaling issue
				Fu[i] = addpt2(Fu[i], mulpt2(dv, Δ / dylen(g->edges) / ((double)dylen(g->edges)/dylen(g->nodes))));
				j = from - g->nodes;
				Fu[j] = subpt2(Fu[j], mulpt2(dv, Δ / dylen(g->edges) / ((double)dylen(g->edges)/dylen(g->nodes))));
			}
		}
		for(ui=g->node0.next, i=0, R=0; ui>=0; i++, ui=u->next){
			u = g->nodes + ui;
			dv = Fu[i];
			Δ = diff(dv);
			dv = mulpt2(divpt2(dv, Δ), MIN(Δ, δ));
			u->vrect.o = addpt2(u->vrect.o, dv);
			if(R < Δ)
				R = Δ;
		}
		DPRINT(Debuglayout, "R %.2f ε %.2f K %.2f δ %.2f n %d", R, ε, K, δ, n);
		if(R < ε)
			break;
		if(δ > 0.0001)
			δ *= ΔT;
	}
	free(Fu);
}

static Layout ll = {
	.name = "force",
	.init = init,
	.compute = compute,
};

Layout *
regforce(void)
{
	return &ll;
}
