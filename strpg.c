#include "strpg.h"

int debug;

void
run(void)
{
	init();
	resetdraw();
	flushcmd();
	evloop();
}

static void
usage(void)
{
	sysfatal("usage: %s [-Ds] [-l layout] [FILE]\n", argv0);
}

int
parseargs(int argc, char **argv)
{
	char *s;

	ARGBEGIN{
	case 'D': debug = 1; break;
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
	case 's': drawstep = 1; break;
	default: usage();
	}ARGEND
	while(*argv != nil){
		if(pushcmd(COMload, strlen(*argv), FFgfa, (uchar *)*argv) < 0)
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
	initdrw();
	initui();
	resetui(1);
}
