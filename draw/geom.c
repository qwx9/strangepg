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

int
drawline(Quad r, double w)
{
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
