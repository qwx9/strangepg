#include "strpg.h"

// FIXME: single graph for now
Graph *lolgraph;
Layer lolayer;
Render lolrender;

void
run(void)
{
	flushcmd();
	lolayer = dolayout(lolgraph, LLconga);
	lolrender = render(lolgraph, &lolayer);
	redraw(lolgraph, &lolrender);
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
	run();
}
