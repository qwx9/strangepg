#include "strpg.h"
#include "fs.h"
#include "em.h"

void
threadmain(int, char**)
{
	int i;
	u64int m;
	int u[] = {0,9,1,8,6,2,4,3,7,5};
	EM *em;

	debug = Debugextmem;
	em = emnew(0);
	for(i=0; i<nelem(u); i++){
		warn("<-- [%d] %d\n", i, u[i]);
		if(empget64(em, u[i]) != EMbupkis)
			sysfatal("empget64: not eof: %s", error());
	}
	for(i=0; i<nelem(u); i++){
		warn("--> [%d] %d\n", i, u[i]);
		if(empput64(em, u[i], u[i]) < 0)
			sysfatal("empget64: %s", error());
	}
	for(i=0; i<nelem(u); i++){
		warn("<-- [%d] %d\n", i, u[i]);
		if((m = empget64(em, u[i])) != u[i])
			sysfatal("empget64: %llx not tgt %x: %s", m, u[i], error());
	}
	emclose(em);
	sysquit();
}
