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
drawedge(usize id, Vquad *r)
{
	return drawline(id, r->u, r->v, 0.0);
}

static int
drawnode(usize id, Vquad *r)
{
	dprint("drawnode %llud %d,%d,%d,%d\n", id, r->u.x, r->u.y, r->v.x, r->v.y);
	return drawquad(id, r->u, r->v);
}

static int
drawedges(Graph *, Render *)
{
	// FIXME
	return 0;
}

static int
//drawshapes(Graph *g, Render *rd)
drawnodes(Graph *g, Render *rd)
{
	Vnode *v, *ve;
	Vquad r;

	USED(g);
	for(v=&kv_A(rd->nodes, 0), ve=v+kv_size(rd->nodes); v<ve; v++){
		r = scaletrans(v->q, view.zoom, (Vertex){50,50});
		drawnode(v->id >> 1, &r);
	}
	/*
	for(s=&kv_A(rd->shapes, 0), se=s+kv_size(rd->shapes); s<se; s++){
		// FIXME
		//r = scaletrans(s->r, view.zoom, view.pan);
		r = scaletrans(s->r, view.zoom, ZV);
		if((s->id & SHrect) == SHrect)
			drawnode(s->id >> 1, &r);
		else
			drawedge(s->id >> 1, &r);
	}
	*/
	return 0;
}

// FIXME: for all graphs, same for the others
static void
drawworld(Graph *g, Render *r)
{
	drawnodes(g, r);
	drawedges(g, r);
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
updatedraw(Graph *g, Render *r)
{
	USED(g, r);
	drawui();
	flush();
	return 0;
}

int
redraw(Graph *g, Render *r)
{
	cleardraw();
	//centerdraw(r->dim);	// FIXME: center view, ie. default pan + zoom
	drawworld(g, r);
	return updatedraw(g, r);
}

int
resetdraw(Graph *g, Render *r)
{
	resetdraw_();
	return redraw(g, r);
}

int
initvdraw(void)
{
	return initdraw_();
}
