#include "strpg.h"
#include "em.h"

int
main(int, char **)
{
	ssize i, w, f;
	EM *em[10];

	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil, 0)) == nil)
			sysfatal("emopen: %s", error());
	for(i=0; i<100000000; i++){
		f = i % nelem(em);
		emw64(em[f], i, i);
	}
	debug |= Debugtheworld;
	for(i=0; i<100000000; i++){
		f = i % nelem(em);
		if((w = emr64(em[f], i)) != i){
			warn("emr64: %zx not %zx: %s\n", w, i, error());
			abort();
		}
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	sysquit();
	return 0;
}