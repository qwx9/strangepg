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

static void
drawguides(void)
{
	drawline(ZV, view.center, 0, 1, -1, color(theme[Cemph]));
	drawline(ZV, view.pan, 0, 2, -1, color(theme[Ctext]));
}

static inline void
drawedge(Graph *g, ioff i, ioff u, ioff v, int urev, int vrev)
{
	float m;
	Color *c;
	Vertex p1, p2, du, dv;
	REdge *r;
	RNode *n1, *n2;

	/* FIXME */
	assert(u >= 0 && u < dylen(rnodes) && v >= 0 && v < dylen(rnodes));
	n1 = rnodes + u;
	n2 = rnodes + v;
	du.x = n1->dir[0];
	du.y = n1->dir[1];
	p1.x = n1->pos[0];
	p1.y = n1->pos[1];
	dv.x = n2->dir[0];
	dv.y = n2->dir[1];
	p2.x = n2->pos[0];
	p2.y = n2->pos[1];
	m = sqrt(du.x * du.x + du.y * du.y) + 0.000001;
	du = divv(du, m);
	//du = mulv(du, Nodesz * n1->length / 2);
	du = mulv(du, Nodesz / 2);
	du = urev ? subv(p1, du) : addv(p1, du);
	m = sqrt(dv.x * dv.x + dv.y * dv.y) + 0.000001;
	dv = divv(dv, m);
	//dv = mulv(dv, Nodesz * n2->length / 2);
	dv = mulv(dv, Nodesz / 2);
	dv = vrev ? addv(p2, dv) : subv(p2, dv);
	r = redges + i;
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
}

static inline void
drawnode(Graph *g, ioff i)
{
	float m;
	Vertex v;
	RNode *r;
	Node *n;

	assert(i >= 0 && i < dylen(rnodes));
	r = rnodes + i;
	/* FIXME */
	n = g->nodes + i;
	/* FIXME: done in layout */
	/*
	r->pos[0] = n->pos.x;
	r->pos[1] = n->pos.y;
	r->dir[0] = n->dir.x;
	r->dir[1] = n->dir.y;
	*/
}

static int
drawedges(Graph *g)
{
	ioff i, ie;
	Edge *e;

	for(i=0, ie=dylen(g->edges); i<ie; i++){
		e = g->edges + i;
		drawedge(g, i, e->u >> 1, e->v >> 1, e->u & 1, e->v & 1);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	ioff i, ie;

	for(i=0, ie=dylen(g->nodes); i<ie; i++)
		drawnode(g, i);
	return 0;
}

static void
drawworld(void)
{
	Graph *g;

	lockgraphs(0);
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type <= FFdead || g->layout == nil)
			continue;
		DPRINT(Debugdraw, "drawworld: draw graph %#p", g);
		drawedges(g);
		//drawnodes(g);
	}
	unlockgraphs(0);
	if(debug)
		drawguides();
}

int
redraw(int force)
{
	int go;
	static Clk clk = {.lab = "redraw"};

	go = 1;
	if(!reshape(force))
		go = 0;
	CLK0(clk);
	cleardraw();
	drawworld();
	CLK1(clk);
	return go;
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	settheme();
	cmap = kh_init(cmap);
	initsysdraw();
}
