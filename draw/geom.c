#include "strpg.h"
#include "drw.h"

Quad
scaletrans(Quad q, double zoom, Vertex pan)
{
	return Qd(addpt2(mulpt2(q.o, zoom), pan), mulpt2(q.v, zoom));
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
