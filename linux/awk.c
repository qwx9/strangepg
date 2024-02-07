#include "strpg.h"
#include "cmd.h"
#include "chan.h"
#include <pthread.h>

chan_t *cmdc;
int stupidfd[2];	/* children's pipes, mandrake */

void
sendcmd(char *cmd)
{
	int n;

	n = strlen(cmd);
	DPRINT(Debugcmd, "→ sendcmd:[%d][%s]", n, cmd);
	if(epfd[1] < 0)
		warn("sendcmd: closed pipe\n");
	else if(write(epfd[1], cmd, n) != n)
		sysfatal("sendcmd: %s", error());
}

static void *
readcproc(void *)
{
	int n, fd;
	char buf[8192], *p, *s;

	fd = stupidfd[0];
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
			chan_send_buf(cmdc, estrdup(s), strlen(s));
		}
		if(s < buf + n)
			chan_send_buf(cmdc, estrdup(s), strlen(s));
	}
	close(fd);
	if(n < 0)
		warn("readcproc: %s", error());
	return NULL;
}

int
startengine(void)
{
	int r;
	static pthread_t th;	// FIXME

	if(pipe(stupidfd) < 0)
		sysfatal("pipe: %s", error());
	if((cmdc = chan_init(16*sizeof(void*))) == nil)
		sysfatal("startengine: cmdcreate");
	r = fork();
	switch(r){
	case -1: sysfatal("fork: %s", error());
	case 0:
		close(epfd[1]);
		close(stupidfd[0]);
		dup2(epfd[0], STDIN_FILENO);
		dup2(stupidfd[1], STDOUT_FILENO);
		close(epfd[0]);
		close(stupidfd[1]);
		execl("/usr/bin/env", "env", "-S", "awk", "-S", "-f", "/tmp/main.awk", "-f", "-", NULL);
		sysfatal("execl: %s", error());
	default:
		close(epfd[0]);
		close(stupidfd[1]);
		break;
	}
	if((r = pthread_create(&th, NULL, readcproc, NULL)) != 0)
		sysfatal("startengine: pthread_create failed with err=%d", r);
	return 0;
}
