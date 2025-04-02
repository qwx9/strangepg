#include "strpg.h"
#include "em.h"

/* write first then reread with multiple buffers */
int
main(int argc, char **argv)
{
	ssize i, w, f;
	EM em[8];

	if(argc > 1)
		multiplier = atoi(argv[1]);
	if(argc > 2)
		debug |= Debugem;
	initem();
	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil)) < 0)
			sysfatal("emopen: %s", error());
	for(i=0; i<50000000; i++){
		f = i % nelem(em);
		emw64(em[f], i, i);
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	for(i=0; i<50000000; i++){
		f = i % nelem(em);
		if((w = emr64(em[f], i)) != i)
			sysfatal("emr64: %zx not %zx", w, i);
		if(i % 100000 == 0 && i > 0)
			warn("%.3g iterations...\n", (double)i);
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	return 0;
}
