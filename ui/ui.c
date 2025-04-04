#include "strpg.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "graph.h"
#include "layout.h"
#include "ui.h"

int prompting;
char hoverstr[256], selstr[512];
REdge selbox[4];

/* FIXME: horrible */
KHASHL_SET_INIT(KH_LOCAL, sel, sel, ioff, kh_hash_uint32, kh_eq_generic)
static sel *sels;

static ioff selected = -1, shown = -1, focused = -1;
static Rekt rsel;

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

#define scr2world(x,y)	do{ \
	(x) = 2.0f * (((x) - view.w * 0.5f) / view.w) * view.Δeye.z * view.ar * view.tfov; \
	(y) = 2.0f * ((-(y) + view.h * 0.5f) / view.h) * view.Δeye.z * view.tfov; \
}while(0)

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
}

static void
rotate(float Δx, float Δy)
{
	if(Δx == 0.0f && Δy == 0.0f)
		return;
	rotdraw(Δx, Δy);
}

int
keyevent(Rune r, int down)
{
	int m;

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
	case 'r': reqlayout(Lreset); break;
	case 'p': reqlayout(graph.flags & GFdrawme ? Lstop : Lstart); break;
	case 'a': reqdraw(Reqshape); break;
	case 'l': drawing.flags ^= DFdrawlabels; reqdraw(Reqshallowdraw); break;
	case '\n': prompt(r); break;
	}
	return 0;
}

/* FIXME: separate pipeline, quads? */
/* FIXME: wrong viewpoint! */

static void
resetbox(void)
{
	REdge *r;

	if(rsel.x1 < 0)
		return;
	if(sels != nil){
		pushcmd("showselected()");
		flushcmd();
		sel_destroy(sels);
		sels = nil;
	}
	rsel.x1 = -1;
	r = selbox;
	r->pos1[0] = r->pos2[0];
	r->pos1[1] = r->pos2[1];
	r->pos1[2] = r->pos2[2];
	r++;
	r->pos1[0] = r->pos2[0];
	r->pos1[1] = r->pos2[1];
	r->pos1[2] = r->pos2[2];
	r++;
	r->pos1[0] = r->pos2[0];
	r->pos1[1] = r->pos2[1];
	r->pos1[2] = r->pos2[2];
	r++;
	r->pos1[0] = r->pos2[0];
	r->pos1[1] = r->pos2[1];
	r->pos1[2] = r->pos2[2];
	updateedges();
}

static inline void
selectionbox(float x1, float y1, float x2, float y2)
{
	float z;
	REdge *r;

	scr2world(x1, y1);
	scr2world(x2, y2);
	x1 += view.eye.x;
	y1 += view.eye.y;
	x2 += view.eye.x;
	y2 += view.eye.y;
	z = view.center.z;
	r = selbox;
	r->pos1[0] = x1;
	r->pos1[1] = y1;
	r->pos2[0] = x2;
	r->pos2[1] = y1;
	r->pos1[2] = r->pos2[2] = z;
	r++;
	r->pos1[0] = x2;
	r->pos1[1] = y1;
	r->pos2[0] = x2;
	r->pos2[1] = y2;
	r->pos1[2] = r->pos2[2] = z;
	r++;
	r->pos1[0] = x1;
	r->pos1[1] = y2;
	r->pos2[0] = x2;
	r->pos2[1] = y2;
	r->pos1[2] = r->pos2[2] = z;
	r++;
	r->pos1[0] = x1;
	r->pos1[1] = y1;
	r->pos2[0] = x1;
	r->pos2[1] = y2;
	r->pos1[2] = r->pos2[2] = z;
	updateedges();
}

/* FIXME: this all SUCKS */
/* x1,y1----
 *  |   ↘  |
 *  |___ x2,y2 */
static int
dragselect(int x, int y)
{
	int abs;
	ioff idx, Δx, Δy, oid;
	Rekt rx, ry;
	union{
		ioff i;
		u32int u;
	} u;

	if(rsel.x1 < 0){
		rsel.x1 = rsel.x2 = x;
		rsel.y1 = rsel.y2 = y;
	}
	Δx = x - rsel.x1;
	Δy = y - rsel.y1;
	if(Δx <= 0 || Δy <= 0)
		return 0;
	x = rsel.x1 + Δx;
	y = rsel.y1 + Δy;
	if(rsel.x1 == x && rsel.y1 == y)
		return 0;
	rx.x1 = rsel.x1;
	rx.x2 = x;
	rx.y1 = rsel.y2;
	rx.y2 = y;
	ry.x1 = rsel.x2;
	ry.x2 = x;
	ry.y1 = rsel.y1;
	ry.y2 = rsel.y2;
	oid = -1;
	rsel.x2 = x;
	rsel.y2 = y;
	if(sels == nil)
		sels = sel_init();
	for(x=rx.x1; x<rx.x2; x++)
		for(y=rx.y1; y<rx.y2; y++){
			u.u = mousepick(x, y);
			if(u.i >= 0){
				if((idx = u.i) == oid)
					continue;
				sel_put(sels, idx, &abs);
				if(!abs)
					continue;
				pushcmd("selectnodebyid(%d,1)", getrealid(idx));
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
				pushcmd("selectnodebyid(%d,1)", getrealid(idx));
				highlightnode(idx);
				oid = idx;
			}
		}
	flushcmd();
	selectionbox(rsel.x1, rsel.y1, rsel.x2, rsel.y2);
	return 0;
}

