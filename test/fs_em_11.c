#include "strpg.h"
#include "em.h"

/* random io on multiple buffers within one bank's worth of pages,
 * with random buffer */
int
main(int argc, char **argv)
{
	ssize r, i, v, w, f;
	EM em[16];

	if(argc > 1)
		multiplier = atoi(argv[1]);
	if(argc > 2)
		debug |= Debugem;
	initem();
	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil)) < 0)
			sysfatal("emopen: %s", error());
	for(i=0; i<50000000; i++){
		f = xnrand(nelem(em));
		r = xlrand() & ((1ULL<<32>>3>>4)-1);
		v = r | r<<32ULL;
		emw64(em[f], r, v);
		if((w = emr64(em[f], r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	return 0;
}
