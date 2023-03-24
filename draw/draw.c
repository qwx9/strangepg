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
	return drawquad(id, r->u, r->v);
}

static int
drawshapes(Graph *g, Render *rd)
{
	Shape *s, *se;
	Vquad r;

	USED(g);
	for(s=&kv_A(rd->shapes, 0), se=s+kv_size(rd->shapes); s<se; s++){
		r = scaletrans(s->r, view.zoom, view.pan);
		if((s->id & SHrect) == SHrect)
			drawnode(s->id >> 1, &r);
		else
			drawedge(s->id >> 1, &r);
	}
	return 0;
}

// FIXME: for all graphs, same for the others
static void
drawworld(Graph *g, Render *r)
{
	drawshapes(g, r);
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
