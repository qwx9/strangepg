#include "strpg.h"
#include "cmd.h"
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

extern Channel *cmdc;

static Keyboardctl *kc;
static Mousectl *mc;

static int
k2e(Rune r)
{
	switch(r){
	case Kleft: r = KBleft; break;
	case Kright: r = KBright; break;
	case Kup: r = KBup; break;
	case Kdown: r = KBdown; break;
	case Kesc: r = KBescape; break;
	}
	return r;
}

char *
enterprompt(Rune r)
{
	int n;
	char buf[256] = {0};

	runetochar(buf, &r);
	if((n = enter("Cmd: ", buf, sizeof(buf)-UTFmax, mc, kc, nil)) < 0){
		warn("enterprompt: %r\n");
		return nil;
	}else if(n > 0)
		return estrdup(buf);
	else
		return nil;
}

void
evloop(void)
{
	char *s;
	Rune r;
	Point Δ;
	Mouse mold;

	enum{
		Aresize,
		Amouse,
		Akbd,
		Acmd,
		Aend,
	};
	Alt a[] = {
		[Aresize] {mc->resizec, nil, CHANRCV},
		[Amouse] {mc->c, &mc->Mouse, CHANRCV},
		[Akbd] {kc->c, &r, CHANRCV},
		[Acmd] {cmdc, &s, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	mold = mc->Mouse;	/* likely blank */
	mold.xy = Pt(-1,-1);	/* ignore first mouse delta */
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			reqdraw(Reqresetdraw);
			mold.xy = mc->xy;
			/* wet floor */
		case Amouse:
			if(mold.xy.x < 0 || mold.xy.y < 0)
				mold = mc->Mouse;
			Δ = subpt(mc->xy, mold.xy);
			// FIXME: scroll
			mouseevent(Vec2(mc->xy.x, mc->xy.y), Vec2(Δ.x, Δ.y), mc->buttons & 7);
			mold = mc->Mouse;
			break;
		case Akbd:
			switch(r){
			case Kdel:
			case 'q': quit();
			default: keyevent(k2e(r)); break;
			}
			break;
		case Acmd:
			DPRINT(Debugcmd, "← ui:<%s>", s);
			parseresponse(s);
			free(s);
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
