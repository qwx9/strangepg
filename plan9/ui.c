#include "strpg.h"
#include "ui.h"
#include "cmd.h"
#include "drw.h"
#include "threads.h"
#include <draw.h>
#include <mouse.h>

typedef struct Kev Kev;
struct Kev{
	Rune r;
	int down;
};
static int conson;
static Channel *kbc;
static Keyboardctl kc;
static char *ptext;

static Mousectl *mc;

static void
kproc(void *)
{
	int n, fd;
	char c, buf[256], *s;
	Rune r;
	Kev k;

	if((fd = open("/dev/kbd", OREAD)) < 0)
		sysfatal("kproc: %r");
	memset(buf, 0, sizeof buf);
	for(;;){
		if(buf[0] != 0){
			n = strlen(buf)+1;
			memmove(buf, buf+n, sizeof(buf)-n);
		}
		if(buf[0] == 0){
			if((n = read(fd, buf, sizeof(buf)-1)) <= 0)
				break;
			buf[n-1] = 0;
			buf[n] = 0;
		}
		c = *buf;
		if(conson){
			if(c == 'c'){
				chartorune(&r, buf+1);
				send(kc.c, &r);
			}
			continue;
		}
		if(c == 'k')
			k.down = 1;
		else if(c == 'K')
			k.down = 0;
		else
			continue;
		s = buf+1;
		while(*s != 0){
			s += chartorune(&k.r, s);
			send(kbc, &k);
		}
	}
}

char *
enterprompt(Rune r, char *old)
{
	int n;
	char buf[512], *p;

	if(old != nil){
		p = strecpy(buf, buf+sizeof buf, old);
		free(old);
	}else
		p = buf;
	n = p - buf;
	if(r != 0 && r != '\n' && sizeof buf - n > runelen(r) + 1){
		p += runetochar(p, &r);
		*p = 0;
	}
	lockdisplay(display);
	conson = 1;
	n = enter("cmd:", buf, sizeof buf - UTFmax, mc, &kc, _screen);
	conson = 0;
	/* enter() empties chan at the beginning, but we want neither kproc
	 * to block on send nor miss key strokes; it sucks */
	while(nbrecv(kc.c, nil) == 1)
		;
	unlockdisplay(display);
	if(n < 0){
		warn("enterprompt: %r\n");
		return nil;
	}else if(n > 0)
		return estrdup(buf);
	else
		return nil;
}

void
resetprompt(void)
{
	free(ptext);
	ptext = nil;
}

void
prompt(Rune r)
{
	if((ptext = enterprompt(r, ptext)) == nil)
		return;
	pushcmd("%s", ptext);
}

void
evloop(void)
{
	int i;
	Kev k;
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
		[Amouse] {mc->c, &mc->Mouse, CHANRCV},
		[Akbd] {kbc, &k, CHANRCV},
		[Aend] {nil, nil, CHANEND},
	};
	mold = mc->Mouse;	/* likely blank */
	mold.xy = Pt(-1,-1);	/* ignore first mouse delta */
	for(;;){
		switch(alt(a)){
		case Aresize:
			lockdisplay(display);
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			unlockdisplay(display);
			reqdraw(Reqresetdraw);
			mold.xy = mc->xy;
			/* wet floor */
		case Amouse:
			if(mold.buttons != mc->buttons){
				for(i=1<<0; i<1<<5; i<<=1)
					if(((mold.buttons ^ mc->buttons) & i) != 0)
						keyevent(Kmouse | i, mc->buttons & i);
			}
			if(mold.xy.x < 0 || mold.xy.y < 0)
				mold = mc->Mouse;
			Δ = subpt(mc->xy, mold.xy);
			mouseevent(V(mc->xy.x - screen->r.min.x, mc->xy.y - screen->r.min.y, 0.0f), V(Δ.x, Δ.y, 0.0f));
			mold = mc->Mouse;
			break;
		case Akbd:
			switch(k.r){
			case Kdel: /* wet floor */
			case 'q': quit();
			default: keyevent(k.r, k.down); break;
			}
			break;
		}
	}
}

void
initsysui(void)
{
	if((kc.c = chancreate(sizeof(Rune), 20)) == nil
	|| (kbc = chancreate(sizeof(Kev), 8)) == nil)
		sysfatal("initsysui:chancreate: %r");
	if(proccreate(kproc, nil, mainstacksize) < 0)
		sysfatal("proccreate: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
}
