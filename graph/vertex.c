#include "strpg.h"

Vertex ZV;

// FIXME: better names (vx → vert? or just use Point/Rectangle?
// and reimplement/copy for linux?)
// FIXME: Vector type? Vec2D/Vec3D? Or just use rodri's geometry(2) api
//	and study its use for rotation etc
//  why not pull in this kind of code if it's simple enough
//	could contemplate even bundling a libdraw+libgeometry for linux

Vertex
Vx(int x, int y)
{
	return (Vertex){x, y};
}

int
eqvx(Vertex a, Vertex b)
{
	return memcmp(&a, &b, sizeof a) == 0;
}

int
dxvx(Quad r)
{
	return r.v.x - r.u.x;
}

int
dyvx(Quad r)
{
	return r.v.y - r.u.y;
}

Quad
vx2r(Vertex a, Vertex b)
{
	return (Quad){a, b};
}

Vertex
addvx(Vertex a, Vertex b)
{
	return (Vertex){a.x + b.x, a.y + b.y};
}

Vertex
subvx(Vertex a, Vertex b)
{
	return (Vertex){a.x - b.x, a.y - b.y};
}

Vertex
scalevx(Vertex a, float f)
{
	return (Vertex){a.x * f, a.y * f};
}

int
vxinquad(Vertex v, Quad q)
{
	return v.x >= q.u.x && v.x < q.v.x
		&& v.y >= q.u.y && v.y < q.v.y;
}

Quad
insetvx(Vertex v, int Δ)
{
	return (Quad){(Vertex){v.x - Δ, v.y - Δ}, (Vertex){v.x + Δ, v.y + Δ}};
}

Quad
quadaddvx(Quad q, Vertex v)
{
	q.u = addvx(q.u, v);
	q.v = addvx(q.v, v);
	return q;
}
