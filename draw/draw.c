#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"

View view;
RNode *rnodes;
REdge *redges;
ssize ndnodes, ndedges;

/* FIXME: not great */
void
fixlengths(Graph *g, int min, int max)
{
	int Δ;
	Node *n, *ne;
	RNode *r;

	Δ = MAX(1, max - min);
	for(n=g->nodes, ne=n+dylen(n), r=rnodes; n<ne; n++, r++)
		r->len = Maxsz - (Maxsz - Minsz) * exp(-n->length / (float)Δ);
}

static inline void
drawedge(REdge *r, RNode *u, RNode *v, int urev, int vrev)
{
	float m;
	Vertex p1, p2, du, dv;

	du.x = u->dir[0];
	du.y = u->dir[1];
	du.z = u->dir[2];
	p1.x = u->pos[0];
	p1.y = u->pos[1];
	p1.z = u->pos[2];
	dv.x = v->dir[0];
	dv.y = v->dir[1];
	dv.z = v->dir[2];
	p2.x = v->pos[0];
	p2.y = v->pos[1];
	p2.z = v->pos[2];
	m = sqrtf(du.x * du.x + du.y * du.y) + 0.000001;
	du = divv(du, m);
	du = mulv(du, u->len * Nodesz / 2.0f);
	du = urev ? addv(p1, du) : subv(p1, du);
	m = sqrtf(dv.x * dv.x + dv.y * dv.y) + 0.000001;
	dv = divv(dv, m);
	dv = mulv(dv, v->len * Nodesz / 2.0f);
	dv = vrev ? subv(p2, dv) : addv(p2, dv);
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos1[2] = du.z;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
	r->pos2[2] = dv.z;
}

static REdge *
drawedges(REdge *r, RNode *rn, Graph *g)
{
	ioff x, *e, *ee;
	Node *n, *ne;
	RNode *u, *v;

	for(u=rn, n=g->nodes, ne=n+dylen(n); n<ne; n++, u++){
		for(e=n->out, ee=e+dylen(e); e<ee; e++, r++){
			x = *e;
			v = rnodes + (x >> 2);
			assert(v >= rn && v < rnodes + dylen(rnodes));
			drawedge(r, u, v, x & 2, x & 1);
		}
	}
	return r;
}

static inline void
faceyourfears(RNode *ru, Node *u)
{
	float x, y, Δ, Δx, Δy;
	float θ, c, s;
	ioff e, *i, *ie;
	RNode *rv;

	x = ru->pos[0];
	y = ru->pos[1];
	c = s = 0.0;
	for(i=u->in, ie=i+dylen(i); i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		Δx = rv->pos[0] - x;
		Δy = rv->pos[1] - y;
		if((e & 2) != 0){
			Δx = -Δx;
			Δy = -Δy;
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy);
		c += Δx / Δ;
		s += Δy / Δ;
	}
	for(i=u->out, ie=i+dylen(i); i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		Δx = x - rv->pos[0];
		Δy = y - rv->pos[1];
		if((e & 2) != 0){
			Δx = -Δx;
			Δy = -Δy;
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy);
		c += Δx / Δ;
		s += Δy / Δ;
	}
	θ = fmodf(atan2f(s, c), 2.0f*(float)PI);
	ru->dir[0] = cosf(θ);
	ru->dir[1] = sinf(θ);
	ru->dir[2] = 0.0f;
}

static RNode *
drawnodes(RNode *r, Graph *g)
{
	Node *n, *e;

	for(n=g->nodes, e=n+dylen(n); n<e; n++, r++)
		faceyourfears(r, n);
	return r;
}

static int
drawworld(int go)
{
	int r;
	Graph *g;
	RNode *rn, *rne;
	REdge *re;

	r = 0;
	rn = rnodes;
	re = redges;
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type <= FFdead || g->layout == nil)
			continue;
		if((g->flags & GFdrawme) != 0)
			r++;
		else if(!go){
			rn += dylen(g->nodes);
			re += g->nedges;
			continue;
		}
		lockgraph(g, 0);
		rne = drawnodes(rn, g);
		re = drawedges(re, rn, g);
		unlockgraph(g, 0);
		rn = rne;
	}
	ndnodes = rn - rnodes;
	ndedges = re - redges;
	return r;
}

static void
drawui(void)
{
	if(ndedges < 1 || selbox[0].pos2[0] - selbox[0].pos1[0] == 0.0f)
		return;
	assert(ndedges + nelem(selbox) <= dylen(redges));	/* realloc would race */
	memcpy(redges + ndedges, selbox, sizeof selbox);
	ndedges += nelem(selbox);
}

int
redraw(int go)
{
	double t;

	if((debug & Debugperf) != 0)
		t = μsec();
	go = drawworld(go);
	drawui();
	DPRINT(Debugperf, "redraw: %.2f ms", (μsec() - t) / 1000);
	return go;
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	settheme();
	initcol();
	initsysdraw();
}
