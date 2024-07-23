#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"

View view;
Obj aintnothingthere = {nil, Onil, -1};

static Obj *visobj;

static u32int
mapvis(Graph *g, int type, ioff idx)
{
	Obj o;

	o = (Obj){g, type, idx};
	dypush(visobj, o);
	return dylen(visobj) - 1;
}

Obj
mouseselect(int x, int y)
{
	u32int i;

	if(x < 0 || y < 0 || x >= view.w || y >= view.h)
		return aintnothingthere;
	if((i = scrobj(x, y)) == 0 || i-1 >= dylen(visobj))
		return aintnothingthere;
	return visobj[i-1];
}

static void
drawguides(void)
{
	drawline(ZV, view.center, 0, 1, -1, color(theme[Cemph]));
	drawline(ZV, view.pan, 0, 2, -1, color(theme[Ctext]));
}

static inline int
drawedge(Graph *g, Node *u, Node *v, int urev, int vrev, ioff idx)
{
	u32int i;
	float m;
	Vertex du, dv;

	i = mapvis(g, Oedge, idx);
	/* FIXME: just compute angle then length vector based on that */
	/* FIXME: occasional fp errors, fixed with x≠0? */
	du = u->dir;
	m = sqrt(du.x * du.x + du.y * du.y) + 0.000001;
	du = divv(du, m);
	//du = mulv(du, Nodesz * u->length / 2);
	du = mulv(du, Nodesz / 2);
	du = urev ? subv(u->pos, du) : addv(u->pos, du);
	dv = v->dir;
	m = sqrt(dv.x * dv.x + dv.y * dv.y) + 0.000001;
	dv = divv(dv, m);
	//dv = mulv(dv, Nodesz * v->length / 2);
	dv = mulv(dv, Nodesz / 2);
	dv = vrev ? addv(v->pos, dv) : subv(v->pos, dv);
	return drawbezier(du, dv, i, color(theme[Cedge]));
}

static inline int
drawnode(Graph *g, Node *n, ioff idx)
{
	u32int i;
	float m;
	Vertex v;

	i = mapvis(g, Onode, idx);
	v = n->dir;
	if((view.flags & VFdrawarrows) == 0){
		v = n->dir;
		/*
		m = sqrt(v.x * v.x + v.y * v.y);
		v.x /= m;
		v.y /= m;
		m = Nodesz * n->length / 2.0f;
		v = mulv(v, m);
		*/
		if(drawquad(n->pos, v, i, n->col) < 0)
			return -1;
	/* FIXME */
	}else{
		m = atan2(n->dir.y, n->dir.x);
		v.x = Nodesz * cos(m);
		v.y = Nodesz * sin(m);
		if(drawline(n->pos, addv(n->pos, v), 1, 1, -1, color(theme[Cemph])) < 0)
			return -1;
	}
	if((view.flags & VFdrawlabels) != 0 && drawlabel(n, idx, color(theme[Ctext])) < 0)
		return -1;
	return 0;
}

static int
drawedges(Graph *g)
{
	ioff i, ie;
	Edge *e;
	Node *u, *v;

	for(i=0, ie=dylen(g->edges); i<ie; i++){
		e = g->edges + i;
		u = g->nodes + (e->u >> 1);
		v = g->nodes + (e->v >> 1);
		assert(u != nil && v != nil);
		drawedge(g, u, v, e->u & 1, e->v & 1, i);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	ioff i, ie;
	Node *n;

	for(i=0, ie=dylen(g->nodes); i<ie; i++){
		n = g->nodes + i;
		drawnode(g, n, i);
	}
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
		drawnodes(g);
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
	dyclear(visobj);
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
