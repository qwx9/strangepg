#include "strpg.h"

Vertex ZV;
Quad ZQ;

Quad
Qd(Vertex o, Vector v)
{
	return (Quad){o, v};
}

double
qΔx(Quad q)
{
	return q.v.x;
}

double
qΔy(Quad q)
{
	return q.v.y;
}

int
ptinquad(Vertex v, Quad q)
{
	return v.x >= q.o.x && v.x < q.o.x + q.v.x
		&& v.y >= q.o.y && v.y < q.o.y + q.v.y;
}

Quad
insetquad(Quad q, int Δ)
{
	return (Quad){
		Vec2(q.o.x - Δ, q.o.y - Δ),
		Vec2(q.v.x + Δ, q.v.y + Δ)
	};
}

Quad
quadaddpt2(Quad q, Vector v)
{
	return (Quad){addpt2(q.o, v), addpt2(q.v, v)};
}

Vertex
floorpt2(Vertex v)
{
	return Vec2(floor(v.x), floor(v.y));
}

int
eqpt2(Point2 a, Point2 b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w;
}
