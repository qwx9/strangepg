#include "strpg.h"
#include "em.h"

void
threadmain(int, char**)
{
	EM *em;

	em = emnew(0);
	if(empput64(em, 0, 0xdeadbeefcafebabeULL) < 0)
		sysfatal("emput64: %s", error());
	if(empget64(em, 0) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	//assert(em->c.left == em->c.right && em->c.lleft == em->c.lright && em->c.left == &em->c);
	emclose(em);
	sysquit();
}