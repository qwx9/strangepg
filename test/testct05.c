#include "strpg.h"
#include "fs.h"
#include "em.h"

int
main(int argc, char **argv)
{
	int i;
	u64int m;
	int u[] = {0,9,1,8,6,2,4,3,7,5};
	EM *em;

	if((em = emopen(nil, 0)) == nil)
		sysfatal("emopen: %s", error());
	for(i=0; i<nelem(u); i++)
		if(emr64(em, i) != 0)
			sysfatal("emr64: not eof: %s", error());
	for(i=0; i<nelem(u); i++)
		emw64(em, i, u[i]);
	for(i=0; i<nelem(u); i++)
		if((m = emr64(em, i)) != u[i])
			sysfatal("emr64: %llx not tgt %x: %s", m, u[i], error());
	emclose(em);
	sysquit();
}
