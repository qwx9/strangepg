#include "strpg.h"
#include "em.h"

/* random io beyond one bank size */
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
	for(i=0; i<10000000; i++){
		r = xlrand() % (((1ULL<<32)-1)/4);
		v = r | r<<32ULL;
		emw64(em, r, v);
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	emclose(em);
	return 0;
}
