#include "strpg.h"
#include "dispprv.h"

View view;

void
show(Graph *, Layer *)
{
	// take vector image
	// draw at default zoom level + translated by pan offset
	// paste on screen
}

// FIXME: rename disp → view?
void
initdisp(void)
{
	view = initsysdisp();
}
