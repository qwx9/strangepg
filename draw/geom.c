#include "strpg.h"
#include "drawprv.h"

Vquad
scaletrans(Vquad r, double zoom, Vertex pan)
{
	r.u = addvx(scalevx(r.u, zoom), pan);
	r.v = addvx(scalevx(r.v, zoom), pan);
	return r;
}

int
rotate(Vquad)
{
	return 0;
}

int
drawline(usize, Vertex u, Vertex v, double w)
{
	return drawline_(u, v, w);
}

int
drawquad(usize, Vertex u, Vertex v)
{
	return drawquad_(u, v);
}

int
drawbezier(void)
{
	return 0;
}
