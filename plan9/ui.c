#include "strpg.h"
#include "uiprv.h"
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

typedef Vertex Point;

static Keyboardctl *kc;
static Mousectl *mc;

/*
static int
menter(char *label, char *buf, int bufsz)
{
	if(enter(label, buf, bufsz, mc, kc, nil) < 0)
		return -1;
	return 0;
}
*/

int
evloop(void)
{
	Rune r;
	Mouse mold;

	enum{
		Aresize,
		Amouse,
		Akbd,
		Aend,
	};
	Alt a[] = {
		[Aresize] {mc->resizec, nil, CHANRCV},
		[Amouse] {mc->c, &mc->Mouse, CHANRCV},
		[Akbd] {kc->c, &r, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			resetdraw(lolgraph, &lolrender);
			/* wet floor */
		case Amouse:
			mold = mc->Mouse;
			break;
		case Akbd:
			switch(r){
			case Kdel:
			case 'q':
				threadexitsall(nil);
			}
			break;
		}
	}
}

void
initui_(void)
{
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkeyboard: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
}
