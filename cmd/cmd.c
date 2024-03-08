#include "strpg.h"
#include "cmd.h"
#include "drw.h"

int epfd[2] = {-1, -1};

/* FIXME: per-graph state */

void
parseresponse(char *s)
{
	int m;
	ssize x;
	char *fld[8], *p;
	Node *n;
	Graph *g;

	switch(s[0]){
	case 'C': break;
	case 'E':
		warn("parseresponse: %s\n", s+2);
		return;
	default:
		warn("> %s\n", s);
	}
	if((m = getfields(s+2, fld, nelem(fld), 1, "\t ")) < 1)
		goto error;
	USED(m);
	// FIXME: multiple graphs: one pipe per graph!
	g = graphs;
	switch(s[0]){
	case 'C':
		if((n = str2node(g, fld[0])) == nil)
			goto error;
		fprint(2, "%s %s\n", fld[0], fld[1]);
		x = strtoll(fld[1], &p, 0);
		if(p == fld[1]){
			werrstr("invalid color %s", fld[1]);
			goto error;
		}
		if(setnodecolor(g, n, x) < 0)
			goto error;
		reqdraw(Reqredraw);	// FIXME: rate limit, or batch parse
		break;
	}
	return;
error:
	warn("parseresponse: %s\n", error());
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
