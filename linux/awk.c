#include "strpg.h"
#include "cmd.h"
#include "threads.h"

Channel *cmdc;

static int epfd[2], fucker[2];	/* children's pipes, mandrake */

static void
cproc(void)
{
	close(epfd[1]);
	close(fucker[0]);
	dup(epfd[0], STDIN_FILENO);
	dup(fucker[1], STDOUT_FILENO);
	execl("/usr/bin/env", "env", "-S", "strawk", "-f", "/tmp/main.awk", nil);
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

static void
readcproc(void *)
{
	int n, fd;
	char buf[8192], *p, *s;

	fd = fucker[0];
	for(;;){
		if((n = read(fd, buf, sizeof buf)) <= 0)
			break;
		if(n == sizeof buf)	// FIXME: unhandled
			n--;
		buf[n] = 0;
		DPRINT(Debugcmd, "← cproc:[%d][%s]", n, buf);
		for(s=p=buf; p<buf+n; s=++p){
			if((p = strchr(p, '\n')) == nil)
				break;
			/* reader must free */
			*p = 0;
			sendp(cmdc, estrdup(s));
		}
		if(s < buf + n)
			sendp(cmdc, estrdup(s));
	}
	close(fd);
	if(n < 0)
		warn("readcproc: %s", error());
}

int
initrepl(void)
{
	int r;

	if(pipe(epfd) < 0 || pipe(fucker) < 0)
		return -1;
	if((cmdc = chancreate(sizeof(void*), 16)) == nil)
		return -1;
	r = fork();
	switch(r){
	case -1: return -1;
	case 0: cproc(); sysfatal("execl: %s", error());
	default:
		close(epfd[0]);
		close(fucker[1]);
		break;
	}
	newthread(readcproc, nil, nil, nil, "cmd", mainstacksize);
	return 0;
}
