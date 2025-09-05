#include "strpg.h"
#include "threads.h"
#include "graph.h"
#include "cmd.h"
#include "drw.h"
#include <locale.h>
#include <signal.h>
#include "strawk/awk.h"

/* FIXME: broken: expand, getrealedge, csv loading */
/* FIXME: type errors here will halt everything */

extern QLock symlock;
extern QLock buflock;	/* FIXME */

typedef struct Val Val;
typedef struct Core Core;

/* FIXME: switch strawk to use 32-bit values as well? */
/* GFAv1 does not support 64-bit wide types */
enum{
	Tint,
	Tuint,
	Tfloat,
	Tstring,
};
typedef union{
	s32int i;
	u32int u;
	float f;
	char *s;
} TVal;

struct Val{
	Cell *cp;
	ioff id;
	char type;
	TVal v;
};
static Val *valbuf;

struct Core{
	char **labels;
	u32int *colors;
	Array *strs;
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

static void
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

/* FIXME: fx, and other optional tabs: strcmp and set upfn */
static inline Array *
mktab(Cell *cp, char type)
{
	voff n;
	int m;
	short atype;
	void *buf;
	Array *ap;

	if(isptr(cp))
		return (Array *)cp->sval;
	m = 0;
	atype = 0;
	switch(type){
	case Tint: m = sizeof(s32int); atype = NUM; break;
	case Tuint: m = sizeof(u32int); atype = NUM | USG; break;
	case Tfloat: m = sizeof(float); atype = NUM | FLT; break;
	case Tstring: m = sizeof(char*); atype = STR; break;
	default: panic("mktab: unknown type %o\n", type);
	}
	n = dylen(core.labels);
	buf = emalloc(n * m);
	if(type != Tstring)
		memset(buf, 0xfe, n * m);
	qlock(&symlock);
	ap = attach(cp->nval, core.ids, buf, n, atype, nil);
	qunlock(&symlock);
	return ap;
}

static inline void
set(Cell *cp, voff id, char type, TVal v)
{
	Array *ap;

	ap = mktab(cp, type);
	cp = setptrtab(id, ap, 0);
	switch(type){
	case Tint: setival(cp, v.i); break;
	case Tuint: setival(cp, v.u); break;
	case Tfloat: setfval(cp, v.f); break;
	case Tstring: setsval(cp, v.s, 0); break;
	default: panic("set: unknown type %o\n", type);
	}
	tempfree(cp);
}

void
loadbatch(void)
{
	Val *vp, *vs, *ve;

	if(valbuf == nil)	/* because clang is stupid */
		return;
	qlock(&buflock);
	vs = valbuf;
	valbuf = nil;
	qunlock(&buflock);
	for(vp=vs, ve=vp+dylen(vp); vp<ve; vp++)
		set(vp->cp, vp->id, vp->type, vp->v);
	dyfree(vs);
	USED(vs);
}

void
loadvars(void)
{
	static char already;
	ioff id;
	Node *n, *ne;
	RNode *r;
	Cell *cp;
	TVal v;

	if(already)
		return;
	already++;
	loadbatch();
	qlock(&symlock);
	cp = lookup("CL", symtab);
	qunlock(&symlock);
	assert(cp != nil);
	/* FIXME: this is a kludge, just assign a default color and overwrite,
	 * then we can remove loadall altogether */
	for(id=0, r=rnodes, n=nodes, ne=n+dylen(n); n<ne; n++, r++, id++){
		if(r->col[3] == 0.0f){
			v.u = somecolor(id, nil);
			set(cp, id, Tuint, v);
		}
		/* FIXME: edges */
	}
}

static inline void
pushval(Cell *cp, ioff id, char type, TVal v)
{
	ssize n;
	Val vp;

	vp = (Val){cp, id, type, v};
	qlock(&buflock);
	dypush(valbuf, vp);
	n = dylen(valbuf);
	qunlock(&buflock);
	if(n >= 64*1024){
		pushcmd("loadbatch()");
		flushcmd();
	}
}

/* FIXME: offload to loadbatch? */
/* FIXME: compare against user-provided tag type */
static inline char
vartype(char *val, TVal *v, int iscolor)
{
	char type, c, *s, *p;
	int i;
	float f;
	Cell *cp;

	s = val;
	if(s[0] == '#'){
		s++;
		i = strtoul(s, &p, 16);
		if(iscolor && p - s < 8)	/* sigh */
			i = i << 8;
		type = Tuint;
	}else{
		i = strtol(s, &p, 0);
		type = Tint;
	}
	if(p != s){
		if((c = *p) == '\0' || isspace(c)){
			v->i = i;
			return type;
		}else if(c == 'e' || c == 'E' || c == '.'){
			f = strtof(s, &p);
			if(p != s && ((c = *p) == '\0' || isspace(c))){
				v->f = f;
				return Tfloat;
			}
		}else
			type = Tstring;
	}else
		type = Tstring;
	if(type == Tstring){
		qlock(&symlock);
		cp = lookup(val, symtab);
		qunlock(&symlock);
		if(cp != nil){
			if(cp->tval & FLT){
				v->f = getfval(cp);
				return Tfloat;
			}else if(cp->tval & NUM){
				v->i = getival(cp);
				return Tint;
			}else{
				val = getsval(cp);
				if((cp->tval & DONTFREE) == 0)
					val = estrdup(val);
			}
		}else{
			qlock(&symlock);
			cp = setsymtab(val, NULL, ZV, STR|CON, core.strs);
			qunlock(&symlock);
			val = cp->nval;
		}
	}
	v->s = val;
	return Tstring;
}

void
settag(char *tag, voff id, char *val)
{
	char type;
	Cell *cp;
	TVal v;

	DPRINT(Debugawk, "settag %s[%d] = %s", tag, id, val);
	assert(id >= 0 && id < dylen(core.labels));
	qlock(&symlock);
	cp = setsymtab(tag, NULL, ZV, NUM, symtab);
	qunlock(&symlock);
	type = vartype(val, &v, (Array *)cp->sval == core.color);	/* FIXME */
	pushval(cp, id, type, v);
}

void
setnamedtag(char *tag, char *name, char *val)
{
	settag(tag, getid(name), val);
}

/* FIXME */
int
gettab(char *)
{
	return 0;
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
fixtabs(voff nnodes, int *lenp, ushort *degp)
{
	/* FIXME: resizable for mooltigraph? */
	assert(core.length == nil && core.degree == nil);
	dyresize(lenp, nnodes);
	dyresize(degp, nnodes);
	dyresize(core.labels, nnodes);
	dyresize(core.colors, nnodes);
	qlock(&symlock);
	core.length = attach("LN", core.ids, lenp, nnodes, NUM|USG, nil);
	core.degree = attach("degree", core.ids, degp, nnodes, NUM|P16|USG, nil);
	core.label = attach("node", core.ids, core.labels, nnodes, STR, nil);
	core.color = attach("CL", core.ids, core.colors, nnodes, NUM|USG, setnodecolor);
	qunlock(&symlock);
}

void
initvars(void)
{
	Cell *c;

	core.ids = makesymtab(NSYMTAB);
	c = setsymtab("id", NULL, ZV, ARR, symtab);
	c->sval = (char *)core.ids;
	core.strs = makesymtab(NSYMTAB);
	c = setsymtab("STR", NULL, ZV, ARR, symtab);
	c->sval = (char *)core.strs;
}
