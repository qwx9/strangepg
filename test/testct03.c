#include "strpg.h"
#include "fs.h"
#include "em.h"

void
threadmain(int, char**)
{
	EM *em;

	em = emnew();
	if(empget64(em, 1) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empput64(em, 1, 0xdeadbeefcafebabeULL) < 0)
		sysfatal("emput64: %s", error());
	if(empget64(em, 1) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	emclose(em, 0);
	sysquit();
}
