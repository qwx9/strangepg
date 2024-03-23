#include "strpg.h"
#include "cmd.h"
#include "threads.h"

Obj selected;

static char *prompt;
static Vertex panmax;

int
panview(Vector v)
{
	DPRINT(Debugdraw, "panview to %.2f,%.2f", v.x, v.y);
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
	DPRINT(Debugdraw, "view.zoomview %.1f,%.1f → Δ %.2f: ", v.x, v.y, Δ);
	if(view.zoom + Δ < 0.1 || view.zoom + Δ > 10){
		DPRINT(Debugdraw, "nope");
		return -1;
	}
	view.zoom += Δ;
	DPRINT(Debugdraw, "%.1f", view.zoom);
	return 0;
}

static void
keyprompt(Rune r)
{
	if((prompt = enterprompt(r, prompt)) == nil)
		return;
	pushcmd("%s", prompt);
}

int
keyevent(Rune r)
{
	Graph *g;

	DPRINT(Debugdraw, "keyevent %d", r);
	switch(r){
	case KBup: if(panview(Vec2(0,+16)) >= 0) reqdraw(Reqredraw); break;
	case KBdown: if(panview(Vec2(0,-16)) >= 0) reqdraw(Reqredraw); break;
	case KBright: if(panview(Vec2(-16,0)) >= 0) reqdraw(Reqredraw); break;
	case KBleft: if(panview(Vec2(+16,0)) >= 0) reqdraw(Reqredraw); break;
	case KBescape: free(prompt); prompt = nil; reqdraw(Reqresetui); break;
	case '\n': keyprompt(0);  reqdraw(Reqresetui); break;
	/* FIXME: doesn't quite make sense */
	case '+': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, 1); unlockgraphs(0); break;
	case '-': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, -1); unlockgraphs(0); break;
	case 'R': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) resetlayout(g); unlockgraphs(0); break;
	case 'a': showarrows ^= 1; reqdraw(Reqredraw); break;
	case 'r': norefresh ^= 1; break;
	default: keyprompt(r); break;
	}
	return 0;
}

/* the very first mouse event will have nonsense deltas */
int
mouseevent(Vertex v, Vertex Δ, int b)
{
	Obj o;

	if((b & 7) == Mlmb){
		// FIXME: detect out of focus and click twice?
		// FIXME: drag → move (in draw fsm)
		o = selected;
		// FIXME: everything should stop while this does its thing
		//rlock(&drawlock);
		selected = mouseselect(v);
		if(selected.type == Onode && o.g != nil && o.g->c != nil){
			if(memcmp(&selected, &o, sizeof o) == 0){
				assert(o.idx < dylen(o.g->nodes));
				stoplayout(o.g);
				expandnode(o.g, o.g->nodes + o.idx);
				selected = aintnothingthere;
				updatelayout(o.g);
			}
		}
		reqdraw(Reqshallowdraw);
		//runlock(&drawlock);
	}else if((b & 7) == Mmmb){
		// FIXME: menu
	}else if((b & 7) == Mrmb){
		if(panview(subpt2(ZV, Δ)) >= 0)
			reqdraw(Reqredraw);
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
	DPRINT(Debugdraw, "resetui center %.1f,%.1f", view.center.x, view.center.y);
	panmax = view.dim.v;
	if(all){
		view.pan = ZV;
		view.zoom = 1.0;
	}
}

void
initui(void)
{
	initsysui();
}
