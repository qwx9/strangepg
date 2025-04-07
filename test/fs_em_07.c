#include "strpg.h"
#include "em.h"

/* random io within one block */
int
main(int argc, char **argv)
{
	ssize i, v, w;
	u64int r;
	EM em;

	if(argc > 1)
		multiplier = atoi(argv[1]);
	if(argc > 2)
		debug |= Debugem;
	initem();
	xsrand(42);
	if((em = emopen(nil)) < 0)
		sysfatal("emopen: %s", error());
	for(i=0; i<50000000; i++){
		r = xlrand() % ((1<<16)-1);
		v = r | r<<16 | r<<32 | r<<48;
		emw64(em, r, v);
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	emclose(em);
	return 0;
}
