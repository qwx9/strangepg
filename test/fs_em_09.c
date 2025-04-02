#include "strpg.h"
#include "fs.h"
#include "em_new.h"

/* random io on multiple buffers within one bank's worth of pages,
 * rotating buffers */
int
main(int argc, char **argv)
{
	int e;
	ssize i, v, w;
	u64int r;
	EM em[8];

	if(argc > 1)
		multiplier = atoi(argv[1]);
	if(argc > 2)
		debug |= Debugem;
	initem();
	srand(42);
	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil)) < 0)
			sysfatal("emopen: %s", error());
	for(i=0; i<50000000; i++){
		e = em[i & nelem(em)-1];
		r = xlrand() & ((1ULL<<32>>3>>3)-1);
		v = r | r<<32;
		emw64(e, r, v);
		if((w = emr64(e, r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	return 0;
}
