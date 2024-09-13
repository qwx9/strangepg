#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"

/* FIXME: right now the renderer just redraws everything, which is fine
 * up until some size of graph, at which point performance starts to drop
 * even though many objects likely have not changed at all */
View view;
RNode *rnodes;
REdge *redges;

static inline void
drawedge(ioff i, ioff u, ioff v, int urev, int vrev)
{
	float m;
	Vertex p1, p2, du, dv;
	REdge *r;
	RNode *n1, *n2;

	/* FIXME */
	assert(u >= 0 && u < dylen(rnodes) && v >= 0 && v < dylen(rnodes));
	n1 = rnodes + u;
	n2 = rnodes + v;
	du.x = n1->dir[0];
	du.y = n1->dir[1];
	du.z = 0.0f;
	p1.x = n1->pos[0];
	p1.y = n1->pos[1];
	p1.z = 0.0f;
	dv.x = n2->dir[0];
	dv.y = n2->dir[1];
	dv.z = 0.0f;
	p2.x = n2->pos[0];
	p2.y = n2->pos[1];
	p2.z = 0.0f;

	m = sqrt(du.x * du.x + du.y * du.y) + 0.000001;
	du = divv(du, m);
	//du = mulv(du, Nodesz * n1->length / 2);
	du = mulv(du, Nodesz / 2);
	du = urev ? addv(p1, du) : subv(p1, du);
	m = sqrt(dv.x * dv.x + dv.y * dv.y) + 0.000001;
	dv = divv(dv, m);
	//dv = mulv(dv, Nodesz * n2->length / 2);
	dv = mulv(dv, Nodesz / 2);
	dv = vrev ? subv(p2, dv) : addv(p2, dv);

	r = redges + i;
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
}

static int
drawedges(Graph *g)
{
	ioff i;
	Edge *e, *ee;

	for(i=0, e=g->edges, ee=e+dylen(e); e<ee; e++, i++)
		drawedge(i, e->u >> 1, e->v >> 1, e->u & 1, e->v & 1);
	return 0;
}

static inline void
faceyourfears(Graph *g, Node *u, RNode *ru)
{
	int n;
	float x, y, Δx, Δy;
	double θ, c, s;
	ioff *i, *ie;
	Edge *e;
	RNode *rv;

	x = ru->pos[0];
	y = ru->pos[1];
	c = s = 0.0;
	n = 0;
	for(i=u->in, ie=i+dylen(i); i<ie; i++, n++){
		e = g->edges + *i;
		rv = rnodes + (e->u >> 1);
		Δx = rv->pos[0] - x;
		Δy = rv->pos[1] - y;
		θ = atan2(Δy, Δx);
		c += cos(θ);
		s += sin(θ);
	}
	for(i=u->out, ie=i+dylen(i); i<ie; i++, n++){
		e = g->edges + *i;
		rv = rnodes + (e->v >> 1);
		Δx = x - rv->pos[0];
		Δy = y - rv->pos[1];
		θ = atan2(Δy, Δx);
		c += cos(θ);
		s += sin(θ);
	}
	if(n > 0){
		c /= n;
		s /= n;
	}
	θ = atan2(s, c);
	ru->dir[0] = cos(θ);
	ru->dir[1] = sin(θ);
}

static void
drawnodes(Graph *g)
{
	Node *n, *e;
	RNode *r;

	for(r=rnodes, n=g->nodes, e=n+dylen(n); n<e; n++, r++)
		faceyourfears(g, n, r);
}

static int
drawworld(void)
{
	int r;
	Graph *g;

	r = 0;
	lockgraphs(0);
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type <= FFdead || g->layout == nil)
			continue;
		if((g->flags & GFdrawme) != 0)
			r++;
		DPRINT(Debugdraw, "drawworld: draw graph %#p", g);
		drawnodes(g);
		drawedges(g);
	}
	unlockgraphs(0);
	return r;
}

int
redraw(void)
{
	int go;
	static Clk clk = {.lab = "redraw"};

	go = 1;
	CLK0(clk);
	if(!drawworld())
		go = 0;
	CLK1(clk);
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
