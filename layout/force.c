#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Niter = 100,
	Length = 100,
	Temp0 = 20,
};

static double
diff(Vertex v)
{
	return sqrt(v.x * v.x + v.y * v.y) + 0.0001;
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

static double
cooldown(double T)
{
	return T > 0.0001 ? 0.999 * T : 0.0001;
}

static int
compute(Graph *g)
{
	int i, j, r;
	double k, T, R, Δ;
	Vertex *Δr, dv;
	Node *u, *iu, *v, *ne;
	Edge **e, **ee;

	k = sqrt(Length * Length / g->nodes.len);
	T = Temp0;
	Δr = emalloc(g->nodes.len * sizeof *Δr);
	u = g->nodes.buf;
	ne = u + g->nodes.len;
	for(r=0; r<Niter;){
		for(u=g->nodes.buf, i=0; u<ne; i++, u++){
			/* ∀u,v ∈ E, Δr += d_uv / |d_uv| * Fr */
			for(v=g->nodes.buf; v<ne; v++){
				if(v == u)
					continue;
				dv = subpt2(u->q.o, v->q.o);
				Δ = diff(dv);
				Δr[i] = addpt2(Δr[i], mulpt2(divpt2(dv, Δ), repulsion(Δ, k)));
			}
			/* ∀u,v ∈ E:
			 *	Δr_u -= d_uv/|d_uv| * Fa
			 *	Δr_v += d_uv/|d_uv| * Fa */
			for(e=u->in.buf,ee=e+u->in.len; e!=nil && e<ee; e++){
				if((iu = id2n(g, (*e)->from >> 1)) == nil)
					panic("phase error -- missing incident node");
				dv = subpt2(iu->q.o, u->q.o);
				Δ = diff(dv);
				Δr[i] = addpt2(Δr[i], mulpt2(divpt2(dv, Δ), attraction(Δ, k)));
				j = vecindexof(&g->nodes, iu);
				Δr[j] = subpt2(Δr[j], mulpt2(divpt2(dv, Δ), attraction(Δ, k)));
			}
		}
		for(u=g->nodes.buf, R=0, i=0; u<ne; i++, u++){
			dv = Δr[i];
			Δ = diff(dv);
			dv = mulpt2(divpt2(dv, Δ), MIN(Δ, T));
			u->q.o = addpt2(u->q.o, dv);
			R += Δ;
		}
		//if(R < 0.0005 * g->nodes.len)
			r++;
		T = cooldown(T);
	}
	free(Δr);
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
