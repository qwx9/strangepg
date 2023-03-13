#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

// may be mode or layout-specific
static int	(*mousefn)(Mouse, Point);
static int	(*keyfn)(Rune);
static Keyboardctl *kc;
static Mousectl *mc;

static int
mousepan(Point Δ)
{
	return panview(Δ);
}

static int
mouseselect(Point p)
{
	USED(p);
	return 0;
}

static int
mousemenu(Mouse m)
{
	USED(m);
	return 0;
}

int
mouseprompt(char *label, char *buf, int bufsz)
{
	if(enter(label, buf, bufsz, mc, kc, nil) < 0)
		return -1;
	return 0;
}

// FIXME: render more generic; mouse/keyfn shouldn't be
// platform specific, for the most part
static int
defaultmouse(Mouse m, Point Δ)
{
	int r;
	Mouse om;

	// FIXME: make sure these return 0 or -1
	r = 0;
	// no hover yet
	switch(m.buttons & 7){
	case 1: r = mouseselect(m.xy); break;
	case 2: r = mousemenu(m); break;
	case 4: r = mousepan(Δ); break;
	}
	om = m;
	return r;
}

static int
defaultkey(Rune r)
{
	switch(r){
	case Kdel:
	case 'q': threadexitsall(nil);
	}
	return 0;
}

Event
waitevent(void)
{
	int n;
	Rune r;
	Mouse mold;
	Event e;

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
	memset(&e, 0, sizeof e);
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			resetdisp();
			mold = mc->Mouse;
			break;
		case Amouse:
			n = mousefn(mc->Mouse, subpt(mc->Mouse.xy, mold.xy));
			mold = mc->Mouse;
			USED(n);
			break;
		case Akbd:
			n = keyfn(r);
			USED(n);
			break;
		}
	}
}

void
initsysui(void)
{
	keyfn = defaultkey;
	mousefn = defaultmouse;
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkeyboard: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
}
