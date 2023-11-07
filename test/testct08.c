#include "strpg.h"
#include "em.h"

int
main(int, char **)
{
	ssize r, i, v, w;
	EM *em;

	if((em = emopen(nil, 0)) == nil)
		sysfatal("emopen: %s", error());
	for(i=0; i<10000000; i++){
		r = lrand() % (((1ULL<<32)-1)/8);
		v = r | r<<32ULL;
		emw64(em, r, v);
		if((w = emr64(em, r)) != v)
			sysfatal("emr64: %zx not %zx %s", v, w, error());
	}
	emclose(em);
	sysquit();
	return 0;
}
