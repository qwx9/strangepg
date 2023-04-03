#include "strpg.h"
#include "drawprv.h"

/* interface:
 * take a quad layout
 * rotate rectangles (from quad)
 * draw rectangles based on their unit vector
 * draw edges: straight lines or bezier curves
 * color nodes
 */

View view;

static void
flush(void)
{
	flushdraw_();
}

static int
drawedge(Edge *e, Vquad *r)
{
	USED(e);
	return drawline(r->u, r->v, e->w);
}

static int
drawnode(Vquad *r)
{
	dprint("drawnode %d,%d,%d,%d\n", r->u.x, r->u.y, r->v.x, r->v.y);
	return drawquad(r->u, r->v);
}

static int
drawedges(Graph *g)
{
	Vnode *v, *ve;
	Node *u;
	Edge *e, *ee;
	Vquad r;

	for(v=&kv_A(g->r.nodes, 0), ve=v+kv_size(g->r.nodes); v<ve; v++){
		warn("node %zd -- ", v->u - &kv_A(g->nodes, 0));
		u = v->u;
		// FIXME: :for some reason, loop does not terminate even if list is empty (variable is *not* zero, but some small integer
		warn("e %#p size %lud\n", kv_A(u->out, 0), kv_size(u->out));
		for(e=kv_A(u->out, 0), ee=e+kv_size(u->out); e<ee; e++){
			// FIXME: ugh
			warn("e %#p q->u %#p ", e, e->u);
			e->u->q.u;
			warn("q->v %#p\n", e->v);
			e->v->q.v;
			r = scaletrans((Vquad){e->u->q.u, e->v->q.u}, view.zoom, (Vertex){50,50});
			drawedge(e, &r);
		}
	}
	return 0;
}

static int
drawnodes(Graph *g)
{
	Vnode *v, *ve;
	Vquad r;

	USED(g);
	for(v=&kv_A(g->r.nodes, 0), ve=v+kv_size(g->r.nodes); v<ve; v++){
		r = scaletrans(v->q, view.zoom, (Vertex){50,50});
		drawnode(&r);
	}
	/*
	for(s=&kv_A(g->r.shapes, 0), se=s+kv_size(g->r.shapes); s<se; s++){
		// FIXME
		//r = scaletrans(s->r, view.zoom, view.pan);
		r = scaletrans(s->r, view.zoom, ZV);
		if((s->id & SHrect) == SHrect)
			drawnode(s->id >> 1,&r);
		else
			drawedge(s->id >> 1, &r);
	}
	*/
	return 0;
}

// FIXME: for all graphs, same for the others
static void
drawworld(Graph *g)
{
	drawnodes(g);
	drawedges(g);
}

static void
drawui(void)
{
}

static void
cleardraw(void)
{
	cleardraw_();
}

// FIXME: until there is a need to do drawing asynchronously,
// there's no need to distinguish draw and ui layers, it will
// all be redrawn anyway
int
updatedraw(Graph *g)
{
	USED(g);
	drawui();
	flush();
	return 0;
}

int
redraw(Graph *g)
{
	cleardraw();
	//centerdraw(g->r->dim);	// FIXME: center view, ie. default pan + zoom
	drawworld(g);
	return updatedraw(g);
}

int
resetdraw(Graph *g)
{
	resetdraw_();
	return redraw(g);
}

int
initvdraw(void)
{
	return initdraw_();
}
