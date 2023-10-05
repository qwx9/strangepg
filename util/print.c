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
	char *type;
	va_list arg;

	if((debug & flags) == 0)
		return;
	switch(debug & flags){
	case 0: return;
	case Debugdraw: type = "draw"; break;
	case Debugrender: type = "render"; break;
	case Debuglayout: type = "layout"; break;
	case Debugfs: type = "fs"; break;
	case Debugcoarse: type = "coarse"; break;
	case Debugextmem: type = "extmem"; break;
	default: type = "dafuq"; break;
	}
	va_start(arg, fmt);
	vadebug(type, fmt, arg);
}
