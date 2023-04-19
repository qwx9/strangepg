#include "strpg.h"
#include "layout.h"

/* force-directed layout */

enum{
	Niter = 1,
	Length = 100,
	Temp0 = 20,
};

static double
diffdist(Vertex v)
{
	return sqrt(v.x * v.x + v.y * v.y);
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
    double cold;

	cold = 0.0001;
	return T > cold ? 0.999 * T : T;
}

static int
compute(Graph *g)
{
	int i, j, r;
	double k, T, totΔr, Δ;
	Vertex *Δr, dv;
	Node *ve, *up, *vp, *uu;
	Edge **ep, **ee;

	k = sqrt(Length * Length / g->nodes.len);
	T = Temp0;
	Δr = emalloc(g->nodes.len * sizeof *Δr);
	ve = vecget(&g->nodes, g->nodes.len - 1);	/* just being defensive */
	for(r=0; r<Niter;){
		for(up=vecget(&g->nodes, 0), i=0; up<ve; i++, up++){
			/* ∀u,v ∈ E, Δr += d_uv / |d_uv| * Fr */
			for(vp=vecget(&g->nodes, 0); vp<ve; vp++){
				if(vp == up)
					continue;
				dv = subvx(up->q.u, vp->q.u);
				Δ = diffdist(dv);
				Δr[i] = addvx(Δr[i], mulvx(divvx(dv, Δ), repulsion(Δ, k)));
			}
			/* ∀u,v ∈ E:
			 *	Δr_u -= d_uv/|d_uv| * Fa
			 *	Δr_v += d_uv/|d_uv| * Fa */
			for(ep=vecget(&up->in, 0), ee=vecget(&up->in, up->in.len - 1); ep!=nil&&ep<ee; ep++){
				if((uu = id2n(g, (*ep)->u >> 1)) == nil)
					sysfatal("phase error");
				dv = subvx(uu->q.u, up->q.u);
				Δ = diffdist(dv);
				Δr[i] = addvx(Δr[i], mulvx(divvx(dv, Δ), attraction(Δ, k)));
				j = vecindexof(&g->nodes, uu);
				Δr[j] = subvx(Δr[j], mulvx(divvx(dv, Δ), attraction(Δ, k)));
			}
		}
		totΔr = 0;
		for(vp=vecget(&g->nodes, 0), i=0; vp<ve; i++, vp++){
			dv = Δr[i];
			Δ = diffdist(dv);
			dv = mulvx(divvx(dv, Δ), MIN(Δ, T));
			vp->q.u = addvx(vp->q.u, dv);
			totΔr = totΔr + Δ;
		}
		if(totΔr < 0.0005 * g->nodes.len)
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
