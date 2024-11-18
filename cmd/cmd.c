#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

static char cmdbuf[8*1024], *cmdp = cmdbuf;
static RWLock cmdlock;

void
flushcmd(void)
{
	int n;

	wlock(&cmdlock);
	n = cmdp - cmdbuf;
	if(n == 0)
		return;
	if(write(infd[1], cmdbuf, n) != n){
		DPRINT(Debugcmd, "sendcmd: %s", error());
		close(infd[1]);
		infd[1] = -1;
	}
	cmdp = cmdbuf;
	wunlock(&cmdlock);
}

// FIXME: or, use fs
static void
sendcmd(char *cmd)
{
	int n;
	char *p;

	if(infd[1] < 0)
		return;
	wlock(&cmdlock);
	p = strecpy(cmdp, cmdbuf+sizeof cmdbuf, cmd);
	if(p == cmdbuf + sizeof cmdbuf - 1){
		n = cmdp - cmdbuf;
		if(write(infd[1], cmdbuf, n) != n){
			DPRINT(Debugcmd, "sendcmd: %s", error());
			close(infd[1]);
			infd[1] = -1;
		}
		cmdp = cmdbuf;
		p = strecpy(cmdp, cmdbuf+sizeof cmdbuf, cmd);
		if(p == cmdbuf + sizeof cmdbuf - 1)
			sysfatal("????");	/* FIXME */
	}
	cmdp = p;
	wunlock(&cmdlock);
}

void
pushcmd(char *fmt, ...)
{
	ioff ai;
	double af;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	if(infd[1] < 0)
		return;
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
		case 'f':
			af = va_arg(arg, double);
			sp = seprint(sp, sb+sizeof sb-1, "%f", af);
			break;
		case 'x':
			ai = va_arg(arg, ioff);
			sp = seprint(sp, sb+sizeof sb-1, "0x%x", ai);
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
		n->attr.flags |= fix ? FNfixedx | FNinitx : FNinitx;
		n->attr.pos0.x = c;
	}else{
		n->attr.flags |= fix ? FNfixedy | FNinity : FNinity;
		n->attr.pos0.y = c;
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
	v = strtoul(col, &p, 0);
	if(p == col){
		werrstr("invalid color %s", col);
		return -1;
	}
	v = v << 8 | 0xb0;	/* FIXME: need a better way to handle this */
	setcolor(rnodes[id].col, v);
	return 0;
}

static inline int
fnsetsel(Graph *, char *s)
{
	showselected(s);
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

static char *
nexttok(char *s, char **end)
{
	int n;
	char *t;

	if(s == nil || *s == 0)
		return nil;
	n = strlen(s);
	if((t = memchr(s, '\n', n)) != nil){
		*t++ = 0;
		if(end != nil)
			*end = t;
		return s;
	}else if(end != nil)
		*end = s + n;
	return s;
}

/* FIXME: multiple graphs: per-graph state? one pipe per graph?
* how do we dispatch user queries? would be useful to do queries
* on multiple ones! */
void
readcmd(char *s)
{
	int m, req;
	char *fld[8], *e;
	Graph *g;

	req = 0;
	e = s;
	while((s = nexttok(e, &e)) != nil){
		g = graphs;
		if(s[1] != 0 && s[1] != '\t'){
			logerr(s);
			continue;
		}
		switch(*s){
		case 0:
			return;
		case '!':
			quit();
			break;
		case 'E':
			logerr(va("> error:%s\n", s+1));
			continue;
		case 'I':
			showobject(s + 2);
			req |= Reqshallowdraw;
			continue;
		case 'R':
			reqlayout(g, Lreset);
			continue;
		case 's':
			if(s[1] == 0){
				showselected(nil);
				req |= Reqshallowdraw;
				continue;
			}
			break;
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
			logmsg(va("> %s\n", s));
			continue;
		}
		if((m = getfields(s+1, fld, nelem(fld), 1, "\t")) < 1)
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
			req |= Reqfocus | Reqshallowdraw;
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
			break;
		case 'o':
			if(m != 1)
				goto invalid;
			if(exportlayout(g, fld[0]) < 0)
				warn("readcmd: exportlayout to %s: %s\n", fld[0], error());
			break;
		case 's':
			if(m != 1)
				goto invalid;
			if(fnsetsel(g, fld[0]) < 0)
				goto error;
			req |= Reqshallowdraw;
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
	}
	if(req != 0)
		reqdraw(req);
}

static void
readcproc(void *fd)
{
	char *s;
	File *f;

	if((f = fdopenfs(outfd[0], OREAD)) == nil)
		sysfatal("readcproc: %s", error());
	while((s = readline(f)) != nil){
		DPRINT(Debugcmd, "cmd < [%d][%s]", f->len, s);
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

	if((fd = initrepl()) < 0){
		logerr(va("initcmd: failed: %s\n", error()));
		gottagofast = 1;
		return -1;
	}
	newthread(readcproc, nil, nil, nil, "readawk", mainstacksize);
	return 0;
}
