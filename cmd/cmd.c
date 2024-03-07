#include "strpg.h"
#include "cmd.h"

int epfd[2] = {-1, -1};

/* FIXME: per-graph state */

void
parseresponse(char *s)
{
	warn("> %s\n", s);
}

void
pushcmd(char *fmt, ...)
{
	ssize ai;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	DPRINT(Debugcmd, "pushcmd %c", fmt[0]);
	if(epfd[1] < 0){
		warn("pushcmd: pipe closed\n");
		return;
	}
	va_start(arg, fmt);
	for(f=fmt, sp=sb; sp<sb+sizeof sb-1;){
		if((c = *f++) == 0)
			break;
		else if(c != '%'){
			*sp++ = c;
			continue;
		}
		if((c = *f++) == 0){
			warn("pushcmd: malformed command %s\n", fmt);
			break;
		}
		switch(c){
		case 's':
			as = va_arg(arg, char*);
			sp = strecpy(sp, sb+sizeof sb-1, as);
			break;
		case 'd':
			ai = va_arg(arg, ssize);
			sp = seprint(sp, sb+sizeof sb-1, "%zd", ai);
			break;
		case 'x':
			ai = va_arg(arg, ssize);
			sp = seprint(sp, sb+sizeof sb-1, "%08zx", ai);
			break;
		default:
			warn("pushcmd: unknown format %c\n", c);
			*sp++ = c;
			break;
		}
	}
	va_end(arg);
	if(sp < sb + sizeof sb - 1)
		*sp++ = '\n';
	*sp = 0;
	sendcmd(sb);
}
