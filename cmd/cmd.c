#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

int epfd[2] = {-1, -1};

static void
sendcmd(char *cmd)
{
	int n;

	if(epfd[1] < 0)
		return;
	n = strlen(cmd);
	DPRINT(Debugcmd, "sendcmd: [%d][%s]", n, cmd);
	if(write(epfd[1], cmd, n) != n){
		DPRINT(Debugcmd, "sendcmd: %s", error());
		close(epfd[1]);
		epfd[1] = -1;
	}
}

void
pushcmd(char *fmt, ...)
{
	ioff ai;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	if(epfd[1] < 0)
		return;
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
			ai = va_arg(arg, ioff);
			sp = seprint(sp, sb+sizeof sb-1, "%d", ai);
			break;
		case 'x':
			ai = va_arg(arg, ioff);
			sp = seprint(sp, sb+sizeof sb-1, "%08x", ai);
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

static inline int
fnsetpos(Graph *g, char *sid, char *pos, int fix, int axis)
{
	ioff id;
	float c;
	char *p;
	Node *n;

	if((id = str2idx(sid)) < 0)
		return -1;
	n = g->nodes + id;
	c = strtod(pos, &p);
	if(p == pos){
		werrstr("invalid coordinate %s", pos);
		return -1;
	}
	if(axis == 0){
		n->flags |= fix ? FNfixedx | FNinitx : FNinitx;
		n->pos0.x = c;
	}else{
		n->flags |= fix ? FNfixedy | FNinity : FNinity;
		n->pos0.y = c;
	}
	return 0;
}

static inline int
fnsetcolor(char *sid, char *col)
{
	ioff id;
	char *p;
	u32int v;

	if((id = str2idx(sid)) < 0)
		return -1;
	v = strtol(col, &p, 0);
	if(p == col){
		werrstr("invalid color %s", col);
		return -1;
	}
	v = v << 8 | 0xb0;	/* FIXME: need a better way to handle this */
	setcolor(rnodes[id].col, v);
	return 0;
}

static inline int
fnfindnode(char *sid){
	ioff id;

	if((id = str2idx(sid)) < 0)
		return -1;
	focusnode(id);
	return 0;
}

/* FIXME: multiple graphs: per-graph state? one pipe per graph?
 * how do we dispatch user queries? would be useful to do queries
 * on multiple ones! */
void
readcmd(char *s)
{
	int m, req;
	char *fld[8], *t;
	Graph *g;

	req = 0;
	t = nextfield(nil, s, nil, '\n');
	while(s != nil){
		g = graphs;
		switch(*s){
		case 0:
			return;
		case 'E': 
			warn("Error:%s\n", s+1);
			goto next;
		case 'I':
			/* FIXME: error check */
			showobject(s + 2);
			req |= Reqredraw;
			goto next;
		case 'R':
			reqlayout(g, Lreset);
			goto next;
		case 'N':
		case 'X':
		case 'Y':
		case 'c':
		case 'f':
		case 'i':
		case 'o':
		case 'x':
		case 'y':
			break;
		default:
			warn("reply: <%s>\n", s);
			goto next;
		}
		if((m = getfields(s+1, fld, nelem(fld), 1, "\t ")) < 1)
			goto error;
		switch(s[0]){
		invalid:
			werrstr("invalid %c message length %d\n", s[0], m);
			/* wet floor */
		error:
			warn("readcmd: %s\n", error());
			break;
		case 'N':
			if(m != 1)
				goto invalid;
			if(fnfindnode(fld[0]) < 0)
				goto error;
			req |= Reqredraw | Reqfocus;
			break;
		case 'X':
			if(m != 2)
				goto invalid;
			if(fnsetpos(g, fld[0], fld[1], 1, 0) < 0)
				goto error;
			break;
		case 'Y':
			if(m != 2)
				goto invalid;
			if(fnsetpos(g, fld[0], fld[1], 1, 1) < 0)
				goto error;
			break;
		case 'c':
			if(m != 2)
				goto invalid;
			if(fnsetcolor(fld[0], fld[1]) < 0)
				goto error;
			req |= Reqshallowdraw;
			break;
		case 'f':
			if(m != 1)
				goto invalid;
			if(loadfs(fld[0], FFcsv) < 0)
				warn("readcmd: csv %s: %s\n", fld[0], error());
			break;
		case 'i':
			if(m != 1)
				goto invalid;
			if(importlayout(g, fld[0]) < 0)
				warn("readcmd: importlayout from %s: %s\n", fld[0], error());
			req |= Reqredraw;
			break;
		case 'o':
			if(m != 1)
				goto invalid;
			if(exportlayout(g, fld[0]) < 0)
				warn("readcmd: exportlayout to %s: %s\n", fld[0], error());
			break;
		case 'x':
			if(m != 2)
				goto invalid;
			if(fnsetpos(g, fld[0], fld[1], 0, 0) < 0)
				goto error;
			break;
		case 'y':
			if(m != 2)
				goto invalid;
			if(fnsetpos(g, fld[0], fld[1], 0, 1) < 0)
				goto error;
			break;
		}
	next:
		s = t;
		t = nextfield(nil, s, nil, '\n');
	}
	if(req != 0)
		reqdraw(req);
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
		readcmd(s);
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
