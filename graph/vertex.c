#include "strpg.h"

Vertex ZV;

int
eqvx(Vertex a, Vertex b)
{
	return memcmp(&a, &b, sizeof a);
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
	return (Vquad){(Vertex){v.x + Δ, v.y + Δ}, (Vertex){v.x - Δ, v.y - Δ}};
}
