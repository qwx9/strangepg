#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Length = 100,
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
	int i, j, n;
	double K, δ, R, Δ, ε, l;
	usize *ep, *ee;
	Vertex *Fu, dv;
	Node *u, *from, *v, *ne;
	Edge *e;

	if(g->edges.len < 2){
		warn("no links to hand");
		// FIXME: error exit
		return;
	}
	l = sqrt(view.dim.v.x*view.dim.v.x+view.dim.v.y*view.dim.v.y)
		/ (16. / log10(dylen(g->nodes) + g->edges.len));
	/* initial random placement, but in same scale as springs */
	for(u=g->nodes, ne=u+dylen(g->nodes); u<ne; u++)
	if(!noui)
		putnode(u, nrand(l) - l / 2, nrand(l) - l / 2);
	else
		putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
	K = ceil(sqrt(l * l / dylen(g->nodes)));
	/* arbitrary displacement minimum function */
	ε = ceil(sqrt(l * l / g->edges.len));
	δ = 1.0;
	Fu = emalloc(dylen(g->nodes) * sizeof *Fu);
	u = g->nodes;
	ne = u + dylen(g->nodes);
	for(n=0; n<Nrep; n++){
		memset(Fu, 0, dylen(g->nodes) * sizeof *Fu);
		for(u=g->nodes, i=0; u<ne; i++, u++)
			for(v=g->nodes; v<ne; v++){
				if(v == u)
					continue;
				dv = subpt2(u->q1.o, v->vrect.o);
				Δ = diff(dv);
				Fu[i] = addpt2(Fu[i], mulpt2(divpt2(dv, Δ), v->w * repulsion(Δ, K)));
			}
		for(u=g->nodes, i=0; u<ne; i++, u++)
			for(ep=u->in.buf,ee=ep+u->in.len; ep!=nil && ep<ee; ep++){
				e = vecp(&g->edges, *ep);
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
		for(u=g->nodes, R=0, i=0; u<ne; i++, u++){
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
