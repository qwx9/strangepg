#include "strpg.h"
#include "lib/khashl.h"
#include "lib/HandmadeMath.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "view.h"
#include "graph.h"
#include "layout.h"
#include "ui.h"

int prompting;
char hoverstr[256], selstr[512];

/* FIXME: horrible */
KHASHL_SET_INIT(KH_LOCAL, sel, sel, ioff, kh_hash_uint32, kh_eq_generic)
static sel *sels;
static ioff selected = -1, shown = -1, focused = -1;

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
static HMM_Vec3 center;

static void
pan(float Δx, float Δy)
{
	if(Δx == 0.0f && Δy == 0.0f)
		return;
	DPRINT(Debugui, "pan %.2f,%.2f", Δx, Δy);
	pandraw(Δx, Δy);
}

static void
zoom(float Δx, float Δy)
{
	float Δ;
	HMM_Vec3 v;

	if(Δx == 0.0f && Δy == 0.0f)
		return;
	/* scalar projection of v onto (1,1) unit vector; this way,
	 * zoom in when dragging ↘; also scale to reasonable
	 * increment */
	Δ = 0.01 * -(Δx + Δy) / 2;
	if(view.zoom + Δ == view.zoom)
		return;
	/* FIXME: not here */
	DPRINT(Debugui, "zoom %.1f (%.1f,%.1f) → %.2f ", Δ, Δx, Δy, view.zoom);
	v = HMM_MulV3F(center, Δ);
	zoomdraw(Δ, v.X, v.Y);
}

int
keyevent(Rune r, int down)
{
	int m;

	DPRINT(Debugui, "keyevent %d", r);
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
	case Kesc: resetprompt(); reqdraw(Reqresetview); break;
	case 'r': reqlayout(Lreset); break;
	case 'p': reqlayout(graph.flags & GFdrawme ? Lstop : Lstart); break;
	case 'a': reqdraw(Reqshape); break;
	case '\n': prompt(r); break;
	}
	return 0;
}

static void
clearsel(void)
{
	sel_clear(sels);	/* FIXME: shrink? */
}

static void
commitsel(void)
{
	if(kh_size(sels) > 0){
		pushcmd("showselected()");
		flushcmd();
	}
}

static void
resetselbox(int x, int y)
{
	selbox.x1 = selbox.x2 = x;
	selbox.y1 = selbox.y2 = y;
}

/* FIXME: this all SUCKS */
/* x1,y1----
 *  |   ↘  |
 *  |___ x2,y2 */
static int
dragselect(int x, int y)
{
	int abs;
	ioff id, idx, Δx, Δy, oid;
	struct {
		int x1;
		int x2;
		int y1;
		int y2;
	} rx, ry;
	union{
		ioff i;
		u32int u;
	} u;

	Δx = x - selbox.x1;
	Δy = y - selbox.y1;
	if(Δx <= 0 || Δy <= 0)
		return 0;
	x = selbox.x1 + Δx;
	y = selbox.y1 + Δy;
	if(selbox.x1 == x && selbox.y1 == y)
		return 0;
	rx.x1 = selbox.x1;
	rx.x2 = x;
	rx.y1 = selbox.y2;
	rx.y2 = y;
	ry.x1 = selbox.x2;
	ry.x2 = x;
	ry.y1 = selbox.y1;
	ry.y2 = selbox.y2;
	oid = -1;
	selbox.x2 = x;
	selbox.y2 = y;
	for(x=rx.x1; x<rx.x2; x++)
		for(y=rx.y1; y<rx.y2; y++){
			u.u = mousepick(x, y);
			if(u.i >= 0){
				if((idx = u.i) == oid)
					continue;
				sel_put(sels, idx, &abs);
				if(!abs)
					continue;
				if((id = getrealid(idx)) < 0){
					warn("dragselect: %s\n", error());
					continue;
				}
				pushcmd("selectnodebyid(%d,1)", id);
				highlightnode(idx);
				oid = idx;
			}
		}
	flushcmd();
	for(x=ry.x1; x<ry.x2; x++)
		for(y=ry.y1; y<ry.y2; y++){
			u.u = mousepick(x, y);
			if(u.i >= 0){
				if((idx = u.i) == oid)
					continue;
				sel_put(sels, idx, &abs);
				if(!abs)
					continue;
				if((id = getrealid(idx)) < 0){
					warn("dragselect: %s\n", error());
					continue;
				}
				pushcmd("selectnodebyid(%d,1)", id);
				highlightnode(idx);
				oid = idx;
			}
		}
	flushcmd();
	reqdraw(Reqrefresh);
	return 0;
}

static int
mousedrag(float Δx, float Δy)
{
	ioff idx;
	float d;
	RNode *r;
	HMM_Vec3 v;

	if((selected & 1UL<<31) != 0)
		return 0;
	Δx /= view.w;
	Δy /= view.h;
	//kh_foreach(sels, k){
	//	idx = kh_key(sels, k);
	idx = selected;
	r = rnodes + idx;
	/* FIXME: not here */
	if((drawing.flags & DF3d) == 0){
		Δx = 2 * Δx * view.Δeye.Z * view.ar * view.tfov;
		Δy = 2 * Δy * view.Δeye.Z * view.tfov;
		r->pos[0] += Δx;
		r->pos[1] -= Δy;
	}else{
		v = HMM_V3(r->pos[0], r->pos[1], r->pos[2]);
		v = HMM_SubV3(view.eye, v);
		d = HMM_LenV3(v);
		Δx *= d * view.ar * view.tfov;
		Δy *= d * view.tfov;
		/* FIXME: mul + sub */
		r->pos[0] += Δx * view.right.X - Δy * view.up.X;
		r->pos[1] += Δx * view.right.Y - Δy * view.up.Y;
		r->pos[2] += Δx * view.right.Z - Δy * view.up.Z;
	}
	reqdraw(Reqredraw);
	return 1;
}

