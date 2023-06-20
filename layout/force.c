#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Length = 150,
	Nrep = 5000,
};

#define	ΔT	(1.0 - 1.0 / Nrep)		/* eg. δ *= 0.999 at every step */

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
	int i, j, n, x, y;
	double K, δ, R, Δ, ε, l;
	usize *ep, *ee;
	Vertex *Fu, dv;
	Node *u, *from, *v, *ne;
	Edge *e;

	//sleep(1000);
	if(dylen(g->edges) < 2){
		warn("no links to hand\n");
		// FIXME: error exit
		return;
	}
	l = Length;
	/* initial random placement, but in same scale as springs */
	for(u=g->nodes, ne=u+dylen(g->nodes); u<ne; u++){
		if(u->erased)
			continue;
		x = y = nrand(l);
		putnode(u, x, y);
	}
	K = ceil(sqrt(l * l / g->len));
	/* arbitrary displacement minimum function */
	ε = ceil(sqrt(l * l / dylen(g->edges)));
	δ = 1.0;
	Fu = emalloc(g->len * sizeof *Fu);
	u = g->nodes;
	ne = u + dylen(g->nodes);
	for(n=0; n<Nrep; n++){
		memset(Fu, 0, g->len * sizeof *Fu);
		for(u=g->nodes, i=0; u<ne; i++, u++){
			if(u->erased)
				continue;
			for(v=g->nodes; v<ne; v++){
				if(v == u)
					continue;
				dv = subpt2(u->q1.o, v->vrect.o);
				Δ = diff(dv);
				Fu[i] = addpt2(Fu[i], mulpt2(divpt2(dv, Δ), v->w * repulsion(Δ, K)));
			}
		}
		for(u=g->nodes, i=0; u<ne; i++, u++){
			if(u->erased)
				continue;
			for(ep=u->in,ee=ep+dylen(u->in); ep!=nil && ep<ee; ep++){
				e = g->edges + *ep;
				if((from = e2n(g, e->from)) == nil)
					panic("phase error -- missing incident node");
				dv = subpt2(from->vrect.o, u->vrect.o);
				Δ = diff(dv);
				dv = mulpt2(divpt2(dv, Δ), attraction(Δ, K));
				Fu[i] = addpt2(Fu[i], mulpt2(dv, e->w));
				j = from - g->nodes;
				if(e->w < 1){
					assert(e->w > 0.0);
					Fu[j] = subpt2(Fu[j], dv);
				}else
					Fu[j] = subpt2(Fu[j], divpt2(dv, e->w));
			}
		}
		for(u=g->nodes, R=0, i=0; u<ne; i++, u++){
			if(u->erased)
				continue;
			dv = Fu[i];
			Δ = diff(dv);
			dv = mulpt2(divpt2(dv, Δ), MIN(Δ, δ));
			u->vrect.o = addpt2(u->vrect.o, dv);
			if(R < Δ)
				R = Δ;
		}
		dprint("R %.2f ε %.2f K %.2f δ %.2f n %d\n", R, ε, K, δ, n);
		if(R < ε)
			break;
		if(δ > 0.0001)
			δ *= ΔT;
		yield();
	}
	free(Fu);
}

static Layout ll = {
	.name = "force",
	.compute = compute,
};

Layout *
regforce(void)
{
	return &ll;
}
