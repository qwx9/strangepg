#include "strpg.h"
#include "em.h"

int
main(int, char **)
{
	ssize r, i, v, w;
	EM *em;

	em = emopen(nil, 0);
	for(i=0; i<10000000; i++){
		r = lrand() % ((1<<16)-1);
		v = r | r<<16 | r<<32ULL | r<<48ULL;
		emw64(em, r, v);
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
	}
	emclose(em);
	sysquit();
	return 0;
}
