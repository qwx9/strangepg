#include "strpg.h"
#include "threads.h"
#include <pool.h>

int noui, debug;

void
sysquit(void)
{
	/* FIXME: inelegant */
	if(noui)
		threadexits(nil);
	else
		threadexitsall(nil);
}

char *
error(void)
{
	static char err[1024];

	errstr(err, sizeof err);
	return err;
}

void
vawarn(char *fmt, va_list arg)
{
	vfprint(2, fmt, arg);
	va_end(arg);
}

void
vadebug(char *type, char *fmt, va_list arg)
{
	fprint(2, "[%s] ", type);
	vfprint(2, fmt, arg);
	va_end(arg);
	fprint(2, "\n");
}

vlong
μsec(void)
{
	return nsec() / 1000;
}

char *
estrdup(char *s)
{
	if((s = strdup(s)) == nil){
		if(debug)
			abort();
		sysfatal("estrdup: %r");
	}
	setmalloctag(s, getcallerpc(&s));
	return s;
}

void *
erealloc(void *p, usize n, usize oldn)
{
	if((p = realloc(p, n)) == nil){
		if(n == 0)
			return nil;
		if(debug)
			abort();
		sysfatal("realloc: %r");
	}
	setrealloctag(p, getcallerpc(&p));
	if(n > oldn)
		memset((uchar *)p + oldn, 0, n - oldn);
	return p;
}

void *
emalloc(usize n)
{
	void *p;

	if((p = mallocz(n, 1)) == nil){
		if(n == 0)
			return nil;
		if(debug)
			abort();
		sysfatal("emalloc: %r");
	}
	setmalloctag(p, getcallerpc(&n));
	return p;
}

/* FIXME: em: make sure we don't run into wrappers too much */

void
sysinit(void)
{
	//mainmem->flags |= POOL_NOREUSE | POOL_PARANOIA;
}
