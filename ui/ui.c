#include "strpg.h"
#include "threads.h"
#include "cmd.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"
#include "ui.h"

ioff selected = -1;
int prompting;
char hoverstr[256], selstr[512];

static ioff shown = -1, focused = -1;

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
	v = mulv(center, Δ);
	zoomdraw(Δ, v.x, v.y);
	reqdraw(Reqshallowdraw);
}

static void
rotate(float Δx, float Δy)
{
	Vertex v;

	if(Δx == 0.0f && Δy == 0.0f)
		return;
	Δx *= PI / 180.0f;
	Δy *= PI / 180.0f;
	Δx *= 0.1f;
	Δy *= 0.1f;
	v.x = Δx;
	v.y = Δy;
	v.z = 0.0f;
	rotdraw(v);
	reqdraw(Reqshallowdraw);
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
	case Kup: pan(0.0f, -view.w / 4.0f); break;
	case Kdown: pan(0.0f, +view.h / 4.0f); break;
	case Kright: pan(+view.w / 4.0f, 0.0f); break;
	case Kleft: pan(-view.w / 4.0f, 0.0f); break;
	case Kscrlup: zoom(5.0f, 5.0f); break;
	case Kscrldn: zoom(-5.0f, -5.0f); break;
	case Kesc: resetprompt(); reqdraw(Reqresetui); break;
	case 'r':
		for(g=graphs; g<graphs+dylen(graphs); g++)
			reqlayout(g, Lreset);
		break;
	case 'p':
		for(g=graphs; g<graphs+dylen(graphs); g++){
			if((g->flags & GFdrawme) != 0)
				reqlayout(g, Lstop);
			else
				reqlayout(g, Lstart);
		}
		break;
	case 'a': reqdraw(Reqshape); break;
	case 'l': view.flags ^= VFdrawlabels; reqdraw(Reqshallowdraw); break;
	case '\n': prompt(r); break;
	}
	return 0;
}

void
showobject(char *s)
{
	strecpy(hoverstr, hoverstr+sizeof hoverstr, s);
}

/* FIXME: make clear that this is called from a different thread: req...
 *	or maybe even its own file */
static int
mousedrag(float Δx, float Δy)
{
	RNode *r;

	if(selected == -1 || (selected & 1<<31) != 0)
		return 0;
	Δx /= view.w;
	Δy /= view.h;
	Δx = 2 * Δx * view.Δeye.z * view.ar * view.tfov;
	Δy = 2 * Δy * view.Δeye.z * view.tfov;
	r = rnodes + selected - 1;
	r->pos[0] += Δx;
	r->pos[1] -= Δy;
	return 1;
}

static void
getinfo(ioff id)
{
	if((id & (1<<31)) == 0){
		id = (uint)id - 1;
		pushcmd("nodeinfo(%d)", id);
	}else{
		id = (uint)(id & ~(1<<31)) - 1;
		pushcmd("edgeinfo(%d)", id);
	}
}

static ioff
mousehover(int x, int y)
{
	uint v;

	if(x < 0 || y < 0 || x >= view.w || y >= view.h
	|| (v = mousepick(x, y)) == -1){
		hoverstr[0] = 0;
		return -1;
	}
	if(v == shown)
		return v;
	getinfo(v);
	return v;
}

static void
unhighlightnode(ioff id)
{
	pushcmd("print \"c\", %d, CL[label[%d]]", id, id);
}

/* FIXME: plan9 */
static void
highlightnode(ioff id)
{
	mixcolors(rnodes[id].col, theme[Chigh] >> 8);
}

static int
mouseselect(ioff id)
{
	char *p;

	if(selected == id)
		return 0;
	if((selected & 1<<31) == 0)
		unhighlightnode(selected - 1);
	if((selected = id) != -1){
		p = strecpy(selstr, selstr+sizeof selstr, "Selected: ");
		strecpy(p, selstr+sizeof selstr, hoverstr);
		if((id & 1<<31) == 0)
			highlightnode(id - 1);
		reqdraw(Reqshallowdraw);
		return 1;
	}
	selstr[0] = 0;
	return 0;
}

void
focusnode(ioff i)
{
	focused = i;
}

/* FIXME: naming and shit is stupid; view.pan/zoom are only for plan9
 *	should instead convert to world coordinates *here* and request an
 *	update, which *also* can be here, it just all updates view, and
 *	the mvp, so either in draw/world.c, or ui/view.c or something */
/* won't touch selection */
void
focusobj(void)
{
	RNode *r;

	if(focused == -1 || (focused & (1<<31)) != 0)	/* unimplemented */
		return;
	r = rnodes + focused;
	worldview(V(r->pos[0], r->pos[1], r->pos[2] + 10.0f));
	mousehover(view.w / 2, view.h / 2);	/* FIXME: hack, unreliable */
	mouseselect(focused + 1);
}

int
mouseevent(Vertex v, Vertex Δ)
{
	int m;
	static int omod, inwin;

	m = mod & Mmask;
	if(vinrect(v, view.prompt) || m != 0 && inwin){
		inwin = 1;
		goto nope;
	}else
		inwin = 0;
	if(m == 0 || (omod & Mrmb) == 0)
		center = V(v.x - view.w / 2, v.y - view.h / 2, 0);
	if(Δ.x != 0.0 || Δ.y != 0.0)
		shown = mousehover(v.x, v.y);
	if(m == Mlmb){
		if((omod & Mlmb) == 0){
			mouseselect(shown);
		}else if(Δ.x != 0.0 || Δ.y != 0.0){
			if(mousedrag(Δ.x, Δ.y))
				reqdraw(Reqredraw);
		}
	}else if(m == Mrmb){
		if((mod & Mctrl) != 0)
			zoom(-Δ.x, -Δ.y);
		else
			pan(-Δ.x, -Δ.y);
	}else if(m == Mmmb)
		rotate(-Δ.x, -Δ.y);
	else if(m == (Mlmb | Mrmb))
		zoom(-Δ.x, -Δ.y);
nope:
	omod = m;
	return 0;
}

void
resetui(void)
{
	view.center = ZV;
	view.θ = 0.0f;
	view.φ = 0.0f;
	view.eye = V(0.0f, 0.0f, 80.0f);
	view.up = V(0.0f, 1.0f, 0.0f);
	view.Δeye = subv(view.eye, view.center);
	view.pan = ZV;
	view.zoom = 1.0;
}

void
initui(void)
{
	initsysui();
	view.fov = 45.0f;
	view.tfov = tanf(view.fov / 2);
	resetui();
}
