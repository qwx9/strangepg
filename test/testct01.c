#include "strpg.h"
#include "em.h"

int
main(int argc, char **argv)
{
	EM *em;

	if((em = emopen(nil, 0)) == nil)
		sysfatal("emopen: %s", error());
	emw64(em, 0, 0xdeadbeefcafebabeULL);
	if(emr64(em, 0) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %s", error());
	emclose(em);
	sysquit();
}
