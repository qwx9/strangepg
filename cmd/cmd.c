#include "strpg.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"

int epfd[2] = {-1, -1};

/* FIXME: multiple graphs: per-graph state? one pipe per graph?
 * how do we dispatch user queries? would be useful to do queries
 * on multiple ones! */
void
readcmd(char *s)
{
	int m;
	ssize x;
	char *fld[8], *p, *t;
	Node *n;
	Graph *g;

	switch(s[0]){
	case 'C': break;
	case 'E':
		warn("readcmd: %s\n", s+2);
		return;
	case 0:
		warn("readcmd: empty input\n");
		return;
	default:
		warn("> %s\n", s);
	}
	t = getfield(s);
	s++;
	while(s != nil && *s != 0){
		if((m = getfields(s, fld, nelem(fld), 1, "\t ")) < 1)
			goto error;
		USED(m);
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
		s = t;
		t = getfield(s);
	}
	return;
error:
	warn("readcmd: %s\n", error());
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
			warn("pushcmd: unknown format %c in <%s>\n", c, f);
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
