#include "strpg.h"
#include <string.h>

/* /sys/src/libc/port/strecpy.c */

char*
strecpy(char *to, char *e, const char *from)
{
	if(to >= e)
		return to;
	to = memccpy(to, from, '\0', e - to);
	if(to == nil){
		to = e - 1;
		*to = '\0';
	}else{
		to--;
	}
	return to;
}
