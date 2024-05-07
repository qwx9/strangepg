#include "strpg.h"
#include "ui.h"
#include "cmd.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"
#include "threads.h"

Obj selected;
int prompting;

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
	DPRINT(Debugdraw, "zoom %.1f (%.1f,%.1f) → %.2f ", Δ, Δx, Δy, view.zoom);
	zoomdraw(Δ);
	reqdraw(Reqredraw);
}

int
keyevent(Rune r)
{
	Graph *g;

	DPRINT(Debugdraw, "keyevent %d", r);
	if(prompting){
		prompt(r);
		return 0;
	}
	switch(r){
	case KBup: pan(0.0f, -view.w / 2.0f); break;
	case KBdown: pan(0.0f, +view.h / 2.0f); break;
	case KBright: pan(+view.w / 2.0f, 0.0f); break;
	case KBleft: pan(-view.w / 2.0f, 0.0f); break;
	case KBescape: resetprompt(); reqdraw(Reqresetui); break;
	/* FIXME: doesn't quite make sense */
	case '+': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, 1); unlockgraphs(0); break;
	case '-': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) zoomgraph(g, -1); unlockgraphs(0); break;
	case 'r': lockgraphs(0); for(g=graphs; g<graphs+dylen(graphs); g++) resetlayout(g); unlockgraphs(0); break;
	case 'a': view.flags ^= VFdrawarrows; reqdraw(Reqredraw); break;
	case 'l': view.flags ^= VFdrawlabels; reqdraw(Reqredraw); break;
	default: prompt(r); break;
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
		selected = mouseselect(v.x, v.y);
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
