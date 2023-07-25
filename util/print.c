#include "strpg.h"

void
warn(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
}

void
dprint(int flags, char *fmt, ...)
{
	va_list arg;

	if((debug & flags) == 0)
		return;
	va_start(arg, fmt);
	vawarn(fmt, arg);
}
