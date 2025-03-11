#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"

Graph graph;
Node *nodes;
ioff *edges;

void
explode(ioff id)
{
	RNode *r;

	r = rnodes + id;
	r->pos[0] += 32.0f * (0.5f - xfrand());
	r->pos[1] += 32.0f * (0.5f - xfrand());
	r->pos[2] += 32.0f * (0.5f - xfrand());
}

ioff
getrealid(ioff id, int isedge)
{
	if(isedge){
		/* FIXME: use id as is and have awk print this edge instead of indexing redges;
		 * in other words maybe get rid of edges[] and shit, right now they're unusable
		 * anyway */
		return id >= dylen(redges) ? -1 : id;
	}
	if(id < 0 || id >= dylen(nodes))
		sysfatal("getrealid: %d out of bounds %d", id, dylen(nodes));
	return nodes[id].id;
}

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
initnodes(ioff nnodes, int *len, ioff *off, ushort *deg)
{
	int i;
	vlong nn;
	Node *n, *ne;
	V v;

	nn = dylen(nodes);	/* for mooltigraph */
	dyresize(nodes, (nn + nnodes));
	dyresize(rnodes, (nn + nnodes));
	for(i=nn, n=nodes+nn, ne=n+nnodes; n<ne; n++, i++){
		n->id = i;
		n->nedges = *deg++;
		n->eoff = *off++;
		v.i = *len++;
		setspectag(TLN, i, v);
	}
}
