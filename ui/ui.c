#include "strpg.h"

static Vertex panmax;

int
panview(Vertex p)
{
	p.x = view.pan.x - p.x;
	p.y = view.pan.y - p.y;
	if(p.x < -panmax.x / 2)
		p.x = -panmax.x / 2;
	else if(p.x > panmax.x / 2)
		p.x = panmax.x / 2;
	if(p.y < 0)
		p.y = 0;
	else if(p.y > panmax.y)
		p.y = panmax.y;
	if(eqv(p, view.pan)){
		view.pan = p;
		return 0;
	}
	return -1;
}

void
resetui(void)
{
	view.pan = ZV;
	view.zoom = 1.0;
	panmax = (Vertex){view.w, view.h};
}

void
initui(void)
{
	resetui();
}
