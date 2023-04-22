#include "strpg.h"

static int
rendernode(Node *u)
{
	// FIXME: orientation, length
	u->q.v = addpt2(u->q.v, Vec2(Nodesz, Nodesz));
	return 0;
}

static int
rendershapes(Graph *g)
{
	Node *u, *ue;
	Quad d;
	Vertex p;

	d = Qd(view.dim.o, addpt2(view.dim.o, view.dim.v));
	for(u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		dprint("render node %s\n", quadfmt(&u->q));
		u->q.v = ZV;
		rendernode(u);
		p = addpt2(u->q.o, u->q.v);
		if(p.x < d.o.x)
			d.o.x = p.x;
		else if(p.x > d.v.x)
			d.v.x = p.x;
		if(p.y < d.o.y)
			d.o.y = p.y;
		else if(p.y > d.v.y)
			d.v.y = p.y;
	}
	g->dim = Qd(d.o, subpt2(d.v, d.o));
	return 0;
}

int
render(Graph *g)
{
	if(g->nodes.len < 1){
		werrstr("empty graph");
		return -1;
	}
	return rendershapes(g);
}

void
initrend(void)
{
}
