#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Length = 300,
	Nrep = 1000,
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
	Vertex *Fu, dv;
	Node *u, *from, *v, *ne;
	Edge **e, **ee;

	/* initial random placement, but in same scale as springs */
	for(u=g->nodes.buf, ne=u+g->nodes.len; u<ne; u++)
		//putnode(u, nrand(view.dim.v.x), nrand(view.dim.v.y));
		putnode(u, 10+nrand(Length), 10+nrand(Length));
	K = ceil(sqrt((double)Length * Length / g->nodes.len));
	/* arbitrary displacement minimum function */
	ε = ceil(sqrt((double)Length * Length / g->edges.len));
	δ = 1.0;
	Fu = emalloc(g->nodes.len * sizeof *Fu);
	u = g->nodes.buf;
	ne = u + g->nodes.len;
	for(n=0; n<Nrep; n++){
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
			for(e=u->in.buf,ee=e+u->in.len; e!=nil && e<ee; e++){
				if((from = id2n(g, (*e)->from >> 1)) == nil)
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


/* FIXME: kludge */
{
	Quad d;

	//Quad d = Qd(view.dim.o, addpt2(view.dim.o, view.dim.v));
	for(u=g->nodes.buf, d=u->q; u<ne; u++){
		if(u->q.o.x < d.o.x)
			d.o.x = u->q.o.x;
		else if(d.v.x < u->q.o.x + u->q.v.x)
			d.v.x = u->q.o.x + u->q.v.x;
		if(u->q.o.y < d.o.y)
			d.o.y = u->q.o.y;
		else if(d.v.y < u->q.o.y + u->q.v.y)
			d.v.y = u->q.o.y + u->q.v.y;
	}
	//g->dim = Qd(d.o, subpt2(d.v, d.o));
	g->dim = Qd(d.o, subpt2(d.v, d.o));
	dprint("dim %.2f,%.2f %.2f,%.2f\n", g->dim.o.x, g->dim.o.y, g->dim.v.x, g->dim.v.y);
	redraw();
	sleep(1);
}


	}
	free(Fu);
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
