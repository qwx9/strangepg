#include "strpg.h"
#include <thread.h>

mainstacksize = 16*1024;

char *
error(void)
{
	static char err[1024];

	errstr(err, sizeof err);
	return err;
}

void
warn(char *fmt, ...)
{
	char s[256];
	va_list arg;

	va_start(arg, fmt);
	vseprint(s, s+sizeof s, fmt, arg);
	va_end(arg);
	fprint(2, "%s", s);
}

void
dprint(char *fmt, ...)
{
	char s[256];
	va_list arg;

	if(!debug)
		return;
	va_start(arg, fmt);
	vseprint(s, s+sizeof s, fmt, arg);
	fprint(2, "%s", s);
}

vlong
msec(void)
{
	return nsec() / 1000000;
}

char *
estrdup(char *s)
{
	if((s = strdup(s)) == nil)
		sysfatal("estrdup: %r");
	setmalloctag(s, getcallerpc(&s));
	return s;
}

void *
erealloc(void *p, usize n, usize oldn)
{
	if((p = realloc(p, n)) == nil)
		sysfatal("realloc: %r");
	setrealloctag(p, getcallerpc(&p));
	if(n > oldn)
		memset((uchar *)p + oldn, 0, n - oldn);
	return p;
}

void *
emalloc(usize n)
{
	void *p;

	if((p = mallocz(n, 1)) == nil)
		sysfatal("emalloc: %r");
	setmalloctag(p, getcallerpc(&n));
	return p;
}

void
quit(void)
{
	threadexitsall(nil);
}

void
threadmain(int argc, char **argv)
{
	if(parseargs(argc, argv) < 0)
		sysfatal("usage");
	run();
	threadexitsall(nil);
}
