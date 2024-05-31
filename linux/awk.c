#include "strpg.h"
#include "cmd.h"
#include "threads.h"

extern Channel *cmdc;

static int epfd[2], fucker[2];	/* children's pipes, mandrake */

static void
cproc(void)
{
	close(epfd[1]);
	close(fucker[0]);
	dup(epfd[0], STDIN_FILENO);
	dup(fucker[1], STDOUT_FILENO);
	execl("/usr/bin/env", "env", "-S", "strawk", awkprog, nil);
}

// FIXME: portable code
void
sendcmd(char *cmd)
{
	int n;

	n = strlen(cmd);
	DPRINT(Debugcmd, "→ sendcmd:[%d][%s]", n, cmd);
	if(epfd[1] < 0)
		warn("sendcmd: closed pipe\n");
	else if(write(epfd[1], cmd, n) != n){
		warn("sendcmd: %s", error());
		close(epfd[1]);
		epfd[1] = -1;
		close(fucker[0]);
		fucker[0] = -1;
	}
}

int
initrepl(void)
{
	int r;

	if(pipe(epfd) < 0 || pipe(fucker) < 0)
		return -1;
	if((cmdc = chancreate(sizeof(char*), 16)) == nil)
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
