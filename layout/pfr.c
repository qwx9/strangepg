#include "strpg.h"
#include "drw.h"
#include "layout.h"
#include "cmd.h"

enum{
	W = 1024,
	H = 1024,
	Area = W * H,
};

#define	C	(drawing.nodesz * Minsz / Maxsz)
#define Tolerance	0.005f

typedef struct Aux Aux;
struct Aux{
	float k;
};

#define Fa(x, k)	((x) * (x) / (k))
#define Fr(x, k)	((k) * (k) / (x))
#define	cool(t)	(0.99985f * (t))
#define	Δ(x, y)	(sqrtf((x) * (x) + (y) * (y)) + 0.0001f)
#define	Δ3(x, y, z)	(sqrtf((x) * (x) + (y) * (y) + (z) * (z)) + 0.0001f)

static void *
init(void)
{
	float dim[2];
	Aux *aux;

	if(drawing.xbound.min < drawing.xbound.max)
		dim[0] = (drawing.xbound.max - drawing.xbound.min) / 2.0f;
	else
		dim[0] = W;
	if(drawing.ybound.min < drawing.ybound.max)
		dim[1] = (drawing.ybound.max - drawing.ybound.min) / 2.0f;
	else
		dim[1] = H;
	aux = emalloc(sizeof *aux);
	aux->k = C * sqrtf(dim[0] * dim[1] / dylen(rnodes));
	return aux;
}

static void
initdim(float *mid, float *var)
{
	if(drawing.xbound.min < drawing.xbound.max){
		var[0] = (drawing.xbound.max - drawing.xbound.min) / 2.0f;
		mid[0] = drawing.xbound.min + var[0];
	}else{
		var[0] = W;
		mid[0] = 0;
	}
	if(drawing.ybound.min < drawing.ybound.max){
		var[1] = (drawing.ybound.max - drawing.ybound.min) / 2.0f;
		mid[1] = drawing.ybound.min + var[1];
	}else{
		var[1] = H;
		mid[1] = 0;
	}
	if(drawing.zbound.min < drawing.zbound.max){
		var[2] = (drawing.zbound.max - drawing.zbound.min) / 2.0f;
		mid[2] = drawing.zbound.min + var[2];
	}else{
		var[2] = W;
		mid[2] = 0;
	}
	drawing.mid[0] = mid[0];
	drawing.mid[1] = mid[1];
	drawing.mid[2] = mid[2];
}

static inline int
hasadjacencies(Node *u)
{
	ioff i, *e, *ee;

	for(i=u-nodes, e=edges+u->eoff, ee=e+u->nedges; e<ee; e++)
		if((*e >> 2 & 0x3fffffff) != i)
			return 1;
	return 0;
}

static void *
new_(int nuke, int is3d)
{
	int flags, orphans, nosphere;
	ssize nn;
	float l, θ, φ, z, c, f, var[3], mid[3];
	Node *u, *ue;
	RNode *r;

	if(!nuke)
		return init();
	orphans = 0;
	initdim(mid, var);
	initpos();
	f = 2.0f * drawing.nodesz;
	θ = 0.0f;
	z = 0.0f;	/* shut compiler up */
	nosphere = var[0] > W || var[1] > H || var[2] > W;
	nn = dylen(nodes);
	for(r=rnodes, u=nodes, ue=u+nn; u<ue; u++, r++){
		u->flags &= ~FNorphan;
		flags = u->flags;
		if(!hasadjacencies(u)){
			u->flags |= FNorphan;
			flags |= FNorphan;	/* shut up about maybe uninitialized */
			orphans++;
		}else if(!nosphere){
			l = xfrand() * var[0];
			φ = xfrand() * 2.0 * PI;
			if(is3d){
				z = l - 2.0f * l * xfrand();
				θ = asinf(z / l);
			}
			c = cosf(φ);
		}
		if((flags & FNinitx) == 0){
			if(flags & FNorphan)
				r->pos[0] = mid[0] - xfrand() * (2.0f * var[0]);
			else if(nosphere){
				l = var[0];
				r->pos[0] = (l - xfrand() * (2.0f * l)) / (l / f);
			}else if(!is3d)
				r->pos[0] = (l * c) / (f * var[0]);
			else
				r->pos[0] = (l * c * cosf(θ)) / (f * var[0]);
		}
		if((flags & FNinity) == 0){
			if(flags & FNorphan)
				r->pos[1] = mid[1] - xfrand() * (2.0f * var[1]);
			else if(nosphere){
				l = var[1];
				r->pos[1] = (l - xfrand() * (2.0f * l)) / (l / f);
			}else if(!is3d){
				if(θ < -PI || θ >= PI)
					r->pos[1] = (l * -sqrtf(1 - c * c)) / (f * var[1]);
				else
					r->pos[1] = (l * sqrtf(1 - c * c)) / (f * var[1]);
			}else
				r->pos[1] = (l * cosf(θ) * sin(φ)) / (f * var[1]);
		}
		if((flags & FNinitz) == 0){
			if(!is3d || flags & FNorphan){
				z = -0.5f + (float)(nn - (r - rnodes)) / nn;
				r->pos[2] = (drawing.flags & DFnodepth) == 0
					? 0.1f * z
					: 0.00001f * z;
			}else if(nosphere){
				l = var[2];
				r->pos[2] = (l - xfrand() * (2.0f * l)) / (l / f);
			}else
				r->pos[2] = z / (f * var[2]);
		}
	}
	if(orphans > 0)
		logmsg(va("layout: ignoring %d nodes with no adjacencies\n", orphans));
	return init();
}

