#include "strpg.h"
#include "strawk/mt19937-64.h"

void
xsrand(u64int s)
{
	init_genrand64(s);
}

float
xfrand(void)
{
	return genrand64_real2();	/* [0,1) */
}

u32int
xlrand(void)
{
	return genrand64_int63() & (1ULL<<32)-1;
}

u32int
xnrand(u32int lim)
{
	return genrand64_int64() % lim;
}

void
initrand(void)
{
	xsrand(time(nil));
}
