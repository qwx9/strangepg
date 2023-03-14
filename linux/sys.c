#include "strpg.h"
#include <stdio.h>
#include <string.h>

int debug;
char *argv0;

static char err[512];

char *
errstr(void)
{
	return err[0] != 0 ? err : "";
}

void
werrstr(char *fmt, ...)
{
	va_list arg;
	memset(err, 0, sizeof err);
	va_start(arg, fmt);
	vsnprintf(err, sizeof err, fmt, arg);
	va_end(arg);
}

void
sysfatal(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	exit(66);
}

void
warn(char *fmt, ...)
{
	va_list arg;

	if(debug <= 0)
		return;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

void *
erealloc(void *p, usize n, usize oldn)
{
	if((p = realloc(p, n)) == nil)
		sysfatal("realloc: %r");
	if(n > oldn)
		memset((uchar *)p + oldn, 0, n - oldn);
	return p;
}

void *
emalloc(usize n)
{
	void *p;

	if((p = calloc(1, n)) == nil)
		sysfatal("emalloc: %r");
	return p;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(1);
}

int
main(int argc, char **argv)
{
	ARGBEGIN{
	default: usage();
	}ARGEND
	if(parseargs(argc, argv) < 0)
		usage();
	init();
	evloop();
	return 0;
}