static ioff
mousehover(int x, int y)
{
	ioff id, idx;
	int isedge;
	union{
		ioff i;
		u32int u;
	} u;

	DPRINT(Debugui, "mousehover %d,%d win [%d,%d]", x, y, view.w, view.h);
	if(x < 0 || y < 0 || x >= view.w || y >= view.h)
		return -1;
	u.u = mousepick(x, y);
	DPRINT(Debugui, "mousehover %d,%d: %x", x, y, u.u);
	if(u.i == -1){
		hoverstr[0] = 0;
		return -1;
	}
	if(u.i == shown)
		return u.i;
	isedge = u.u & 1UL<<31;
	idx = u.u & ~(1UL<<31);
	if(isedge)
		pushcmd("edgeinfo(%d)", idx);	/* untranslated to avoid lookup lag */
	else{
		/* FIXME: race between coarsening and ui mouse hover/select;
		 * box sel could add shit that is no longer there, etc. */
		if((id = getrealid(idx)) < 0){
			warn("mousehover: %s\n", error());
			return -1;
		}
		pushcmd("nodeinfo(%d)", id);
	}
	flushcmd();
	return u.i;
}

static int
mouseselect(ioff idx, int multi)
{
	int abs;
	ioff id;

	if(selected >= 0 && selected == idx && !multi || prompting)
		return 0;
	if(idx != -1){
		if((idx & 1UL<<31) == 0){
			sel_put(sels, idx, &abs);
			if((id = getrealid(idx)) < 0){
				warn("mouseselect: %s\n", error());
				return 0;
			}
			selected = idx;
			if(multi)
				pushcmd("toggleselect(%d)", id);
			else
				pushcmd("reselectnode(%d)", id);
			flushcmd();
			highlightnode(idx);
			updatenode(idx);
		}else{	/* FIXME: edges: not implemented */
			selected = idx;
		}
		return 1;
	}else
		selected = -1;
	if(!multi){
		clearsel();
		pushcmd("deselect()");
		flushcmd();
		selstr[0] = 0;
	}
	return 0;
}

void
focusobj(void)
{
	RNode *r;

	if(focused == -1 || (focused & (1UL<<31)) != 0)	/* unimplemented */
		return;
	r = rnodes + focused;
	worldview(HMM_V3(r->pos[0], r->pos[1], r->pos[2] + 10.0f));
	clearsel();
	mouseselect(focused, 0);
	resetselbox(view.w, view.h);
	reqdraw(Reqrefresh);
	focused = -1;
}

void
focusnode(ioff id)
{
	if((focused = getnodeidx(id)) < 0)
		return;
	reqdraw(Reqfocus);
}

/* FIXME: rotate with Mmmb */
int
mouseevent(float x, float y, float Δx, float Δy)
{
	int m;
	static int omod, inwin;
	khint_t k;

	if(Δx == 0.0f && Δy == 0.0f && mod == omod)	/* first event */
		return 0;
	DPRINT(Debugui, "mouseevent %f,%f Δ %f,%f mod %d", x, y, Δx, Δy, mod);
	m = mod & Mmask;
	/* FIXME: clean up */
	if(x >= promptbox.x1 && x < promptbox.x2
	&& y >= promptbox.y1 && y < promptbox.y2 || m != 0 && inwin){
		inwin = 1;
		goto nope;
	}else
		inwin = 0;
	if((omod & Mrmb) == 0)
		center = HMM_V3(x - view.w / 2, y - view.h / 2, 0);
	if(m == 0){
		endmove();	/* FIXME: doesn't really belong here? */
		if(omod == Mlmb){
			commitsel();
			resetselbox(x, y);
			reqdraw(Reqrefresh);
		}
		omod = 0;
	}else if(m != Mlmb && omod == Mlmb){
		resetselbox(x, y);
		reqdraw(Reqrefresh);
		omod = 0;
	}
	if(Δx != 0.0f || Δy != 0.0f)
		shown = mousehover(x, y);
	/* FIXME: maybe if clicking on empty space, check if there's a node
	 * very nearby first */
	/* FIXME: hold shift while dragselect -> super slow down on vnc */
	if(m == Mlmb){
		if(omod == 0){
			/* FIXME: multisel: we'd need to intersect two sets, a new
			 * sel and the old ones */
			k = sel_get(sels, shown);
			if(k == kh_end(sels) && (mod & (Mshift | Mctrl)) == 0)
				clearsel();
			resetselbox(x, y);
			mouseselect(shown, mod & (Mshift|Mctrl));
		}else if(omod == Mlmb && (Δx != 0.0f || Δy != 0.0f)){
			/* FIXME: would be nice to just redraw the one node */
			if(selected != -1)
				mousedrag(Δx, Δy);
			else
				dragselect(x, y);
		}
	}else if(m == Mrmb){
		if((mod & Mctrl) != 0)
			zoom(-Δx, -Δy);
		else
			pan(-Δx, -Δy);
	}else if(m == (Mlmb | Mrmb))
		zoom(-Δx, -Δy);
nope:
	omod |= m;
	return 0;
}

void
initui(void)
{
	if(drawing.flags & DFnope)
		return;
	initsysui();
	if((sels = sel_init()) == nil)
		sysfatal("initui: %s", error());
}
