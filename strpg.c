#include "strpg.h"

int debug;
int haxx0rz;
int noui;

static int intype;
static char **filev;

void
warn(char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vawarn(fmt, arg);
}

void
dprint(char *fmt, ...)
{
	va_list arg;

	if(!debug)
		return;
	va_start(arg, fmt);
	vawarn(fmt, arg);
}

void
quit(void)
{
	sysquit();
}

void
run(void)
{
	char *s;

	init();
	while((s = *filev++) != nil)
		if(loadfs(s, intype) < 0)
			sysfatal("loadfs: could not load %s\n", s);
	if(noui)
		quit();
	evloop();
}

static void
usage(void)
{
	sysfatal("usage: %s [-Dbins] [-l layout] [FILE]\n", argv0);
}

int
parseargs(int argc, char **argv)
{
	char *s;

	intype = FFgfa;
	ARGBEGIN{
	case 'D': debug = 1; break;
	case 'b': haxx0rz = 1; break;
	case 'i': intype = FFindex; break;
	case 'l':
		s = EARGF(usage());
		if(strcmp(s, "random") == 0)
			deflayout = LLrandom;
		else if(strcmp(s, "conga") == 0)
			deflayout = LLconga;
		else if(strcmp(s, "force") == 0)
			deflayout = LLforce;
		else
			sysfatal("unknown layout type");
		break;
	case 'n': noui = 1; break;
	case 's': drawstep = 1; break;
	default: usage();
	}ARGEND
	filev = argv;
	return 0;
}

void
init(void)
{
	initfs();
	initlayout();
	initrend();
	if(noui)
		return;
	initalldraw();
	initui();
	resetui(1);
}
