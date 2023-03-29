#include "strpg.h"
#include "uiprv.h"

float zoom;

static Vertex panmax;

int
panview(Vertex p)
{
	p = addvx(p, view.pan);
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
	if(!eqvx(p, view.pan)){
		view.pan = p;
		return 0;
	}
	return -1;
}

int
keyevent(Rune r)
{
	switch(r){
	case K↑: if(panview(Vx(0,+16)) >= 0) redraw(graph); break;
	case K↓: if(panview(Vx(0,-16)) >= 0) redraw(graph); break;
	case K→: if(panview(Vx(-16,0)) >= 0) redraw(graph); break;
	case K←: if(panview(Vx(+16,0)) >= 0) redraw(graph); break;
	default: break;	// FIXME: cmd(r)
	}
	return 0;
}

int
mouseevent(Vertex v, Vertex Δ, int b)
{
	dprint("mouseevent %ux %d,%d +%d,%d\n", b, v.x, v.y, Δ.x, Δ.y);
	if((b & Mlmb) == Mlmb){
		// FIXME: select
		// FIXME: drag → move
	}else if((b & Mmmb) == Mmmb){
		// FIXME: menu
	}else if((b & Mrmb) == Mrmb){
		if(panview(subvx(ZV, Δ)) >= 0){
			dprint("pan: %d,%d\n", view.pan.x, view.pan.y);
			redraw(graph);
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
	view.pan = ZV;
	view.zoom = 1.0;
	panmax = (Vertex){view.w, view.h};
	assert(!eqvx(panmax, ZV));
}

void
initui(void)
{
	resetdraw(graph);
	initui_();
	resetui();
}
