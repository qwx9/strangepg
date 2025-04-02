#include "strpg.h"
#include "fs.h"
#include "em.h"

/* write and reread single value past eof */
int
main(int argc, char **argv)
{
	EM em;

	if(argc > 1)
		multiplier = atoi(argv[1]);
	initem();
	if((em = emopen(nil)) < 0)
		sysfatal("emopen: %s", error());
	if(emr64(em, 1) != 0)
		sysfatal("emr64: not eof: %s", error());
	emw64(em, 1, 0xdeadbeefcafebabeULL);
	if(emr64(em, 1) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %s", error());
	emclose(em);
	return 0;
}
