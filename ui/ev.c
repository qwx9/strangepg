#include "strpg.h"

static int
parsekey(Event e)
{
	USED(e);
	return 0;
}

static int
parsemouse(Event e)
{
	USED(e);
	return 0;
}

void
evloop(void)
{
	Event ev;

	for(;;){
		ev = waitevent();
		switch(ev.type){
		case EVmouse:
			//parsemouse(ev.key);
			/* wet floor */
		case EVkbd:
			if(ev.key <= 0){
				warn("evloop: invalid event: %r");
				continue;
			}
			switch(ev.key){
			default:;
			}
			break;
		default:;
		}
		/* FIXME: key events, mouse events ... */
	}
}
