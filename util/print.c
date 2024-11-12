#include "strpg.h"
#include "threads.h"

char logbuf[8192], lastmsg[3][64], iserrmsg[3];
int nlog, logsz;

static char *lp = logbuf;
static RWLock llock;

static inline void
writelog(char *s, int iserr)
{
	int n, m;
	char *p;

	n = strlen(s);
	if(n > sizeof logbuf)
		n = sizeof logbuf - 1;
	while(lp + n >= logbuf + sizeof logbuf - 1){
		if((p = strchr(logbuf, '\n')) == nil || p >= lp){
			logbuf[0] = 0;
			lp = logbuf;
		}else{
			p++;
			m = lp - p + 1;
			memmove(logbuf, p, m);
			lp -= p - logbuf;
		}
	}
	memcpy(lp, s, n);
	lp += n - 1;
	*lp++ = '\n';
	*lp = 0;
	logsz = lp - logbuf;
	strncpy(lastmsg[0], lastmsg[1], sizeof lastmsg[0]-1);
	strncpy(lastmsg[1], lastmsg[2], sizeof lastmsg[1]-1);
	strncpy(lastmsg[2], s, sizeof lastmsg[2]-1);
	p = lastmsg[2] + MIN(n, sizeof lastmsg[2]-1) - 1;
	if(*p == '\n')
		*p = 0;
	iserrmsg[0] = iserrmsg[1];
	iserrmsg[1] = iserrmsg[2];
	iserrmsg[2] = iserr;
	nlog++;
}

void
logmsg(char *s)
{
	wlock(&llock);
	writelog(s, 0);
	wunlock(&llock);
}

void
logerr(char *s)
{
	wlock(&llock);
	writelog(s, 1);
	wunlock(&llock);
}

void
warn(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
}

inline void
dprint(int flags, char *fmt, ...)
{
	char *type;
	va_list arg;

	switch(debug & flags){
	case 0: return;
	case Debugawk: type = "awk"; break;
	case Debugcmd: type = "cmd"; break;
	case Debugcoarse: type = "coarse"; break;
	case Debugdraw: type = "draw"; break;
	case Debugextmem: type = "extmem"; break;
	case Debugfs: type = "fs"; break;
	case Debuglayout: type = "layout"; break;
	case Debugmeta: type = "meta"; break;
	case Debugperf: type = "perf"; break;
	case Debugrender: type = "render"; break;
	default: type = "dafuq"; break;
	}
	va_start(arg, fmt);
	vadebug(type, fmt, arg);
}
