#include "strpg.h"
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

static Keyboardctl *kc;
static Mousectl *mc;
static Channel *wc, *dc;

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
	}
	return r;
}

int
evloop(void)
{
	int level;
	Rune r;
	Point Δ;
	Mouse mold;
	Graph *g;

	enum{
		Aresize,
		Amouse,
		Akbd,
		Adraw,
		Alayout,
		Aend,
	};
	Alt a[] = {
		[Aresize] {mc->resizec, nil, CHANRCV},
		[Amouse] {mc->c, nil, CHANRCV},
		[Akbd] {kc->c, &r, CHANRCV},
		[Adraw] {dc, &level, CHANRCV},
		[Alayout] {wc, &g, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	mold = mc->Mouse;	/* likely blank */
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			triggerdraw(DTreinit);
			mold.xy = mc->xy;
			/* wet floor */
		case Amouse:
			if(a[Amouse].v == nil){	/* ignore first mouse delta */
				a[Amouse].v = &mc->Mouse;
				mold = mc->Mouse;
			}
			Δ = subpt(mc->xy, mold.xy);
			// FIXME: scroll
			mouseevent(Vec2(mc->xy.x, mc->xy.y), Vec2(Δ.x, Δ.y), mc->buttons & 7);
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
		case Adraw:
			switch(level){
			case DTrender:
				for(g=graphs; g<graphs+ngraphs; g++)
					render(g);
				redraw();
				break;
			case DTreinit:	resetui();	/* wet floor */
			case DTreset:	resetdraw();	/* wet floor */
			case DTredraw:	redraw(); break;
			case DTmove:	shallowdraw(); break;
			}
			break;
		case Alayout:
			dolayout(g, -1);
			break;
		}
	}
}

void
triggerdraw(ulong level)
{
	nbsendul(dc, level);
}

void
triggerlayout(Graph *g)
{
	nbsendp(wc, g);
}

void
initui(void)
{
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkeyboard: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
	if((dc = chancreate(sizeof(int), 1)) == nil
	|| (wc = chancreate(sizeof(Graph *), 1)) == nil)
		sysfatal("chancreate: %r");
}
