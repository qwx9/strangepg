#include "strpg.h"

// FIXME: single graph for now
Graph *lolgraph;
Layer lolayer;

void
run(void)
{
	Render r;

	flushcmd();
	lolayer = dolayout(lolgraph, LLconga);	// position in space
	r = render(lolgraph, &lolayer);		// pretty drawing with edges, etc.
	show(lolgraph, &r);			// display result
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
	initview();
	initui();
	run();
}
