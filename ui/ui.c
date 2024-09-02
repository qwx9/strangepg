#include "strpg.h"
#include "threads.h"
#include "cmd.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"
#include "ui.h"

ioff selected = -1;
char *selstring;
int prompting;

static ioff shown = -1;

enum{
	Mctrl = 1<<0,
	Mshift = 1<<1,
	Malt = 1<<2,
	Mlmb = 1<<3,
	Mmmb = 1<<4,
	Mrmb = 1<<5,

	Mmask = Mlmb | Mmmb | Mrmb,
};
static int mod;
static Vertex center;

static void
pan(float Δx, float Δy)
{
	Vertex v;

	v = addv(V(Δx, Δy, 0.0f), view.pan);
	if(eqv(v, view.pan))
		return;
	DPRINT(Debugdraw, "pan %.2f,%.2f → %.2f,%.2f", view.pan.x, view.pan.y, v.x, v.y);
	view.pan = v;
	pandraw(Δx, Δy);
	reqdraw(Reqshallowdraw);
}

static void
zoom(float Δx, float Δy)
{
	float Δ;
	Vertex v;

	/* scalar projection of v onto (1,1) unit vector; this way,
	 * zoom in when dragging ↘; also scale to reasonable
	 * increment */
	Δ = 0.01 * -(Δx + Δy) / 2;
	if(view.zoom + Δ == view.zoom)
		return;
	view.zoom += Δ;
	DPRINT(Debugdraw, "zoom %.1f (%.1f,%.1f) → %.2f ", Δ, Δx, Δy, view.zoom);
	zoomdraw(Δ);
	v = mulv(center, Δ);
	pan(v.x, v.y);
}

int
keyevent(Rune r, int down)
{
	int m;
	Graph *g;

	DPRINT(Debugdraw, "keyevent %d", r);
	if(prompting){
		prompt(r);
		return 0;
	}
	switch(r){
	case Kctl: m = Mctrl; goto setmod;
	case Kshift: m = Mshift; goto setmod;
	case Kalt: m = Malt; goto setmod;
	case Klmb: m = Mlmb; goto setmod;
	case Kmmb: m = Mmmb; goto setmod;
	case Krmb: m = Mrmb; goto setmod;
	setmod:
		if(down)
			mod |= m;
		else
			mod &= ~m;
		return 0;
	}
	if(!down)
		return 0;
	switch(r){
	case Kup: pan(0.0f, -view.w / 2.0f); break;
	case Kdown: pan(0.0f, +view.h / 2.0f); break;
	case Kright: pan(+view.w / 2.0f, 0.0f); break;
	case Kleft: pan(-view.w / 2.0f, 0.0f); break;
	case Kscrlup: zoom(5.0f, 5.0f); break;
	case Kscrldn: zoom(-5.0f, -5.0f); break;
	case Kesc: resetprompt(); reqdraw(Reqresetui); break;
	/* FIXME: doesn't quite make sense */
	case '+': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, 1); unlockgraphs(0); break;
	case '-': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, -1); unlockgraphs(0); break;
	case 'r': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) resetlayout(g); unlockgraphs(0); break;
	case 'p': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) togglelayout(g); unlockgraphs(0); break;
	case 'a': view.flags ^= VFdrawarrows; reqdraw(Reqshallowdraw); break;
	case 'l': view.flags ^= VFdrawlabels; reqdraw(Reqshallowdraw); break;
	case '\n': prompt(r); break;
	}
	return 0;
}

void
showobject(char *s)
{
	free(selstring);
	selstring = s;
}

static ioff
hover(int x, int y)
{
	uint v, id;
	static char *last;

	if(x < 0 || y < 0 || x >= view.w || y >= view.h
	|| (v = mousepick(x, y)) == -1){
		if(last != nil)
			free(last);
		last = selstring;
		selstring = nil;
		return -1;
	}
	if(v == shown)
		return v;
	if((v & (1<<31)) == 0){
		id = (uint)v - 1;
		pushcmd("nodeinfo(%d)", id);
	}else{
		id = (uint)(v & ~(1<<31)) - 1;
		pushcmd("edgeinfo(%d)", id);
	}
	return v;
}

static int
mouseselect(ioff)
{
	return 0;
}

int
mouseevent(Vertex v, Vertex Δ)
{
	int m;
	static int omod;

	m = mod & Mmask;
	if(m != 0 && (omod & ~Mrmb) == 0)
		center = V(v.x - view.w / 2, v.y - view.h / 2, 0);
	if(Δ.x != 0.0 && Δ.y != 0.0)
		shown = hover(v.x, v.y);
	if(m == Mlmb && (omod & Mlmb) == 0){
		if(mouseselect(shown))
			reqdraw(Reqshallowdraw);
	}else if(m == Mrmb){
		if((mod & Mctrl) != 0)
			zoom(-Δ.x, -Δ.y);
		else
			pan(-Δ.x, -Δ.y);
	}else if(m == (Mlmb | Mrmb))
		zoom(-Δ.x, -Δ.y);
	omod = m;
	return 0;
}

void
resetui(void)
{
	view.center = ZV;
	view.eye = V(0.0f, 0.0f, 10.0f);
	view.up = V(0.0f, 1.0f, 0.0f);
	view.Δeye = subv(view.eye, view.center);
	view.pan = ZV;
	view.zoom = 1.0;
}

void
initui(void)
{
	initsysui();
	resetui();
}
