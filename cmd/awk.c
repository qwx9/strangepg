#include "strpg.h"
#include "cmd.h"

int
initcmd(void)
{
	if(pipe(epfd) < 0)
		sysfatal("initcmd: %s", error());
	if(startengine() < 0)
		sysfatal("initcmd: %s", error());
	return 0;
}
