#include "strpg.h"
#include "layout.h"
#include "graph.h"
#include "drw.h"

enum{
	W = 1024,
	H = 1024,
	Area = W * H,
};

#define	C	((float)Nodesz / (Maxsz - Minsz) * 0.7f)
#define Tolerance	0.001f

typedef struct Aux Aux;
struct Aux{
	Node *nodes;
	ioff *edges;
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	cool(t)	(0.99985f * (t))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.0001f)

static void *
new(Graph *g)
{
	int orphans;
	ioff i, *e, *ee;
	float k;
	double z, Δ;
	Node *u, *ue;
	RNode *r, *t;
	Aux *aux;

	orphans = 0;
	k = C * sqrtf((float)Area / dylen(rnodes));
	Δ = (drawing.xbound.max - drawing.xbound.min) / 2.0f;
	for(r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++){
		if(u->nedges == 0)
			orphans++;
		if((u->attr.flags & FNinitx) != 0)
			r->pos[0] = u->attr.pos0.x - Δ;
		else
			r->pos[0] = -Δ;
		if((u->attr.flags & FNinity) != 0)
			r->pos[1] = u->attr.pos0.y;
		else
			r->pos[1] = (float)(H / 8 - nrand(H/4)) / (H / 8);
		if((u->attr.flags & FNinitz) != 0)
			r->pos[2] = u->attr.pos0.z;
		else{
			z = (double)(dylen(rnodes) - (r - rnodes)) / dylen(rnodes);
			r->pos[2] = (drawing.flags & DFnodepth) == 0
				? 0.8 * (0.5 - z)
				: 0.00001 * z;
		}
	}
	/* attempt to place free nodes next to a fixed adjacency */
	for(r=rnodes, u=g->nodes, ue=u+dylen(u); u<ue; u++, r++){
		if((u->attr.flags & FNfixedx) != 0 || u->nedges == 0)
			continue;
		for(i=u-g->nodes,e=g->edges+u->eoff, ee=e+u->nedges-1; e<ee; e++){
			i = *e >> 2;
			if((g->nodes[i].attr.flags & FNfixedx) != 0)
				break;
		}
		t = rnodes + i;
		r->pos[0] = 3.0f - t->pos[0] + nrand(6);
	}
	if(orphans > 1)
		logmsg(va("layout: ignoring %d nodes with no adjacencies\n", orphans));

	aux = emalloc(sizeof *aux);
	aux->nodes = g->nodes;
	aux->edges = g->edges;
	aux->k = k;
	return aux;
}

static void
cleanup(void *aux)
{
	free(aux);
}

/* FIXME: compensate for the fact that movable nodes move towards fixed ones,
 * rather than both toward each other */
static int
compute(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *edges, *e, *ee;
	float t, tol, k, f, x, y, Δx, Δy, δx, δy, δ, w, uw, vw, Δr;
	RNode *r0, *r1, *r, *v;
	Aux *aux;
	Node *u, *u0;
	Clk clk = {.lab = "layiter"};

	aux = arg;
	edges = aux->edges;
	k = aux->k;
	t = k;
	tol = Tolerance * k;
	u0 = aux->nodes + i;
	r0 = rnodes + i;
	r1 = rnodes + dylen(aux->nodes);
	skip = nlaythreads;
	for(;;){
		CLK0(clk);
		Δr = 0;
		for(u=u0, r=r0; r<r1; r+=skip, u+=skip){
			if((*stat & LFstop) != 0)
				return 0;
			if(u->nedges == 0)
				continue;
			fixed = u->attr.flags & FNfixed;
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
			for(e=edges+u->eoff, ee=e+u->nedges+u->nin; e<ee; e++){
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
	.name = "linear",
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

Target *
reglinear(void)
{
	return &ll;
}
