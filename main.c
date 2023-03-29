#include "strpg.h"

void
run(void)
{
	flushcmd();
	if(dolayout(graph, LLconga) < 0)
		sysfatal("dolayout: %s\n", error());
	if(render(graph) < 0)
		sysfatal("render: %s\n", error());
	redraw(graph);
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
	flushcmd();	// FIXME: temp
	initlayout();
	initrend();
	initvdraw();
	initui();
	run();
}
