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
	char **label;
	int *degree;
	int *length;
	u32int *color;
};
static Core core;
static Array *idtab;

char *
getname(voff id)
{
	assert(id >= 0 && id < dylen(core.label));
	return core.label[id];
}

voff
getid(char *s)
{
	Cell *c;

	qlock(&symlock);
	c = lookup(s, idtab);
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
	id = dylen(core.label);
	v.i = id;
	qlock(&symlock);
	c = setsymtab(s, nil, v, NUM, idtab);
	qunlock(&symlock);
	dypush(core.label, c->nval);
	return id;
}

void
setcoretag(int type, voff id, V v)
{
	switch(type){
	//case TLN: dygrow(core.length, id); core.length[id] = v.i; break;
	//case Tdegree: dygrow(core.degree, id); core.degree[id] = v.i; break;
	default: setspectag(type, id, v);	/* FIXME */
	//default: panic("setcoretag: unknown core tag type %d", type);
	}
}

void
initvars(void)
{
	Cell *c;

	idtab = makesymtab(NSYMTAB);
	c = setsymtab("id", NULL, ZV, ARR, symtab);
	c->sval = (char *)idtab;
}
