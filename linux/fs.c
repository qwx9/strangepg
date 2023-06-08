#include "strpg.h"
#include "fs.h"

int
sysopen(File *, int)
{
	return 0;
}

void
sysclose(File *)
{
}

void
sysflush(File *)
{
}

int
syswstatlen(File *f, vlong)
{
	return 0;
}

vlong
sysseek(File *, vlong)
{
	return 0;
}

vlong
systell(File *)
{
	return 0;
}

int
syswrite(File *, void *, int)
{
	return 0;
}

int
sysread(File *, void *, int)
{
	return 0;
}

char *
readrecord(File *)
{
	return "";
}
