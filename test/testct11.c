#include "strpg.h"
#include "em.h"

enum{
	Nf = 10,
	Niter = 5000,
};

int
main(int, char **)
{
	ssize i, w, f;
	EM *em[Nf];

	initem();
	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil)) == nil)
			sysfatal("emopen: %s", error());
	for(i=0; i<Niter; i++){
		if((i+1) % (1<<16) == 0)
			warn("w64[%zd]\n", i);
		f = i % nelem(em);
		emw64(em[f], i, i);
	}
	for(i=0; i<Niter; i++){
		if((i+1) % (1<<16) == 0)
			warn("r64[%zd]\n", i);
		f = i % nelem(em);
		if((w = emr64(em[f], i)) != i){
			warn("emr64: %zx not %zx\n", w, i);
			abort();
		}
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	sysquit();
	return 0;
}
