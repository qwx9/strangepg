#include "strpg.h"
#include <draw.h>
#include "drw.h"

Quad
scaletrans(Quad r, double zoom, Vertex pan)
{
	r.u = addvx(mulvx(r.u, zoom), pan);
	r.v = addvx(mulvx(r.v, zoom), pan);
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
