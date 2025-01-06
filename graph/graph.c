#include "strpg.h"
#include "graph.h"
#include "threads.h"
#include "cmd.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"

Graph *graphs;

static RWLock *locks;

int
setattr(Node *n, ioff id, int type, char *val)
{
	int i;
	char *p;
	u32int u;
	float f;

	switch(type){
	case TLN:
		i = atoi(val);
		if(i < 0)
			logerr(va("warning: nonsense segment length %d\n", i));
		if(n->attr.length > 0){
			if(n->attr.length != i)
				logerr(va("warning: segment length already set to %d != %d\n",
					n->attr.length, i));
			break;
		}
		n->attr.length = i;
		if(i <= 0)
			break;
		if(drawing.length.min > i){
			drawing.length.min = i;
			drawing.flags |= DFstalelen;
		}
		if(drawing.length.max < i){
			drawing.length.max = i;
			drawing.flags |= DFstalelen;
		}
		break;
	case TCL:
		if(val[0] == '#'){
			val++;
			u = strtoul(val, &p, 16);
			if(p <= val + 6)
				u = u << 8 | 0xc0;
		}else
			u = strtoul(val, &p, 0);
		if(p == val)	/* assuming string value, let strawk interpret it */
			return 0;
		setcolor(rnodes[id].col, u);
		n->attr.color = u;
		break;
	case Tfx:
		n->attr.flags |= FNfixedx;
		/* wet floor */
	case Tx0:
		f = atof(val);
		n->attr.pos0.x = f;
		n->attr.flags |= FNinitx;
		if(f < drawing.xbound.min)
			drawing.xbound.min = f;
		if(f > drawing.xbound.max)
			drawing.xbound.max = f;
		break;
	case Tfy:
		n->attr.flags |= FNfixedy;
		/* wet floor */
	case Ty0:
		f = atof(val);
		n->attr.pos0.y = f;
		n->attr.flags |= FNinity;
		if(f < drawing.ybound.min)
			drawing.ybound.min = f;
		if(f > drawing.ybound.max)
			drawing.ybound.max = f;
		break;
	case Tfz:
		n->attr.flags |= FNfixedz;
		/* wet floor */
	case Tz0:
		f = atof(val);
		n->attr.pos0.z = atof(val);
		n->attr.flags |= FNinitz;
		if(f < drawing.zbound.min)
			drawing.zbound.min = f;
		if(f > drawing.zbound.max)
			drawing.zbound.max = f;
		break;
	default:
		return 0;
	}
	return 1;
}

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

void
lockgraph(Graph *g, int w)
{
	RWLock *l;

	l = locks + (g - graphs);
	if(w)
		wlock(l);
	else
		rlock(l);
}

void
unlockgraph(Graph *g, int w)
{
	RWLock *l;

	l = locks + (g - graphs);
	if(w)
		wunlock(l);
	else
		runlock(l);
}

Graph *
pushgraph(Graph *g)
{
	RWLock l;

	dypush(graphs, *g);
	g = graphs + dylen(graphs) - 1;
	newlayout(g, -1);
	memset(&l, 0, sizeof l);
	dypush(locks, l);
	return g;
}

void
initgraph(Graph *g, int type)
{
	memset(g, 0, sizeof *g);
	g->type = type;
}
