#include "strpg.h"
#include "fs.h"
#include "em.h"

void
threadmain(int, char**)
{
	EM *em;

	debug = Debugextmem;
	em = emnew();
	if(empget64(em, 1) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	// FIXME: off hasn't changed from 1*8
	if(emget64(em) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(emget64(em) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empget64(em, 4) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empget64(em, 5) != EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empput64(em, 1, 0xdeadbeefcafebabeULL) < 0)
		sysfatal("emput64: %s", error());
	if(empput64(em, 10, 0xdeadbeefcafebabeULL) < 0)
		sysfatal("empput64: %s", error());
	if(empget64(em, 1) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	if(emget64(em) == EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(emget64(em) == EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empget64(em, 4) == EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empget64(em, 5) == EMeof)
		sysfatal("empget64: not eof: %s", error());
	if(empget64(em, 1) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	if(empget64(em, 10) != 0xdeadbeefcafebabeULL)
		sysfatal("empget64: %s", error());
	emclose(em, 0);
	sysquit();
}
