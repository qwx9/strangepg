#include "strpg.h"
#include "threads.h"
#include "cmd.h"

static int fucker[2];	/* children's pipes, mandrake */

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
		close(epfd[1]);
		close(fucker[0]);
		dup(epfd[0], STDIN_FILENO);
		dup(fucker[1], STDOUT_FILENO);
		dup(fucker[1], STDERR_FILENO);
		awk(awkprog);
		exit(0);
	default:
		close(epfd[0]);
		close(fucker[1]);
		break;
	}
	return fucker[0];
}
