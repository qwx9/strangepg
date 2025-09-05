#include "strpg.h"
#include "threads.h"
#include "graph.h"
#include "cmd.h"
#include <locale.h>
#include <signal.h>
#include "strawk/awk.h"

extern QLock symlock;

typedef struct Core Core;

struct Core{
	char **labels;
	u32int *colors;
	Array *ids;
	Array *label;
	Array *length;
	Array *degree;
	Array *color;
};
static Core core;

char *
getname(voff id)
{
	assert(id >= 0 && id < dylen(core.labels));
	return core.labels[id];
}

voff
getid(char *s)
{
	Cell *c;

	qlock(&symlock);
	c = lookup(s, core.ids);
	qunlock(&symlock);
	if(c == nil)
		return -1;
	else
		return getival(c);	/* FIXME: rather than c->val.i? */
}

voff
pushname(char *s)
{
	voff id;
	Value v;
	Cell *c;

	if((id = getid(s)) != -1)
		return id;
	id = dylen(core.labels);
	v.i = id;
	qlock(&symlock);
	c = setsymtab(s, nil, v, NUM, core.ids);
	qunlock(&symlock);
	dypush(core.labels, c->nval);
	return id;
}

void
setcoretag(int type, voff id, V val)
{
	assert(id >= 0 && id < dylen(core.labels));
	switch(type){
	case TCL: core.colors[id] = val.u; break;
	default: panic("setcoretag: unknown type %d", type);
	}
}

void
fixtabs(voff nnodes, int *lenp, ushort *degp)
{
	/* FIXME: resizable for mooltigraph? */
	assert(core.length == nil && core.degree == nil);
	dyresize(lenp, nnodes);
	dyresize(degp, nnodes);
	dyresize(core.labels, nnodes);
	dyresize(core.colors, nnodes);
	qlock(&symlock);
	core.length = attach("LN", core.ids, lenp, nnodes, NUM|P32, nil);
	core.degree = attach("degree", core.ids, degp, nnodes, NUM|P16, nil);
	core.label = attach("node", core.ids, core.labels, nnodes, STR, nil);
	core.color = attach("CL", core.ids, core.colors, nnodes, NUM|P32, nil);
	qunlock(&symlock);
}

void
initvars(void)
{
	Cell *c;

	core.ids = makesymtab(NSYMTAB);
	c = setsymtab("id", NULL, ZV, ARR, symtab);
	c->sval = (char *)core.ids;
}
