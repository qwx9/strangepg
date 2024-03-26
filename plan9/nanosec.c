#include "strpg.h"
#include <tos.h>

/*
 * nsec() is wallclock and can be adjusted by timesync
 * so need to use cycles() instead, but fall back to
 * nsec() in case we can't
 */
u64int
nanosec(void)
{
	static u64int fasthz, xstart;
	u64int x, div;

	if(fasthz == ~0ULL)
		return nsec() - xstart;

	if(fasthz == 0){
		if(_tos->cyclefreq){
			fasthz = _tos->cyclefreq;
			cycles(&xstart);
		} else {
			fasthz = ~0ULL;
			xstart = nsec();
		}
		return 0;
	}
	cycles(&x);
	x -= xstart;

	/* this is ugly */
	for(div = 1000000000ULL; x < 0x1999999999999999ULL && div > 1 ; div /= 10ULL, x *= 10ULL);

	return x / (fasthz / div);
}
