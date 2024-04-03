#include "strpg.h"
#include "em.h"

enum{
	Niter = 10000,
	M = 29,
	R = 30,
};

int
main(int, char **)
{
	ssize r, i, v, w, s;
	EM *em;

	//debug |= Debugextmem;
	multiplier = M;
	initem();
	if((em = emopen(nil)) == nil)
		sysfatal("emopen: %s", error());
	s = time(nil);
	srand(42);
		vlong t = nsec();
	for(i=0; i<Niter; i++){
		if((i+1) % (1<<8) == 0)
			warn("w64[%zd]\n", i);
		r = lrand() % (((1ULL<<R)-1));
		v = r | r<<32ULL;
		emw64(em, r, v);
	}
	warn("writes: %lld msec\n", (nsec() - t) / 1000000);
	srand(42);
	t = nsec();
	for(i=0; i<Niter; i++){
		if((i+1) % (1<<8) == 0)
			warn("r64[%zd]\n", i);
		r = lrand() % (((1ULL<<R)-1));
		v = r | r<<32ULL;
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: [%zx] read %zx, expected %zx", i, w, v);
	}
	warn("reads: %lld msec\n", (nsec() - t) / 1000000);
	emclose(em);
	sysquit();
	return 0;
}
