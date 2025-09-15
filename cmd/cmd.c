#include "strpg.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

static File *cmdfs;
static QLock cmdlock;

void
killcmd(void)
{
	/* FIXME: potential deadlock on exit from the other functions? */
	qlock(&cmdlock);	/* completely prevent acquisition of dead cmdfs */
	if(cmdfs == nil){
		qunlock(&cmdlock);
		return;
	}
	freefs(cmdfs);
	cmdfs = nil;
	close(outfd[1]);
	outfd[1] = -1;
	close(infd[1]);
	infd[1] = -1;
	close(sysfd(0));
	qunlock(&cmdlock);
}

void
flushcmd(void)
{
	qlock(&cmdlock);
	if(cmdfs != nil)
		flushfs(cmdfs);
	qunlock(&cmdlock);
}

static void
sendcmd(char *cmd)
{
	int n;

	qlock(&cmdlock);
	if(cmdfs != nil){
		n = strlen(cmd);
		DPRINT(Debugcmd, "> [%d][%s]", n, cmd);
		writefs(cmdfs, cmd, n);
	}
	qunlock(&cmdlock);
}

void
pushcmd(char *fmt, ...)
{
	ioff ai;
	double af;
	s64int av;
	char c, *f, sb[1024], *sp, *as;
	va_list arg;

	if(cmdfs == nil)
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
		case 'D':
			av = va_arg(arg, s64int);
			sp = seprint(sp, sb+sizeof sb-1, "0x%llx", av);
			break;
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

/* FIXME: remove */
/* usable once topology has been loaded */
static inline ioff
str2id(char *s)
{
	ioff id;
	char *t;

	if((id = strtoll(s, &t, 0)) == 0 && t == s)
		return -1;
	return id;
}

static inline int
fnfindnode(char *sid){
	ioff id;

	if((id = str2id(sid)) < 0)
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

static void
readcmd(char *s)
{
	int m, req;
	char *fld[8], *e;

	req = 0;
	e = s;
	while((s = nexttok(e, &e)) != nil){
		if(s[1] != 0 && s[1] != '\t'){
			/* heuristic based on strawk/lib.c... */
			if(s[0] == ' ' || strncmp(s, "strawk:", 7) == 0)
				logerr(va("error: %s\n", s));
			else
				logmsg(va("%s\n", s));
			continue;
		}
		switch(*s){
		case 0:
			return;
		case '!':
			quit();
			break;
		case 'E':
			logerr(va("error: %s\n", s+2));
			continue;
		case 'R':
			graph.flags |= GFarmed;
			if(reqlayout(Lreset))
				warn("readcmd: reqlayout: %s\n", error());
			req |= Reqredraw;
			continue;
		case 'r':
			graph.flags |= GFarmed;
			if(drawing.flags & DFnope){
				if(reqlayout(Lstart))
					warn("readcmd: reqlayout: %s\n", error());
			}else
				req |= Reqredraw;
			continue;
		case 'U':	/* FIXME: kludge (see awkext) */
			pushcmd("deselect()");
			flushcmd();
			continue;
		case 'N':
		case 'f':
		case 'i':
		case 'o':
		case 't':
			break;
		default:
			logmsg(va("%s\n", s));
			continue;
		}
		if((m = getfields(s+1, fld, nelem(fld), 1, "\t")) < 1)
			goto error;
		switch(s[0]){
		invalid:
			werrstr("invalid %c message length %d\n", s[0], m);
			/* wet floor */
		error:
			warn("readcmd: %c: %s\n", s[0], error());
			break;
		case 'N':
			if(m != 1)
				goto invalid;
			if(fnfindnode(fld[0]) < 0)
				goto error;
			break;
		case 'f':
			if(m != 1)
				goto invalid;
			if(loadfs(fld[0], FFcsv) < 0)
				warn("readcmd: csv %s: %s\n", fld[0], error());
			if(graph.flags & GFarmed)
				req |= Reqredraw;
			break;
		case 'i':
			if(m != 1)
				goto invalid;
			if(importlayout(fld[0]) < 0)
				warn("readcmd: importlayout from %s: %s\n", fld[0], error());
			if(graph.flags & GFarmed)
				req |= Reqredraw;
			break;
		case 'o':
			if(m != 1)
				goto invalid;
			if(exportlayout(fld[0]) < 0)
				warn("readcmd: exportlayout to %s: %s\n", fld[0], error());
			break;
		case 't':
			if(m != 1)
				goto invalid;
			if(loadfs(fld[0], FFctab) < 0)
				warn("readcmd: readctab %s: %s\n", fld[0], error());
			break;
		}
	}
	if(req != 0)
		reqdraw(req);
}

static void
readcproc(void *)
{
	char *s;
	File *f;

	if((f = fdopenfs(outfd[0], OREAD)) == nil)
		sysfatal("readcproc: %s", error());
	while((s = readline(f)) != nil){
		DPRINT(Debugcmd, "< [%d][%s]", f->len, s);
		if(f->trunc){
			warn("readcproc: discarding abnormally long awk line");
			continue;
		}
		readcmd(s);
	}
	freefs(f);
}

static void
readstdin(void *)
{
	int fd, n;
	char buf[4096];

	if((fd = sysfd(0)) < 0)
		sysfatal("readstdin: %s", error());
	while((n = read(fd, buf, sizeof buf - 1)) > 0){
		if(n < sizeof buf - 1)
			buf[n++] = '\n';
		buf[n] = 0;
		sendcmd(buf);
		if(n < sizeof buf)
			flushcmd();
	}
}

void
initstdin(void)
{
	newthread(readstdin, nil, nil, nil, "stdin", mainstacksize);
}

int
initcmd(void)
{
	initqlock(&cmdlock);
	if(initrepl() < 0){
		logerr(va("initcmd: failed: %s\n", error()));
		gottagofast = 1;
		return -1;
	}
	if(infd[1] >= 0 && (cmdfs = fdopenfs(infd[1], OWRITE)) == nil)
		sysfatal("initcmd: %s", error());
	newthread(readcproc, nil, nil, nil, "readawk", mainstacksize);
	return 0;
}
