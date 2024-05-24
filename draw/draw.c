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
mapvis(Graph *g, int type, ssize idx)
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
drawedge(Graph *g, Node *u, Node *v, double w, ssize idx)
{
	u32int i;

	i = mapvis(g, Oedge, idx);
	return drawbezier(u->pos, v->pos, w, i, color(theme[Cedge]));
}

static inline int
drawnode(Graph *g, Node *n, ssize idx)
{
	u32int i;

	i = mapvis(g, Onode, idx);
	if((view.flags & VFdrawarrows) == 0){
		if(drawquad(n->pos, n->rot, i, n->col) < 0)
			return -1;
	}else if(drawline(n->pos, addv(n->pos, n->dir), MAX(0., view.zoom/5), 1, -1, color(theme[Cemph])) < 0)
		return -1;
	if((view.flags & VFdrawlabels) != 0 && drawlabel(n, color(theme[Ctext])) < 0)
		return -1;
	return 0;
}

static int
drawedges(Graph *g)
{
	ssize i;
	Edge *e;
	Node *u, *v;

	for(i=g->edge0.next; i>=0; i=e->next){
		e = g->edges + i;
		u = getnode(g, e->u >> 1);
		v = getnode(g, e->v >> 1);
		assert(u != nil && v != nil);
		// FIXME: honor w param
		drawedge(g, u, v, MAX(0., view.zoom/5), i);
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	ssize i;
	Node *n;

	for(i=g->node0.next; i>=0; i=n->next){
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
