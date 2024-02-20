#include "strpg.h"
#include "em.h"

int
main(int, char **)
{
	ssize v;
	EM *em;

	initem();
	if((em = emopen(nil, 0)) == nil)
		sysfatal("emopen: %s", error());
	emw64(em, 0, 0xdeadbeefcafebabeULL);
	if((v = emr64(em, 0)) != 0xdeadbeefcafebabeULL)
		sysfatal("emr64: %zx not %zx", v, 0xdeadbeefcafebabeULL);
	emclose(em);
	sysquit();
	return 0;
}
