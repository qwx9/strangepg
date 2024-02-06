#include "strpg.h"
#include "cmd.h"

int
initcmd(void)
{
	if(pipe(epfd) < 0)
		sysfatal("initcmd: %s", error());
	warn("%d %d\n", epfd[0], epfd[1]);
	if(startengine() < 0)
		sysfatal("initcmd: %s", error());
	return 0;
}
