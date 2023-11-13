#include "strpg.h"
#include "em.h"

int
main(int, char **)
{
	ssize r, i, v, w, f;
	EM *em[10];

	for(i=0; i<nelem(em); i++)
		if((em[i] = emopen(nil, 0)) == nil)
			sysfatal("emopen: %s", error());
	for(i=0; i<10000000; i++){
		f = nrand(nelem(em));
		r = lrand() % (((1ULL<<32)-1)/4);
		v = r | r<<32ULL;
		emw64(em[f], r, v);
		if((w = emr64(em[f], r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
	}
	for(i=0; i<nelem(em); i++)
		emclose(em[i]);
	sysquit();
	return 0;
}
