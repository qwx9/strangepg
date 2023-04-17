#include "strpg.h"
#include "layout.h"

/* force-directed layout */

static int
compute(Graph *g)
{
	return 0;
}

static Layout ll = {
	.name = "force",
	.compute = compute,
};

Layout *
regforce(void)
{
	return &ll;
}
