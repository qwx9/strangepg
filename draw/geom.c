#include "strpg.h"
#include "drawprv.h"

Quad
scaletrans(Quad r, double zoom, Vertex pan)
{
	r.u = addvx(scalevx(r.u, zoom), pan);
	r.v = addvx(scalevx(r.v, zoom), pan);
	return r;
}

int
rotate(Quad)
{
	return 0;
}

/* top-left to center (kludge) */
static Quad
centernode(Quad r)
{
	int d;

	d = MIN(-(r.v.x - r.u.x), -(r.v.y - r.u.y));
	//r = quadaddvx(r, Vx(d, d));
	warn("%d,%d %d,%d\n", r.u.x, r.u.y, r.v.x, r.v.y);
	return r;
}

int
drawline(Quad r, double w)
{
	r = centernode(r);
	return drawline_(r, w);
}

int
drawquad(Quad r)
{
	return drawquad_(r);
}

int
drawbezier(void)
{
	return 0;
}
