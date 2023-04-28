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

static int
compute(Graph *g)
{
	int i, j, n;
	double K, δ, R, Δ, ε;
	usize *ep, *ee;
	Vertex *Fu, dv;
	Node *u, *from, *v, *ne;
	Edge *e;

	if(g->edges.len < 2){
		warn("no links to hand");
		return -1;
	}
	/* initial random placement, but in same scale as springs */
	for(u=g->nodes.buf, ne=u+g->nodes.len; u<ne; u++)
		//putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
		putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
	K = ceil(sqrt((double)Length * Length / g->nodes.len));
	/* arbitrary displacement minimum function */
	ε = ceil(sqrt((double)Length * Length / g->edges.len));
	δ = 1.0;
	Fu = emalloc(g->nodes.len * sizeof *Fu);
	u = g->nodes.buf;
	ne = u + g->nodes.len;
	for(n=0; n<Nrep; n++){
		if(earlyexit())
			break;
		memset(Fu, 0, g->nodes.len * sizeof *Fu);
		for(u=g->nodes.buf, i=0; u<ne; i++, u++)
			for(v=g->nodes.buf; v<ne; v++){
				if(v == u)
					continue;
				dv = subpt2(u->q.o, v->q.o);
				Δ = diff(dv);
				Fu[i] = addpt2(Fu[i], mulpt2(divpt2(dv, Δ), repulsion(Δ, K)));
			}
		for(u=g->nodes.buf, i=0; u<ne; i++, u++)
			for(ep=u->in.buf,ee=ep+u->in.len; ep!=nil && ep<ee; ep++){
				e = vecp(&g->edges, *ep);
				if((from = e2n(g, e->from)) == nil)
					panic("phase error -- missing incident node");
				dv = subpt2(from->q.o, u->q.o);
				Δ = diff(dv);
				dv = mulpt2(divpt2(dv, Δ), attraction(Δ, K));
				Fu[i] = addpt2(Fu[i], dv);
				j = vecindexof(&g->nodes, from);
				Fu[j] = subpt2(Fu[j], dv);
			}
		for(u=g->nodes.buf, R=0, i=0; u<ne; i++, u++){
			dv = Fu[i];
			Δ = diff(dv);
			dv = mulpt2(divpt2(dv, Δ), MIN(Δ, δ));
			u->q.o = addpt2(u->q.o, dv);
			if(R < Δ)
				R = Δ;
		}
		dprint("R %.2f ε %.2f K %.2f δ %.2f\n", R, ε, K, δ);
		if(R < ε)
			break;
		if(δ > 0.0001)
			δ *= ΔT;
		if(debug){
			triggerdraw(DTrender);
			sleep(10);
		}else if(drawstep && n % 25 == 0)
			triggerdraw(DTrender);
	}
	free(Fu);
	triggerdraw(DTrender);
	return 0;
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
