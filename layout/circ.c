#include "strpg.h"
#include "layout.h"
#include "drw.h"
#include "graph.h"

enum{
	W = 1024,
	H = 1024,
	Area = W * H,
};

#define	C	((float)Nodesz / (Maxsz - Minsz) * 0.7f)
#define Tolerance	0.001f


typedef struct Aux Aux;
struct Aux{
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	cool(t)	(0.99985f * (t))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.0001f)

static void *
init(void)
{
	Aux *aux;

	aux = emalloc(sizeof *aux);
	aux->k = C * sqrtf((float)Area / dylen(rnodes));
	return aux;
}

static void *
new(void)
{
	int flags, orphans;
	double z;
	float n, ρ, ρ1, θ, Δ;
	Node *u, *ue;
	RNode *r;

	orphans = 0;
	/* FIXME: polar coordinates */
	Δ = drawing.xbound.max - drawing.xbound.min;
	for(r=rnodes, u=nodes, ue=u+dylen(u); u<ue; u++, r++){
		if(u->nedges == 0)
			orphans++;
		if((u->flags & FNinitx) != 0)
			r->pos[0] = u->pos0.x;
		else
			r->pos[0] = (float)(W / 2 - nrand(W)) / (W / 2);
		if((u->flags & FNinity) != 0)
			r->pos[1] = u->pos0.y;
		else
			r->pos[1] = (float)(H / 2 - nrand(H)) / (H / 2);
		if((u->flags & FNinitz) != 0)
			r->pos[2] = u->pos0.z;
		else{
			z = (double)(dylen(rnodes) - (r - rnodes)) / dylen(rnodes);
			r->pos[2] = (drawing.flags & DFnodepth) == 0
				? 0.8 * (0.5 - z)
				: 0.00001 * z;
		}
	}
	/* FIXME */
	ρ1 = Δ / (Nodesz * Ptsz);
	for(n=0, r=rnodes, u=nodes, ue=u+dylen(u); u<ue; u++, r++){
		flags = u->flags & (FNinitpos|FNfixed);
		if(flags == 0)
			continue;
		n++;
		/* FIXME */
		ρ = ρ1 + (n / Δ) * Nodesz * 192;
		θ = n * Nodesz / ρ;
		if((flags & FNfixed) != 0)
			ρ = 0.0f;
		r->pos[0] = ρ * cos(θ);
		r->pos[1] = ρ * -sin(θ);
	}
	if(orphans > 1)
		logmsg(va("layout: ignoring %d nodes with no adjacencies\n", orphans));
	return init();
}

static void
cleanup(void *aux)
{
	free(aux);
}

static int
compute(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *e, *ee;
	float t, tol, k, f, x, y, Δx, Δy, δx, δy, δ, w, uw, vw, Δr;
	RNode *r0, *r1, *r, *v;
	Aux *aux;
	Node *u, *u0;
	Clk clk = {.lab = "layiter"};

	aux = arg;
	k = aux->k;
	t = k;
	tol = Tolerance * k;
	u0 = nodes + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(rnodes);
	skip = nlaythreads;
	for(;;){
		CLK0(clk);
		Δr = 0;
		for(u=u0, r=r0; r<r1; r+=skip, u+=skip){
			if((*stat & LFstop) != 0)
				return 0;
			if(u->nedges == 0)
				continue;
			fixed = u->flags & FNfixed;
			if(fixed == FNfixed)
				continue;
			uw = r->len;
			x = r->pos[0];
			y = r->pos[1];
			Δx = Δy = 0.0f;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δ = Δ(δx, δy);
				f = Fr(δ, k);
				vw = v->len;
				w = C * MIN(uw, vw);
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=edges+u->eoff, ee=e+u->nedges-u->nin; e<ee; e++){
				v = rnodes + (*e >> 2);
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				f = Fa(δ, k);
				vw = v->len;
				if(uw < vw)
					w = uw / vw;
				else
					w = vw / uw;
				w *= C;
				Δx += w * f * δx / δ;
				Δy += w * f * δy / δ;
			}
			δ = Δ(Δx, Δy);
			/* limiting layouting area doesn't work well for long linear graphs */
			if((fixed & FNfixedx) == 0){
				f = MIN(t, fabsf(Δx));
				Δx = f * Δx / δ;
				x += Δx;
				r->pos[0] = x;
				if(Δr < Δx)
					Δr = Δx;
			}
			if((fixed & FNfixedy) == 0){
				f = MIN(t, fabsf(Δy));
				Δy = f * Δy / δ;
				y += Δy;
				r->pos[1] = y;
				if(Δr < Δy)
					Δr = Δy;
			}
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

static Target ll = {
	.name = "circ",
	.init = init,
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Target *
regcirc(void)
{
	return &ll;
}
