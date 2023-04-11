#include "strpg.h"

// FIXME: no command queue handling
void
run(void)
{
	init();
	resetdraw();
	flushcmd();
	rendernew();
	evloop();
}

int
parseargs(int argc, char **argv)
{
	char **p;

	// FIXME: option flags
	for(p=argv+1; p<argv+argc; p++){
		if(pushcmd(COMload, strlen(*p), FFgfa, (uchar *)*p) < 0)
			warn("error loading %s: %r\n", *p);
	}
	if(argc > 1 && pushcmd(COMredraw, 0, 0, nil) < 0)
		warn("error sending %s: %r\n", *p);
	return 0;
}

void
init(void)
{
	initfs();
	initlayout();
	initrend();
	initvdraw();
	initui();
}
