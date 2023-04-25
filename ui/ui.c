#include "strpg.h"

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
zoomview(Vector v)
{
	double Δ;

	/* scalar projection of v onto (1,1); so, view.zoom in when dragging ↘ */
	Δ = 0.01 * -(v.x + v.y) / 2;
	dprint("view.zoomview %.1f,%.1f → Δ %.2f\n", v.x, v.y, Δ);
	if(view.zoom + Δ < 0.1 || view.zoom + Δ > 10)
		return -1;
	view.zoom += Δ;
	return 0;
}

int
keyevent(Rune r)
{
	switch(r){
	case K↑: if(panview(Vec2(0,+16)) >= 0) shallowdraw(); break;
	case K↓: if(panview(Vec2(0,-16)) >= 0) shallowdraw(); break;
	case K→: if(panview(Vec2(-16,0)) >= 0) shallowdraw(); break;
	case K←: if(panview(Vec2(+16,0)) >= 0) shallowdraw(); break;
	default: break;	// FIXME: cmd(r)
	}
	return 0;
}

/* the very first mouse event will have non-sense deltas */
int
mouseevent(Vertex v, Vertex Δ, int b)
{
	USED(v);
	if((b & 7) == Mlmb){
		// FIXME: select
		// FIXME: drag → move
	}else if((b & 7) == Mmmb){
		// FIXME: menu
	}else if((b & 7) == Mrmb){
		if(panview(subpt2(ZV, Δ)) >= 0){
			dprint("pan: %s\n", vertfmt(&view.dim.o));
			shallowdraw();
		}
	}else if((b & 7) == (Mlmb | Mrmb)){
		if(zoomview(subpt2(ZV, Δ)) >= 0){
			dprint("pan: %s\n", vertfmt(&view.dim.o));
			redraw();
		}
	}
	return 0;
}

void
resetui(void)
{
	view.dim.o = ZV;
	if(view.zoom == 0.0)
		view.zoom = 1.0;
	panmax = view.dim.v;
	//assert(!eqpt2(panmax, ZV));
}
