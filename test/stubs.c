#include "strpg.h"

/* FIXME: fuck */

int infd[2] = {-1, -1}, outfd[2] = {-1, -1};
Node *nodes;
ioff *edges;

void	killcmd(void){	}

void
printgraph(void)
{
	ioff i, j, x, *e, *ee;
	Node *u, *ue;

	if((debug & Debuggraph) == 0)
		return;
	DPRINT(Debuggraph, "current graph: %zd nodes, %zd edges",
		dylen(nodes), dylen(edges));
	for(i=0, u=nodes, ue=u+dylen(u); u<ue; u++, i++){
		DPRINT(Debuggraph, "[%d] id=%d off=%d ne=%d",
			i, u->id, u->eoff, u->nedges);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			DPRINT(Debuggraph, "  <%zd> %08x %d%c%d%c",
				e-edges, x, i, x&1?'-':'+', j, x&2?'-':'+');
		}
	}
}
