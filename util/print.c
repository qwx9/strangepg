#include "strpg.h"
#include "threads.h"

int debug;
int onscreen;
char logbuf[8192], lastmsg[3][64], iserrmsg[3];
int nlog, logsz;

static char *lp = logbuf;
static QLock llock;

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
	strcpy(lastmsg[0], lastmsg[1]);
	strcpy(lastmsg[1], lastmsg[2]);
	strncpy(lastmsg[2], s, sizeof lastmsg[2]-1);
	p = lastmsg[2] + MIN(n, sizeof lastmsg[2]-1) - 1;
	if(*p == '\n')
		*p = 0;
	iserrmsg[0] = iserrmsg[1];
	iserrmsg[1] = iserrmsg[2];
	iserrmsg[2] = iserr;
	nlog++;
}

/* FIXME: va()-less: most of these will only write up to one line of text;
 * doesn't make sense to do more, survey usage */
void
logmsg(char *s)
{
	qlock(&llock);
	writelog(s, 0);
	if(!onscreen || (status & FSquiet) == 0)
		warn("%s", s);
	qunlock(&llock);
}

void
logerr(char *s)
{
	qlock(&llock);
	writelog(s, 1);
	warn("%s", s);
	qunlock(&llock);
}

void
warn(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
}

void
panic(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
	warn("\n");
	abort();
}

static inline char *
dtype(int flags)
{
	char *type;

	switch(debug & flags){
	case 0: return nil;
	case Debuginfo: type = "warning"; break;
	case Debugawk: type = "awk"; break;
	case Debugcmd: type = "cmd"; break;
	case Debugcoarse: type = "coarse"; break;
	case Debugdraw: type = "draw"; break;
	case Debugem: type = "em"; break;
	case Debugfs: type = "fs"; break;
	case Debuggraph: type = "graph"; break;
	case Debuglayout: type = "layout"; break;
	case Debugload: type = "load"; break;
	case Debugmeta: type = "meta"; break;
	case Debugperf: type = "perf"; break;
	case Debugrender: type = "render"; break;
	case Debugstrawk: type = "strawk"; break;
	case Debugui: type = "ui"; break;
	default: type = "dafuq"; break;
	}
	return type;
}

void
dprint(int flags, char *fmt, ...)
{
	char *type;
	va_list arg;

	if((type = dtype(flags)) == nil)
		return;
	va_start(arg, fmt);
	vadebug(type, fmt, arg);
}

void
dprintnl(int flags, char *fmt, ...)
{
	char *type;
	va_list arg;

	if((type = dtype(flags)) == nil)
		return;
	va_start(arg, fmt);
	vadebug(type, fmt, arg);
	warn("\n");
}

void
initlog(void)
{
	initqlock(&llock);
}
