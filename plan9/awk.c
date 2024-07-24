#include "strpg.h"
#include "cmd.h"
#include "threads.h"
#include <bio.h>

static void
cproc(void *)
{
	close(epfd[1]);
	dup(epfd[0], 0);
	dup(epfd[0], 1);
	close(epfd[0]);
	procexecl(nil, "/bin/strawk", "strawk", awkprog, nil);
	sysfatal("procexecl: %r");
}

int
initrepl(void)
{
	if(pipe(epfd) < 0)
		return -1;
	if(procrfork(cproc, nil, mainstacksize, RFNAMEG|RFENVG|RFFDG|RFNOMNT) < 0)
		return -1;
	return epfd[1];
}