static void *
new(int nuke)
{
	drawing.flags &= ~DF3d;
	return new_(nuke, 0);
}

static void *
new3d(int nuke)
{
	drawing.flags |= DF3d;
	return new_(nuke, 1);
}

static void
cleanup(void *aux)
{
	free(aux);
}

/* FIXME: find more intelligent way to merge the two */
static int
compute3d(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *e, *ee;
	float t, tol, k, f, x, y, z, Δx, Δy, Δz, δx, δy, δz, δ, uw, vw, Δr;
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
			z = r->pos[2];
			Δx = Δy = Δz = 0.0f;
			for(v=rnodes; v<r1; v++){
				if(r == v)
					continue;
				δx = x - v->pos[0];
				δy = y - v->pos[1];
				δz = z - v->pos[2];
				δ = Δ3(δx, δy, δz);
				f = Fr(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
				Δz += f * δz / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
				v = rnodes + (*e >> 2);
				vw = v->len;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δz = v->pos[2] - z;
				δ = Δ3(δx, δy, δz);
				δ -= (uw + vw) / 2.0f;
				δ += 0.0001f;
				f = Fa(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
				Δz += f * δz / δ;
			}
			δ = Δ3(Δx, Δy, Δz);
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
			f = MIN(t, fabs(Δz));
			Δz = f * Δz / δ;
			z += Δz;
			r->pos[2] = z;
			if(Δr < Δz)
				Δr = Δz;
		}
		if(Δr < tol)
			return 0;
		t = cool(t);
		CLK1(clk);
	}
}

/* skipping through nodes ensures that each thread works on a more
 * "global" part of the graph, otherwise the quality of the layout
 * is horrible; preparing the data in contiguous chunks in some
 * manner would be better but more complicated, as it stands,
 * performance doesn't really suffer because of this, possibly
 * because other threads would be accessing what was prefetched
 * anyway */
static int
compute(void *arg, volatile int *stat, int i)
{
	int fixed, skip;
	ioff *e, *ee;
	float t, tol, k, f, x, y, Δx, Δy, δx, δy, δ, uw, vw, Δr;
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
				Δx += f * δx / δ;
				Δy += f * δy / δ;
			}
			if((*stat & LFstop) != 0)
				return 0;
			for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
				v = rnodes + (*e >> 2);
				vw = v->len;
				δx = v->pos[0] - x;
				δy = v->pos[1] - y;
				δ = Δ(δx, δy);
				δ -= (uw + vw) / 2.0f;
				δ += 0.0001f;
				f = Fa(δ, k);
				Δx += f * δx / δ;
				Δy += f * δy / δ;
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
	.name = "pfr",
	.init = init,
	.new = new,
	.cleanup = cleanup,
	.compute = compute,
};

static Target ll3d = {
	.name = "pfr3d",
	.init = init,
	.new = new3d,
	.cleanup = cleanup,
	.compute = compute3d,
};

Target *
regpfr3d(void)
{
	return &ll3d;
}
Target *
regpfr(void)
{
	return &ll;
}
