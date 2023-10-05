#include "strpg.h"
#include "fs.h"
#include "em.h"

void	printchain(Chunk *);

void
threadmain(int, char**)
{
	int i;
	int u[] = {0,9,1,8,6,2,4,3,7,5};
	EM *em;

	debug = Debugextmem;
	em = emnew();
	for(i=0; i<nelem(u); i++){
		warn("<-- [%d] %d\n", i, u[i]);
		if(empget64(em, u[i]) != EMeof)
			sysfatal("empget64: not eof: %s", error());
		printchain(&em->c);
	}
	for(i=0; i<nelem(u); i++){
		warn("--> [%d] %d\n", i, u[i]);
		if(empput64(em, u[i], u[i]) < 0)
			sysfatal("empget64: %s", error());
		printchain(&em->c);
	}
	for(i=0; i<nelem(u); i++){
		warn("<-- [%d] %d\n", i, u[i]);
		if(empget64(em, u[i]) != u[i])
			sysfatal("empget64: not tgt %d: %s", u[i], error());
		printchain(&em->c);
	}
	emclose(em, 0);
	sysquit();
}
