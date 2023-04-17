#include "strpg.h"

int debug;

void
run(void)
{
	init();
	resetdraw();
	flushcmd();
	rendernew();
	redraw();
	evloop();
}

static void
usage(void)
{
	fprint(2, "usage: %s\n", argv0);
	sysfatal("usage");
}

int
parseargs(int argc, char **argv)
{
	warn("parseargs %d\n", argc);
	ARGBEGIN{
	case 'D': debug = 1; break;
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
	resetui();
}
