#include "strpg.h"
#include "threads.h"
#include "cmd.h"

static int fucker[2];	/* children's pipes, mandrake */

static void
cproc(void)
{
	close(epfd[1]);
	close(fucker[0]);
	dup(epfd[0], STDIN_FILENO);
	dup(fucker[1], STDOUT_FILENO);
	execl("/usr/bin/env", "env", "-S", "strawk", awkprog, nil);
}

int
initrepl(void)
{
	int r;

	if(pipe(epfd) < 0 || pipe(fucker) < 0)
		return -1;
	r = fork();
	switch(r){
	case -1: return -1;
	case 0:
		cproc();
		sysfatal("execl: %s", error());
		break;
	default:
		close(epfd[0]);
		close(fucker[1]);
		break;
	}
	return fucker[0];
}
