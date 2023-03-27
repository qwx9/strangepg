#include "strpg.h"

Vertex ZV;

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
dxvx(Vquad r)
{
	return r.v.x - r.u.x;
}

int
dyvx(Vquad r)
{
	return r.v.y - r.u.y;
}

Vquad
vx2r(Vertex a, Vertex b)
{
	return (Vquad){a, b};
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

Vquad
insetvx(Vertex v, int Δ)
{
	return (Vquad){(Vertex){v.x - Δ, v.y - Δ}, (Vertex){v.x + Δ, v.y + Δ}};
}

Vquad
quadaddvx(Vquad q, Vertex v)
{
	q.u = addvx(q.u, v);
	q.v = addvx(q.v, v);
	return q;
}
