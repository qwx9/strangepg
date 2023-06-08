#include "strpg.h"

char *argv0;

int
errstr(char *, uint)
{
}

void
werrstr(char *, ...)
{
}

void
vawarn(char *, va_list)
{
}

void
sysfatal(char *fmt, ...)
{
}

vlong
msec(void)
{
	return 0LL;
}

void *
erealloc(void *, usize, usize)
{
	return nil;
}

void *
emalloc(usize)
{
	return nil;
}