static int
mousedrag(float Δx, float Δy)
{
	ioff idx;
	float d;
	RNode *r;
	Vertex v;

	if(((idx = selected) & 1UL<<31) != 0)
		return 0;
	Δx /= view.w;
	Δy /= view.h;
	r = rnodes + idx;
	if((drawing.flags & DF3d) == 0){
		Δx = 2 * Δx * view.Δeye.z * view.ar * view.tfov;
		Δy = 2 * Δy * view.Δeye.z * view.tfov;
		r->pos[0] += Δx;
		r->pos[1] -= Δy;
	}else{
		v = V(r->pos[0], r->pos[1], r->pos[2]);
		v = subv(view.eye, v);
		d = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		Δx *= d * view.ar * view.tfov;
		Δy *= d * view.tfov;
		r->pos[0] += Δx * view.right.x - Δy * view.up.x;
		r->pos[1] += Δx * view.right.y - Δy * view.up.y;
		r->pos[2] += Δx * view.right.z - Δy * view.up.z;
	}
	updatenode(idx);
	return 1;
}

static ioff
mousehover(int x, int y)
{
	ioff idx;
	int isedge;
	union{
		ioff i;
		u32int u;
	} u;

	DPRINT(Debugdraw, "mousehover %d,%d win [%d,%d]", x, y, view.w, view.h);
	if(x < 0 || y < 0 || x >= view.w || y >= view.h)
		return -1;
	u.u = mousepick(x, y);
	DPRINT(Debugdraw, "mousehover %d,%d: %x", x, y, u.u);
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
	else
		pushcmd("nodeinfo(%d)", getrealid(idx));
	flushcmd();
	return u.i;
}

static int
mouseselect(ioff idx, int multi)
{
	ioff id;

	if(selected >= 0 && selected == idx && !multi || prompting)
		return 0;
	if((selected = idx) != -1){
		if((idx & 1UL<<31) == 0){
			id = getrealid(idx);
			if(multi)
				pushcmd("toggleselect(%d)", id);
			else
				pushcmd("reselectnode(%d)", id);
			flushcmd();
			highlightnode(idx);
			updatenode(idx);
		}else{	/* FIXME: edges: not implemented */
			;
		}
		return 1;
	}
	if(!multi){
		pushcmd("deselect()");
		flushcmd();
		selstr[0] = 0;
	}
	resetbox();
	return 0;
}

/* FIXME: naming and shit is stupid; view.pan/zoom are only for plan9
 *	should instead convert to world coordinates *here* and request an
 *	update, which *also* can be here, it just all updates view, and
 *	the mvp, so either in draw/world.c, or ui/view.c or something */
/* called from render thread */
void
focusobj(void)
{
	RNode *r;

	if(focused == -1 || (focused & (1UL<<31)) != 0)	/* unimplemented */
		return;
	r = rnodes + focused;
	worldview(V(r->pos[0], r->pos[1], r->pos[2] + 10.0f));
	mouseselect(focused, 0);
	focused = -1;
}

void
focusnode(ioff id)
{
	if((focused = getnodeidx(id)) < 0)
		return;
	reqdraw(Reqfocus);
}

int
mouseevent(Vertex v, Vertex Δ)
{
	int m;
	static int omod, inwin;

	DPRINT(Debugdraw, "mouseevent %f,%f Δ %f,%f mod %d", v.x, v.y, Δ.x, Δ.y, mod);
	m = mod & Mmask;
	if(vinrect(v, view.prompt) || m != 0 && inwin){
		inwin = 1;
		goto nope;
	}else
		inwin = 0;
	/* FIXME: clean up */
	if(m == 0){
		endmove();
		if((omod & Mmask) == Mlmb)
			resetbox();
		center = V(v.x - view.w / 2, v.y - view.h / 2, 0);
	}else if((omod & Mmask) == Mlmb && (m & Mmask) != Mlmb)
		resetbox();
	if((omod & Mrmb) == 0)
		center = V(v.x - view.w / 2, v.y - view.h / 2, 0);
	if(Δ.x != 0.0f || Δ.y != 0.0f)
		shown = mousehover(v.x, v.y);
	if(m == Mlmb){
		if((omod & Mlmb) == 0){
			if(!mouseselect(shown, mod & (Mshift|Mctrl)))
				dragselect(v.x, v.y);
		}else if(Δ.x != 0.0f || Δ.y != 0.0f){
			/* FIXME: would be nice to just redraw the one node */
			if(selected != -1)
				mousedrag(Δ.x, Δ.y);
			else
				dragselect(v.x, v.y);
		}
	}else if(m == Mrmb){
		if((mod & Mctrl) != 0)
			zoom(-Δ.x, -Δ.y);
		else
			pan(-Δ.x, -Δ.y);
	/* FIXME: 2d mode rotation */
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
	view.eye = V(0.0f, 0.0f, 100.0f);
	view.up = V(0.0f, 1.0f, 0.0f);
	view.right = V(1.0f, 0.0f, 0.0f);
	view.front = V(0.0f, 0.0f, 1.0f);
	view.Δeye = subv(view.eye, view.center);
	view.pan = ZV;
	view.zoom = 1.0;
}

void
initui(void)
{
	if(drawing.flags & DFnope)
		return;
	initsysui();
	view.fov = 45.0f;
	view.tfov = tanf(view.fov / 2);
	resetui();
}
