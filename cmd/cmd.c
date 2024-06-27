#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "threads.h"
#include "cmd.h"

int noreset;
int epfd[2] = {-1, -1};
Channel *cmdc;

void
sendcmd(char *cmd)
{
	int n;

	if(epfd[1] < 0){
		warn("pushcmd: pipe closed\n");
		return;
	}
	n = strlen(cmd);
	DPRINT(Debugcmd, "→ sendcmd:[%d][%s]", n, cmd);
	if(epfd[1] < 0)
		warn("sendcmd: closed pipe\n");
	else if(write(epfd[1], cmd, n) != n){
		warn("sendcmd: %s", error());
		close(epfd[1]);
		epfd[1] = -1;
		sysfatal("sendcmd: %s", error());
	}
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

/* FIXME: multiple graphs: per-graph state? one pipe per graph?
 * how do we dispatch user queries? would be useful to do queries
 * on multiple ones! */
void
readcmd(char *s)
{
	int m, redraw;
	float x, y;
	ssize v;
	char *fld[8], *p, *t;
	Node *n;
	Graph *g;

	redraw = 0;
	t = nextfield(nil, s, nil, '\n');
	while(s != nil){
		/* FIXME: mootigraph */
		g = graphs;
		switch(*s){
		case 0:
			return;
		case 'E': 
			warn("Error:%s\n", s+1);
			goto next;
		case 'R':
			resetlayout(g);
			goto next;
		case 'c':
		case 'i':
		case 'f':
		case 'o':
		case 'x':
			break;
		default:
			warn("reply: <%s>\n", s);
			goto next;
		}
		if((m = getfields(s+1, fld, nelem(fld), 1, "\t ")) < 1)
			goto error;
		switch(s[0]){
		error:
			warn("readcmd: %s\n", error());
			break;
		case 'f':
			if(m != 1){
				werrstr("invalid f message length %d\n", m);
				goto error;
			}
			if(loadfs(fld[0], FFcsv) < 0)
				warn("readcmd: csv %s: %s\n", fld[0], error());
			break;
		case 'i':
			if(m != 1){
				werrstr("invalid o message length %d\n", m);
				goto error;
			}
			if(importlayout(g, fld[0]) < 0)
				warn("readcmd: importlayout from %s: %s\n", fld[0], error());
			break;
		case 'o':
			if(m != 1){
				werrstr("invalid o message length %d\n", m);
				goto error;
			}
			if(exportlayout(g, fld[0]) < 0)
				warn("readcmd: exportlayout to %s: %s\n", fld[0], error());
			break;
		case 'X':
			if(m != 3){
				werrstr("invalid X message length %d\n", m);
				goto error;
			}
			if((n = str2node(g, fld[0])) == nil)
				goto error;
			x = strtod(fld[1], &p);
			if(p == fld[1]){
				werrstr("invalid coordinate %s", fld[1]);
				goto error;
			}
			y = strtod(fld[2], &p);
			if(p == fld[2]){
				werrstr("invalid coordinate %s", fld[2]);
				goto error;
			}
			n->flags |= FNfixed;
			n->pos0.x = x;
			n->pos0.y = y;
			break;
		case 'x':
			if(m != 2){
				werrstr("invalid x message length %d\n", m);
				goto error;
			}
			if((n = str2node(g, fld[0])) == nil)
				goto error;
			x = strtod(fld[1], &p);
			if(p == fld[1]){
				werrstr("invalid coordinate %s", fld[1]);
				goto error;
			}
			n->flags |= FNinitpos;
			n->pos0.x = x;
			break;
		case 'c':
			if(m != 2){
				werrstr("invalid c message length %d\n", m);
				goto error;
			}
			if((n = str2node(g, fld[0])) == nil)
				goto error;
			v = strtoll(fld[1], &p, 0);
			if(p == fld[1]){
				werrstr("invalid color %s", fld[1]);
				goto error;
			}
			n->col = color(v);
			redraw = 1;
			break;
		}
	next:
		s = t;
		t = nextfield(nil, s, nil, '\n');
	}
	if(redraw)
		reqdraw(Reqredraw);
}

void
pollcmd(void)
{
	char *s;

	while((s = nbrecvp(cmdc)) != nil){
		readcmd(s);
		free(s);
	}
}

static void
readcproc(void *fd)
{
	int n;
	char *s;
	File *f;

	f = emalloc(sizeof *f);
	if(fdopenfs(f, (intptr)fd, OREAD) < 0)
		sysfatal("readcproc: %s", error());
	while((s = readline(f, &n)) != nil){
		DPRINT(Debugcmd, "← cproc:[%d][%s]", n, s);
		if(f->trunc){
			warn("readcproc: discarding abnormally long awk line");
			continue;
		}
		/* reader must free */
		sendp(cmdc, estrdup(s));
	}
	freefs(f);
}

int
initcmd(void)
{
	int fd;

	if((fd = initrepl()) < 0)
		sysfatal("initcmd: %s", error());
	newthread(readcproc, nil, (void*)(intptr)fd, nil, "readawk", mainstacksize);
	return 0;
}
