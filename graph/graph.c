#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "coarse.h"
#include "strawk/awk.h"
#include "var.h"

Graph graph;
Node *nodes;
ioff *edges;

void
printgraph(void)
{
	ioff i, j, x, *e, *ee;
	Node *u, *ue;

	if((debug & Debuggraph) == 0)
		return;
	DPRINT(Debuggraph, "current graph: %zd (%zd) nodes, %zd (%zd) edges",
		dylen(rnodes), dylen(nodes), dylen(redges), dylen(edges));
	for(i=0, u=nodes, ue=u+dylen(u); u<ue; u++, i++){
		DPRINT(Debuggraph, "[%d] id=%d off=%d ne=%d",
			i, u->id, u->eoff, u->nedges);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			DPRINT(Debuggraph, "  <%zd> %08x %d%c%d%c",
				e-edges, x, i, x&1?'-':'+', j, x&2?'-':'+');
		}
	}
}

void
explode(ioff idx)
{
	RNode *r;

	if(idx < 0 || idx >= dylen(rnodes))
		sysfatal("explode: out of bounds index: %d > %zd", idx, dylen(rnodes)-1);
	r = rnodes + idx;
	r->pos[0] += 32.0f * (0.5f - xfrand());
	r->pos[1] += 32.0f * (0.5f - xfrand());
	if(drawing.flags & DF3d)
		r->pos[2] += 32.0f * (0.5f - xfrand());
}

/* FIXME: this is a problem; we're also reencoding and decoding in
 * awk again */
/* bleh, our edge data structures aren't making lookups easy;
 * our list is unsorted, but there shouldn't be many objects on
 * screen in the end */
u64int
getrealedge(ioff idx)
{
	ioff e;
	u64int uv;
	Node *u, *v, *ue;

	idx = vedges[idx];	/* FIXME: horrible crutch */
	for(u=nodes, ue=u+dylen(u); u<ue; u++)
		if(idx >= u->eoff && idx < u->eoff + u->nedges)
			break;
	assert(u < ue);
	uv = (u64int)u->id << 32;
	e = edges[idx];
	v = nodes + (e >> 2);
	uv |= v->id << 2 | e & 3;
	return uv;
}

ioff
getrealid(ioff idx)
{
	if(idx < 0 || idx >= dylen(nodes)){
		werrstr("getrealid: %d out of bounds %zd", idx, dylen(nodes));
		return -1;
	}
	return nodes[idx].id;
}

int
setattr(int type, ioff id, V val)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return - 1;
	u = nodes + idx;
	switch(type){
	case Tfx:
		u->flags |= FNfixedx;
		/* wet floor */
	case Tx0:
		u->pos0.x = val.f;
		u->flags |= FNinitx;
		if(val.f < drawing.xbound.min)
			drawing.xbound.min = val.f;
		if(val.f > drawing.xbound.max)
			drawing.xbound.max = val.f;
		break;
	case Tfy:
		u->flags |= FNfixedy;
		/* wet floor */
	case Ty0:
		u->pos0.y = val.f;
		u->flags |= FNinity;
		if(val.f < drawing.ybound.min)
			drawing.ybound.min = val.f;
		if(val.f > drawing.ybound.max)
			drawing.ybound.max = val.f;
		break;
	case Tfz:
		u->flags |= FNfixedz;
		/* wet floor */
	case Tz0:
		u->pos0.z = val.f;
		u->flags |= FNinitz;
		if(val.f < drawing.zbound.min)
			drawing.zbound.min = val.f;
		if(val.f > drawing.zbound.max)
			drawing.zbound.max = val.f;
		break;
	}
	return 0;
}

void
updatenodelength(ioff idx, uint n)
{
	Node *u;

	u = nodes + idx;
	if(drawing.length.min == 0.0f || drawing.length.min > n){
		drawing.length.min = n;
		drawing.flags |= DFstalelen;
	}else if(drawing.length.min == u->length)
		drawing.flags |= DFstalelen | DFrecalclen;
	if(drawing.length.max < n){
		drawing.length.max = n;
		drawing.flags |= DFstalelen;
	}else if(drawing.length.max == u->length)
		drawing.flags |= DFstalelen | DFrecalclen;
	u->length = n;
}

void
setnodelength(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	if(u->length != 0){
		if(u->length == v.u)
			return;
		else	/* FIXME: will be handled by ro values */
			warn("LN[%s]: conflicting value %d not %d\n",
				getname(id), v.u, u->length);
	}
	updatenodelength(idx, v.u);
}

void
setnodecolor(size_t id, Value v)
{
	ioff idx;
	RNode *r;

	if((idx = getnodeidx(id)) < 0)
		return;
	r = rnodes + idx;
	setcolor(r->col, setdefalpha(v.u));
	reqdraw(Reqrefresh);
}
