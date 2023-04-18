#include "strpg.h"
#include "layout.h"

/* addnodes() already spawns nodes with random coordinates, so
 * no need to do anything */

static int
compute(Graph *)
{
	return 0;
}

static Layout ll = {
	.name = "random",
	.compute = compute,
};

Layout *
regrandom(void)
{
	return &ll;
}
