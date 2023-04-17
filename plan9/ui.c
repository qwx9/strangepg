#include "strpg.h"
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

static int
k2e(Rune r)
{
	switch(r){
	case Kleft: r = K←; break;
	case Kright: r = K→; break;
	case Kup: r = K↑; break;
	case Kdown: r = K↓; break;
	default: dprint("k2e: unhandled key %C\n", r);
	}
	return r;
}

int
evloop(void)
{
	Rune r;
	Point Δ;
	Mouse mold;

	enum{
		Aresize,
		Amouse,
		Akbd,
		Aend,
	};
	Alt a[] = {
		[Aresize] {mc->resizec, nil, CHANRCV},
		[Amouse] {mc->c, nil, CHANRCV},
		[Akbd] {kc->c, &r, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	mold = mc->Mouse;	/* likely blank */
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			resetdraw();
			resetui();
			redraw();
			mold.xy = mc->xy;
			/* wet floor */
		case Amouse:
			if(a[Amouse].v == nil){	/* ignore first mouse delta */
				a[Amouse].v = &mc->Mouse;
				mold = mc->Mouse;
			}
			Δ = subpt(mc->xy, mold.xy);
			// FIXME: scroll
			mouseevent(mc->xy, Δ, mc->buttons & 7);
			mold = mc->Mouse;
			break;
		case Akbd:
			switch(r){
			case Kdel:
			case 'q':
				threadexitsall(nil);
			default: keyevent(k2e(r)); break;
			}
			break;
		}
	}
}

void
initui(void)
{
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkeyboard: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
}
