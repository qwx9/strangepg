#include "strpg.h"
#include "fsprv.h"

static Graph shim;

static Graph *
load(char *)
{
	Graph *g;

	g = initgraph(&shim);
	addnode(g, "a");
	addnode(g, "b");
	addnode(g, "c");
	addnode(g, "d");
	addnode(g, "e");
	addedge(g, "A", 0, 1, 0.);
	addedge(g, "B", 1, 2, 0.);
	addedge(g, "C", 2, 3, 0.);
	addedge(g, "D", 3, 4, 0.);
	addedge(g, "E", 2, 2, 0.);
	addedge(g, "F", 2, 4, 0.);
	addedge(g, "G", 4, 4, 0.);
	return g;
}

static int
save(Graph *)
{
	return 0;
}

static Filefmt ff = {
	.name = "gfa",
	.load = load,
	.save = save,
};

Filefmt *
reggfa(void)
{
	return &ff;
}
