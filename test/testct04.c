#include "strpg.h"
#include "fs.h"
#include "em.h"

void
threadmain(int, char**)
{
	EM *em;

	debug = Debugextmem;
	em = emnew(0);
	if(empget64(em, 1) != EMbupkis)
		sysfatal("empget64: [0] not eof: %s", error());
	// FIXME: off hasn't changed from 1*8
	if(empget64(em, 0) != EMbupkis)
		sysfatal("emget64: [1] not eof: %s", error());
	if(emget64(em) != EMbupkis)
		sysfatal("emget64: [2] not eof: %s", error());
	if(empget64(em, 4) != EMbupkis)
		sysfatal("empget64: [3] not eof: %s", error());
	if(empget64(em, 5) != EMbupkis)
		sysfatal("empget64: [4] not eof: %s", error());
	if(empput64(em, 1, 1) < 0)
		sysfatal("emput64: [5] %s", error());
	if(empput64(em, 10, 10) < 0)
		sysfatal("empput64: [6] %s", error());
	if(empget64(em, 1) != 1)
		sysfatal("empget64: [7] %s", error());
	if(emget64(em) != EMbupkis)
		sysfatal("emget64: [8] not eof: %s", error());
	if(emget64(em) != EMbupkis)
		sysfatal("emget64: [9] not eof: %s", error());
	if(empget64(em, 4) != EMbupkis)
		sysfatal("empget64: [10] not eof: %s", error());
	if(empget64(em, 5) != EMbupkis)
		sysfatal("empget64: [11]not eof: %s", error());
	if(empget64(em, 1) != 1)
		sysfatal("empget64: [12] %s", error());
	if(empget64(em, 10) != 10)
		sysfatal("empget64: [13] %s", error());
	emclose(em);
	sysquit();
}
