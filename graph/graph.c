#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"

Graph *graphs;
Node *nodes;
ioff *edges;
Super *supers;

void
setattr(int type, ioff id, V val)
{
	Node *n;
	RNode *r;

	n = nodes + id;
	r = rnodes + id;
	switch(type){
	case TLN:
		if(r->len > 0.0f || val.i == 0)
			break;
		if(val.i < 0){
			logerr(va("setattr: nonsense segment length %d\n", val.i));
			break;
		}
		r->len = val.i;
		if(drawing.length.min == 0.0f || drawing.length.min > val.i){
			drawing.length.min = val.i;
			drawing.flags |= DFstalelen;
		}
		if(drawing.length.max < val.i){
			drawing.length.max = val.i;
			drawing.flags |= DFstalelen;
		}
		break;
	case TCL:
		setcolor(r->col, val.u);
		break;
	case Tfx:
		n->flags |= FNfixedx;
		/* wet floor */
	case Tx0:
		n->pos0.x = val.f;
		n->flags |= FNinitx;
		if(val.f < drawing.xbound.min)
			drawing.xbound.min = val.f;
		if(val.f > drawing.xbound.max)
			drawing.xbound.max = val.f;
		break;
	case Tfy:
		n->flags |= FNfixedy;
		/* wet floor */
	case Ty0:
		n->pos0.y = val.f;
		n->flags |= FNinity;
		if(val.f < drawing.ybound.min)
			drawing.ybound.min = val.f;
		if(val.f > drawing.ybound.max)
			drawing.ybound.max = val.f;
		break;
	case Tfz:
		n->flags |= FNfixedz;
		/* wet floor */
	case Tz0:
		n->pos0.z = val.f;
		n->flags |= FNinitz;
		if(val.f < drawing.zbound.min)
			drawing.zbound.min = val.f;
		if(val.f > drawing.zbound.max)
			drawing.zbound.max = val.f;
		break;
	}
}

void
explode(ioff id)
{
	RNode *r;

	r = rnodes + id;
	r->pos[0] += 32.0f * (0.5f - xfrand());
	r->pos[1] += 32.0f * (0.5f - xfrand());
	r->pos[2] += 32.0f * (0.5f - xfrand());
}

/* FIXME */
/* usable once topology has been loaded */
ioff
str2idx(char *s)
{
	ioff id;
	char *t;

	if((id = strtoll(s, &t, 0)) == 0 && t == s)
		return -1;
	return id;
}

Graph *
initgraph(int type)
{
	int n;
	Graph g = {0}, *gp;

	n = dylen(graphs);
	g.type = type;
	g.nfirst = dylen(nodes);
	dypush(graphs, g);
	gp = graphs + n;
	return gp;
}
