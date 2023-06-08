#include "strpg.h"

int debug;
int indexed;
int haxx0rz;
int noui;

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
run(void)
{
	init();
	if(!noui)
		resetdraw();
	flushcmd();
	if(!noui)
		evloop();
}

static void
usage(void)
{
	sysfatal("usage: %s [-Dbs] [-l layout] [FILE]\n", argv0);
}

int
parseargs(int argc, char **argv)
{
	char *s;

	ARGBEGIN{
	case 'D': debug = 1; break;
	case 'b': haxx0rz = 1; break;
	case 'i': indexed = 1; break;
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
	while(*argv != nil){
		if(pushcmd(COMload, strlen(*argv), indexed?FFindex:FFgfa, (uchar *)*argv) < 0)
			warn("error loading %s: %r\n", *argv);
		argv++;
	}
	if(argc > 1 && pushcmd(COMredraw, 0, 0, nil) < 0)
		warn("error sending %s: %r\n", *argv);
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
	initdrw();
	initui();
	resetui(1);
}
