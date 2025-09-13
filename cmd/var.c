#include "strpg.h"
#include "threads.h"
#include "graph.h"
#include "cmd.h"
#include "drw.h"
#include <locale.h>
#include <signal.h>
#include "strawk/awk.h"
#include "var.h"

extern QLock symlock;
extern QLock buflock;	/* FIXME */

typedef struct Val Val;
typedef struct Core Core;

/* GFA does not support 64-bit wide types */
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
	Array *ptrs;
	Array *eptrs;
	Array *ids;
	Array *label;
	Array *length;
	Array *degree;
	Array *color;
};
static Core core;

char *
getname(voff idx)
{
	assert(idx >= 0 && idx < dylen(core.labels));
	return core.labels[idx];
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
		return getival(c);
}

uint
getnodelength(voff idx)
{
	assert(idx >= 0 && idx < dylen(core.labels));
	return *((uint *)core.length->tab + idx);
}

static inline Array *
mktab(Cell *cp, char type)
{
	voff n;
	int m;
	short atype;
	void *buf;
	Array *ap, *ids, *ptrs;

	if(isptr(cp))
		return (Array *)cp->sval;
	ids = nil;
	m = 0;
	atype = 0;
	switch(type){
	case Tint: m = sizeof(s32int); atype = NUM; break;
	case Tuint: m = sizeof(u32int); atype = NUM | USG; break;
	case Tfloat: m = sizeof(float); atype = NUM | FLT; break;
	case Tstring: m = sizeof(char*); atype = STR; break;
	default: panic("mktab: unknown type %o\n", type);
	}
	if(cp->tval & UNS){	/* FIXME: hack */
		n = dylen(edges);
		ptrs = core.eptrs;
	}else{
		n = dylen(core.labels);
		ids = core.ids;
		ptrs = core.ptrs;
	}
	buf = emalloc(n * m);
	if(type != Tstring)
		memset(buf, 0xfe, n * m);
	qlock(&symlock);
	ap = attach(cp->nval, ids, buf, n, atype, nil);
	setsymtab(cp->nval, cp->nval, ZV, STR|CON, ptrs);
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
/* FIXME: compare against user-provided tag type (esp for floats vs. int) */
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
		}
	}
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
			val = getsval(cp);	/* FIXME: sval could change */
			if((cp->tval & DONTFREE) == 0)	/* FIXME: when freeable? */
				val = estrdup(val);
		}
	}else{
		val = estrdup(val);
		qlock(&symlock);
		setsymtab(val, NULL, ZV, STR|CON, core.strs);
		qunlock(&symlock);
	}
	v->s = val;
	return Tstring;
}

void
setedgetag(char *tag, voff id, char *val)
{
	char type, etag[32];
	Cell *cp;
	TVal v;

	DPRINT(Debugawk, "setedgetag %s[%d] = %s", tag, id, val);
	qlock(&symlock);
	cp = setsymtab(tag, NULL, ZV, NUM|UNS, symtab);	/* FIXME: hack */
	qunlock(&symlock);
	if(isptr(cp) && ((Array *)cp->sval)->ids != nil
	|| !isptr(cp) && (cp->tval & UNS) == 0){
		DPRINT(Debuginfo, "%s is a node tag, renaming\n", tag);
		snprint(etag, sizeof etag, "e%s", tag);
		qlock(&symlock);
		cp = setsymtab(etag, NULL, ZV, NUM|UNS, symtab);
		qunlock(&symlock);
	}
	type = vartype(val, &v, 0);
	pushval(cp, id, type, v);
}

void
settag(char *tag, voff id, char *val)
{
	char type;
	Cell *cp;
	TVal v;

	DPRINT(Debugawk, "settag %s[%s] = %s", tag, getname(id), val);
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
	memset(core.colors, 0xfe, nnodes * sizeof *core.colors);
	qlock(&symlock);
	/* FIXME: make it RO after loading? */
	core.length = attach("LN", core.ids, lenp, nnodes, NUM|USG, setnodelength);
	core.degree = attach("degree", core.ids, degp, nnodes, RO|NUM|P16|USG, nil);
	core.label = attach("node", core.ids, core.labels, nnodes, RO|STR, nil);
	core.color = attach("CL", core.ids, core.colors, nnodes, NUM|USG, setnodecolor);
	setsymtab("LN", "LN", ZV, STR|CON, core.ptrs);
	setsymtab("CL", "CL", ZV, STR|CON, core.ptrs);
	setsymtab("degree", "degree", ZV, STR|CON, core.ptrs);
	qunlock(&symlock);
}

void
initvars(void)
{
	Cell *c;

	c = setsymtab("id", NULL, ZV, CON|ARR, symtab);
	if(c->sval != EMPTY)
		freesymtab(c);
	core.ids = makesymtab(NSYMTAB);
	c->sval = (char *)core.ids;
	c = setsymtab("STR", NULL, ZV, CON|ARR, symtab);
	if(c->sval != EMPTY)
		freesymtab(c);
	core.strs = makesymtab(NSYMTAB);
	c->sval = (char *)core.strs;
	c = setsymtab("ATTACHED", NULL, ZV, CON|ARR, symtab);
	if(c->sval != EMPTY)
		freesymtab(c);
	core.ptrs = makesymtab(NSYMTAB);
	c->sval = (char *)core.ptrs;
	c = setsymtab("EATTACHED", NULL, ZV, CON|ARR, symtab);
	if(c->sval != EMPTY)
		freesymtab(c);
	core.eptrs = makesymtab(NSYMTAB);
	c->sval = (char *)core.eptrs;
}
