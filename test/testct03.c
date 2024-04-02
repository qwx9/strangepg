#include "strpg.h"
#include "fs.h"
#include "em.h"

int
main(int, char **)
{
	EM *em;

	initem();
	if((em = emopen(nil)) == nil)
		sysfatal("emopen: %s", error());
	if(emr64(em, 1) != 0)
		sysfatal("emr64: not eof: %s", error());
	emw64(em, 1, 0xdeadbeefcafebabeULL);
	if(emr64(em, 1) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %s", error());
	emclose(em);
	sysquit();
	return 0;
}
