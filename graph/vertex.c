#include "strpg.h"

Vertex ZV;

int
eqv(Vertex a, Vertex b)
{
	return memcmp(&a, &b, sizeof a);
}
