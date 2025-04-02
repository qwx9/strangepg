#include "strpg.h"
#include "em.h"

/* write and reread single value */
int
main(int argc, char **argv)
{
	ssize v;
	EM em;

	if(argc > 1)
		multiplier = atoi(argv[1]);
	initem();
	if((em = emopen(nil)) < 0)
		sysfatal("emopen: %s", error());
	emw64(em, 0, 0xdeadbeefcafebabeULL);
	if((v = emr64(em, 0)) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %zx not %zx", v, 0xdeadbeefcafebabeULL);
	emclose(em);
	return 0;
}
