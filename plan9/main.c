#include "strpg.h"
#include <thread.h>
#include <pool.h>

mainstacksize = 16*1024;

void
threadmain(int argc, char **argv)
{
	//mainmem->flags |= POOL_NOREUSE | POOL_PARANOIA;
	srand(time(nil));
	if(parseargs(argc, argv) < 0)
		sysfatal("usage");
	run();
	threadexitsall(nil);
}
