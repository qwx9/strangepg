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
		DPRINT(Debuggraph, "[%d] id=%d eoff=%d ne=%d ln=%d",
			i, u->id, u->eoff, u->nedges, u->length);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			j = x >> 2;
			DPRINT(Debuggraph, "  <%zd> %08x %d%c%d%c",
				e-edges, x, i, x&1?'-':'+', j, x&2?'-':'+');
		}
	}
}

/* FIXME: protect while rnodes == nil? maybe queue commands up? */
void
explode(ioff id, float Δ)
{
	ioff idx;
	RNode *r;

	if((idx = getnodeidx(id)) < 0){
		DPRINT(Debuggraph, "explode: %s", error());
		return;
	}
	r = rnodes + idx;
	r->pos[0] += Δ * (0.5f - xfrand());
	r->pos[1] += Δ * (0.5f - xfrand());
	if(drawing.flags & DF3d)
		r->pos[2] += Δ * (0.5f - xfrand());
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

/* FIXME: none of these take coarsening into account */
static inline void
setz0(Node *u, float f)
{
	u->flags |= FNinitz;
	u->pos0.z = f;
	if(f < drawing.zbound.min)
		drawing.zbound.min = f;
	if(f > drawing.zbound.max)
		drawing.zbound.max = f;
}

static inline void
sety0(Node *u, float f)
{
	u->flags |= FNinity;
	u->pos0.y = f;
	if(f < drawing.ybound.min)
		drawing.ybound.min = f;
	if(f > drawing.ybound.max)
		drawing.ybound.max = f;
}

static inline void
setx0(Node *u, float f)
{
	u->flags |= FNinitx;
	u->pos0.x = f;
	if(f < drawing.xbound.min)
		drawing.xbound.min = f;
	if(f > drawing.xbound.max)
		drawing.xbound.max = f;
}

void
setnodeinitz(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	setz0(u, v.f);
}

void
setnodeinity(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	sety0(u, v.f);
}

void
setnodeinitx(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	setx0(u, v.f);
}

void
setnodefixedz(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	u->flags |= FNfixedz;
	setz0(u, v.f);
}

void
setnodefixedy(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	u->flags |= FNfixedy;
	sety0(u, v.f);
}

void
setnodefixedx(size_t id, Value v)
{
	ioff idx;
	Node *u;

	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	u->flags |= FNfixedx;
	setx0(u, v.f);
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

	DPRINT(Debugawk, "set LN[%s] ← %08zx", getname(id), v.u);
	if((idx = getnodeidx(id)) < 0)
		return;
	u = nodes + idx;
	/* FIXME: wrong place to check, this should check the
	 * LN table, nodes might already be collapsed by now */
	if(u->length != 0){
		if(u->length == v.u)
			return;
		else	/* FIXME: will be handled by ro values */
			DPRINT(Debuginfo, "LN[%s]: conflicting value %llu not %lld\n",
				getname(id), v.u, u->length);
	}
	updatenodelength(idx, v.u);
}

void
setnodecolor(size_t id, Value v)
{
	ioff idx;
	RNode *r;

	DPRINT(Debugawk, "set CL[%s] ← %08zx", getname(id), v.u);
	if((idx = getnodeidx(id)) < 0)
		return;
	if(rnodes == nil)	/* FIXME: come up with a better way */
		return;
	r = rnodes + idx;
	setcolor(r->col, setdefalpha(v.u));
	reqdraw(Reqshallowdraw);
}
