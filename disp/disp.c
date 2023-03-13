#include "strpg.h"
#include "dispprv.h"

View view;

void
show(void)
{
}

// FIXME: rename disp → view?
void
initdisp(void)
{
	view = initsysdisp();
}
