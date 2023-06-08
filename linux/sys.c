#include "strpg.h"	/* FIXME: this sucks */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

char *argv0;

static char errbuf[1024];

int
errstr(char *err, uint nerr)
{
	uint n;

	n = sizeof errbuf < nerr ? sizeof errbuf : nerr;
	strncpy(err, errbuf, n - 1);
	err[n-1] = 0;
	*errbuf = 0;
	return 0;
}

void
werrstr(char *fmt, ...)
{
	va_list arg;
	memset(errbuf, 0, sizeof errbuf);
	va_start(arg, fmt);
	vsnprintf(errbuf, sizeof errbuf, fmt, arg);
	va_end(arg);
}

void
vawarn(char *fmt, va_list arg)
{
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

void
sysfatal(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
	exit(66);
}

char *
error(void)
{
	return errbuf;
}

/* FIXME: check */
vlong
msec(void)
{
	struct timeval tv;

	if(gettimeofday(&tv, nil) < 0){
		warn("gettimeofday: error %d\n", errno);
		return -1;
	}
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
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
