#include "strpg.h"

// FIXME: get ARGBEGIN etc from plan9 and use it?

void
run(void)
{
	flushcmd();
	render();
	show();
}

int
parseargs(int argc, char **argv)
{
	char **p;

	for(p=argv+1; p<argv+argc; p++){
		if(pushcmd(COMload, FFgfa, *p) < 0)
			warn("error loading %s: %r\n", *p);
	}
	return 0;
}

void
init(void)
{
	initfs();
	initlayout();
	initrend();
	initdisp();
	initui();
	run();
}
