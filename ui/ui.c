#include "strpg.h"

static Vertex panmax;

int
panview(Vector v)
{
	dprint("panview to %.2f,%.2f\n", v.x, v.y);
	//v = floorpt2(addpt2(divpt2(v, view.zoom), view.pan));
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
	dprint("view.zoomview %.1f,%.1f → Δ %.2f: ", v.x, v.y, Δ);
	if(view.zoom + Δ < 0.1 || view.zoom + Δ > 10){
		dprint("nope\n");
		return -1;
	}
	view.zoom += Δ;
	dprint("%.1f\n", view.zoom);
	return 0;
}

// FIXME: turn into commands?
int
keyevent(Rune r)
{
	Graph *g;

	dprint("keyevent %d\n", r);
	switch(r){
	case KBup: if(panview(Vec2(0,+16)) >= 0) reqdraw(Reqredraw); break;
	case KBdown: if(panview(Vec2(0,-16)) >= 0) reqdraw(Reqredraw); break;
	case KBright: if(panview(Vec2(-16,0)) >= 0) reqdraw(Reqredraw); break;
	case KBleft: if(panview(Vec2(+16,0)) >= 0) reqdraw(Reqredraw); break;
	case KBescape: reqdraw(Reqresetui); break;
	/* FIXME: doesn't quite make sense */
	case '+': for(g=graphs; g<graphs+ngraphs; g++) chlevel(g, g->level+1); break;
	case '-': for(g=graphs; g<graphs+ngraphs; g++) chlevel(g, g->level-1); break;
	case 'R': for(g=graphs; g<graphs+ngraphs; g++) resetlayout(g); break;
	case 'a': showarrows ^= 1; reqdraw(Reqredraw); break;
	default: break;
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
	// FIXME: should be the other way around, this should happen in the draw
	// proc
	}else if((b & 7) == Mrmb){
		if(panview(subpt2(ZV, Δ)) >= 0)
			reqdraw(Reqredraw);
	// FIXME: should be the other way around, this should happen in the draw
	// proc
	}else if((b & 7) == (Mlmb | Mrmb)){
		if(zoomview(subpt2(ZV, Δ)) >= 0)
			reqdraw(Reqredraw);
	}
	return 0;
}

void
resetui(int all)
{
	view.center = divpt2(view.dim.v, 2);
	dprint("resetui center %.1f,%.1f\n", view.center.x, view.center.y);
	panmax = view.dim.v;
	if(all){
		view.pan = ZV;
		view.zoom = 1.0;
	}
}
