#include "strpg.h"

float zoom;

static Vertex panmax;

int
panview(Vector v)
{
	dprint("panview to %.2f,%.2f\n", v.x, v.y);
	v = floorpt2(addpt2(v, view.pan));
	/* FIXME
	if(p.x < -panmax.x / 2)
		p.x = -panmax.x / 2;
	else if(p.x > panmax.x / 2)
		p.x = panmax.x / 2;
	if(p.y < 0)
		p.y = 0;
	else if(p.y > panmax.y)
		p.y = panmax.y;
	*/
	if(eqpt2(v, view.pan))
		return -1;
	view.pan = v;
	return 0;
}

int
keyevent(Rune r)
{
	switch(r){
	case K↑: if(panview(Vec2(0,+16)) >= 0) redraw(); break;
	case K↓: if(panview(Vec2(0,-16)) >= 0) redraw(); break;
	case K→: if(panview(Vec2(-16,0)) >= 0) redraw(); break;
	case K←: if(panview(Vec2(+16,0)) >= 0) redraw(); break;
	default: break;	// FIXME: cmd(r)
	}
	return 0;
}

/* the very first mouse event will have non-sense deltas */
int
mouseevent(Vertex v, Vertex Δ, int b)
{
	USED(v);
	if((b & Mlmb) == Mlmb){
		// FIXME: select
		// FIXME: drag → move
	}else if((b & Mmmb) == Mmmb){
		// FIXME: menu
	}else if((b & Mrmb) == Mrmb){
		if(panview(subpt2(ZV, Δ)) >= 0){
			dprint("pan: %s\n", vertfmt(&view.dim.o));
			redraw();
		}
	}else if((b & (Mlmb | Mrmb)) == Mlmb | Mrmb){
		// FIXME: zoom
	}
	// FIXME: possible: toggle floating centered mouse
	// FIXME: bound view, in two steps
	return 0;
}

void
resetui(void)
{
	view.dim.o = ZV;
	view.zoom = 1.0;
	panmax = view.dim.v;
	//assert(!eqpt2(panmax, ZV));
}
