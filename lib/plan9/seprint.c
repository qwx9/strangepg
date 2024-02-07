#include "strpg.h"
#include <stdio.h>

/* /sys/src/libc/fmt/^('' v)^seprint.c, modified */

char*
seprint(char *buf, char *e, char *fmt, ...)
{
	char *p;
	va_list args;

	p = buf;
	va_start(args, fmt);
	if(p < e)
		p += vsnprintf(p, e-p, fmt, args);
	va_end(args);
	return p;
}
