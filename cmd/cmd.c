#include "strpg.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"

/* FIXME: multiple graphs: per-graph state? one pipe per graph?
 * how do we dispatch user queries? would be useful to do queries
 * on multiple ones! */
void
readcmd(char *s)
{
	int m, redraw;
	ssize x;
	char *fld[8], *p, *t;
	Node *n;
	Graph *g;

	redraw = 0;
	t = nextfield(nil, s, nil);
	while(s != nil){
		switch(*s){
		case 0:
			return;
		case 'E': 
			warn("Error:%s\n", s+1);
			goto next;
		case 'C':
			break;
		default:
			warn("readcmd: unhandled reply <%s>\n", s);
			goto next;
		}
		if((m = getfields(s+1, fld, nelem(fld), 1, "\t ")) < 1)
			goto error;
		g = graphs;	/* FIXME */
		switch(s[0]){
		error:
			warn("readcmd: %s\n", error());
			break;
		case 'C':
			if(m != 2){
				werrstr("invalid C message length %d\n", m);
				goto error;
			}
			if((n = str2node(g, fld[0])) == nil)
				goto error;
			x = strtoll(fld[1], &p, 0);
			if(p == fld[1]){
				werrstr("invalid color %s", fld[1]);
				goto error;
			}
			n->col = color(x);
			redraw = 1;
			break;
		}
	next:
		s = t;
		t = nextfield(nil, s, nil);
	}
	if(redraw)
		reqdraw(Reqredraw);
}

void
pushcmd(char *fmt, ...)
{
	ssize ai;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	DPRINT(Debugcmd, "pushcmd %c", fmt[0]);
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

int
initcmd(void)
{
	if(initrepl() < 0)
		sysfatal("initcmd: %s", error());
	return 0;
}
