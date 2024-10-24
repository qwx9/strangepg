#include "strpg.h"

char logbuf[8192], *last[3];
int nlog, logsz;

void
logmsg(char *s)
{
	int n, m;
	char *p;
	static char *lp = logbuf;

	n = strlen(s);
	if(n > sizeof logbuf)
		n = sizeof logbuf - 1;
	while(lp + n >= logbuf + sizeof logbuf){
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
	lp += n;
	*lp = 0;
	logsz = lp - logbuf;
	free(last[0]);
	last[0] = last[1];
	last[1] = last[2];
	last[2] = estrdup(s);
	p = last[2] + n - 1;
	if(*p == '\n')
		*p = 0;
	nlog++;
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
	case Debugcmd: type = "cmd"; break;
	case Debugdraw: type = "draw"; break;
	case Debugrender: type = "render"; break;
	case Debuglayout: type = "layout"; break;
	case Debugfs: type = "fs"; break;
	case Debugcoarse: type = "coarse"; break;
	case Debugextmem: type = "extmem"; break;
	case Debugperf: type = "perf"; break;
	default: type = "dafuq"; break;
	}
	va_start(arg, fmt);
	vadebug(type, fmt, arg);
}
