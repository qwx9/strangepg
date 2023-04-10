#include "strpg.h"

// FIXME: no command queue handling
void
run(void)
{
	init();
	resetdraw();


	// FIXME ------> cut from
	flushcmd();
	ngraphs++;
	if(dolayout(&graphs[0], LLconga) < 0)
		sysfatal("dolayout: %s\n", error());
	if(render(&graphs[0]) < 0)
		sysfatal("render: %s\n", error());
	redraw();
	// FIXME <------ to


	redraw();
	evloop();
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
	initvdraw();
	initui();
}
