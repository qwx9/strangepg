#include "strpg.h"
#include "em.h"

int
main(int argc, char **argv)
{
	ssize r, i, v, w;
	EM em;

	if(argc > 1)
		multiplier = atoi(argv[1]);
	if(argc > 2)
		debug |= Debugem;
	initem();
	if((em = emopen(nil)) < 0)
		sysfatal("emopen: %s", error());
	srand(42);
	for(i=0; i<50000000; i++){
		r = xlrand() & ((1ULL<<32>>3)-1);
		v = r | r<<32;
		emw64(em, r, v);
		if(i % 100000 == 0 && i > 0)
			warn("%.3g write iterations...\n", (double)i);
	}
	srand(42);
	for(i=0; i<50000000; i++){
		r = xlrand() & ((1ULL<<32>>3)-1);
		v = r | r<<32;
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: [%zx] read %zx, expected %zx", i, w, v);
		if(i % 100000 == 0 && i > 0)
			warn("%.3g read iterations...\n", (double)i);
	}
	emclose(em);
	return 0;
}
