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
drawbezier(void)
{
	return 0;
}
