#include "strpg.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "threads.h"

Obj selected;

static char *prompt;

static void
pan(float Δx, float Δy)
{
	Vertex v;

	/* FIXME */
	v = (Vertex){Δx, Δy, 0.0f};
	v = addpt2(v, view.pan);
	if(eqpt2(v, view.pan))
		return;
	view.pan = v;
	DPRINT(Debugdraw, "pan %.2f,%.2f → %.2f,%.2f", v.x, v.y, view.pan.x, view.pan.y);
	pandraw(Δx, Δy);
	reqdraw(Reqredraw);
}

static void
zoom(float Δx, float Δy)
{
	float Δ;

	/* scalar projection of v onto (1,1) unit vector; this way,
	 * zoom in when dragging ↘; also scale to reasonable
	 * increment */
	Δ = 0.01 * -(Δx + Δy) / 2;
	if(view.zoom + Δ == view.zoom)
		return;
	view.zoom += Δ;
	DPRINT(Debugdraw, "zoom %.1f,%.1f → Δ %.2f → %.2f ", Δx, Δy, Δ, view.zoom);
	zoomdraw(Δ);
	reqdraw(Reqredraw);
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
	case KBup: pan(0.0f, -view.dim.v.y / 2.0f); break;
	case KBdown: pan(0.0f, +view.dim.v.y / 2.0f); break;
	case KBright: pan(+view.dim.v.x / 2.0f, 0.0f); break;
	case KBleft: pan(-view.dim.v.x / 2.0f, 0.0f); break;
	case KBescape: free(prompt); prompt = nil; reqdraw(Reqresetui); break;
	case '\n': keyprompt(0);  reqdraw(Reqresetui); break;
	/* FIXME: doesn't quite make sense */
	case '+': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, 1); unlockgraphs(0); break;
	case '-': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, -1); unlockgraphs(0); break;
	case 'R': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) resetlayout(g); unlockgraphs(0); break;
	case 'a': showarrows ^= 1; reqdraw(Reqredraw); break;
	case 'r': norefresh ^= 1; break;
	case 'l': drawlabels ^= 1; break;
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
				if(stoplayout(o.g) < 0)
					warn("mouseevent: %s\n", error());
				expandnode(o.g, o.g->nodes + o.idx);
				selected = aintnothingthere;
				updatelayout(o.g);
			}
		}
		reqdraw(Reqshallowdraw);
		//runlock(&drawlock);
	}else if((b & 7) == Mmmb){
		// FIXME: menu
	}else if((b & 7) == Mrmb)
		pan(-Δ.x, -Δ.y);
	else if((b & 7) == (Mlmb | Mrmb))
		zoom(-Δ.x, -Δ.y);
	return 0;
}

void
resetui(int all)
{
	view.center = ZV;
	DPRINT(Debugdraw, "resetui center %.1f,%.1f", view.center.x, view.center.y);
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
