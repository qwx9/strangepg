#include "strpg.h"
#include "cmd.h"
#include <thread.h>
#include <bio.h>

Channel *cmdc;

static void
cproc(void *)
{
	close(epfd[1]);
	dup(epfd[0], 0);
	dup(epfd[0], 1);
	close(epfd[0]);
	procexecl(nil, "/bin/awk", "awk", "-safe", "-f", "/tmp/main.awk", nil);
	sysfatal("procexecl: %r");
}

static void
readcproc(void *)
{
	int n;
	char buf[8192], *p, *s;

	for(;;){
		if((n = read(epfd[1], buf, sizeof buf)) <= 0)
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
	close(epfd[1]);
	if(n < 0)
		warn("readcproc: %r");
}

void
sendcmd(char *cmd)
{
	int n;

	n = strlen(cmd);
	DPRINT(Debugcmd, "→ sendcmd:[%d][%s]", n, cmd);
	if(epfd[1] < 0)
		warn("sendcmd: closed pipe\n");
	else if(write(epfd[1], cmd, n) != n)
		sysfatal("sendcmd: %r");
}

int
startengine(void)
{
	if((cmdc = chancreate(sizeof(void*), 16)) == nil)
		return -1;
	if(procrfork(cproc, nil, mainstacksize, RFNAMEG|RFENVG|RFFDG|RFNOMNT) < 0
	|| proccreate(readcproc, nil, mainstacksize) < 0)
		return -1;
	return 0;
}
