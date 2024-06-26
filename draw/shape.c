#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "drw.h"
#include "threads.h"

/* FIXME: get rid of this */
int
reshape(int force)
{
	int r;
	Graph *g;
	static Clk clk = {.lab = "shape"};

	r = 0;
	CLK0(clk);
	lockgraphs(0);
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type == FFdead || (g->flags & GFlayme) == 0)
			continue;
		if((g->flags & GFdrawme) != 0 || force){
			if(g->layout == nil){
				DPRINT(Debugrender, "reshape: no layout");
				continue;
			}
			if(dylen(g->nodes) < 1){
				DPRINT(Debugrender, "reshape: empty graph");
				continue;
			}
			r = 1;
		}
	}
	unlockgraphs(0);
	CLK1(clk);
	return r;
}
