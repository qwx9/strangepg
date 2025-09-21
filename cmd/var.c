#include "strpg.h"
#include "threads.h"
#include "graph.h"
#include "cmd.h"
#include "drw.h"
#include "strawk/awk.h"
#include "var.h"

extern QLock buflock;	/* FIXME */

Core core;

typedef struct Val Val;

/* GFA does not support 64-bit wide types */
enum{
	Tint,
	Tuint,
	Tfloat,
	Tstring,
	Tnil,
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

	c = lookup(s, core.ids);
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

static Array *
autoattach(Cell *cp)
{
	short type;
	char *tag;
	usize n, m;
	void *buf;
	Array *ap;
	void (*fn)(size_t, Value);

	cp = lookup(cp->nval, symtab);
	assert(cp != nil);
	fn = nil;
	tag = cp->nval;
	type = cp->tval;
	n = dylen(core.labels);
	m = 0;
	switch(type & (NUM|STR|FLT)){
	case NUM:
	case STR|NUM: m = sizeof(s32int); type &= ~(STR|FLT); break;
	case NUM|FLT:
	case STR|NUM|FLT: m = sizeof(float); type &= ~STR; break;
	case STR: m = sizeof(char *); type &= ~(NUM|FLT); break;
	default: panic("autoattach %s: invalid type %o", tag, type & (NUM|STR|FLT));
	}
	cp->tval = type & ~UND;
	buf = emalloc(n * m);
	if((type & STR) == 0)
		memset(buf, 0xfe, n * m);
	if(strlen(tag) == 2){
		if(tag[0] == 'f')
			switch(tag[1]){
			case 'x': fn = setnodefixedx; break;
			case 'y': fn = setnodefixedy; break;
			case 'z': fn = setnodefixedz; break;
			}
		else if(tag[1] == '0')
			switch(tag[0]){
			case 'x': fn = setnodeinitx; break;
			case 'y': fn = setnodeinity; break;
			case 'z': fn = setnodeinitz; break;
			}
	}
	ap = attach(tag, core.ids, buf, n, type, fn);
	setsymtab(tag, tag, ZV, STR|CON, core.ptrs);
	return ap;
}

static inline Array *
mktab(Cell *cp, char type)
{
	voff n;
	int m;
	short atype;
	char *tag;
	void *buf;
	void (*fn)(size_t, Value);
	Array *ap, *ids, *ptrs;

	ap = (Array *)cp->sval;
	if(isptr(cp) && ap != (Array *)EMPTY)
		return ap;
	ids = nil;
	m = 0;
	atype = 0;
	fn = nil;
	tag = cp->nval;
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
		if(strlen(tag) == 2){
			if(tag[0] == 'f')
				switch(tag[1]){
				case 'x': fn = setnodefixedx; break;
				case 'y': fn = setnodefixedy; break;
				case 'z': fn = setnodefixedz; break;
				}
			else if(tag[1] == '0')
				switch(tag[0]){
				case 'x': fn = setnodeinitx; break;
				case 'y': fn = setnodeinity; break;
				case 'z': fn = setnodeinitz; break;
				}
		}
	}
	buf = emalloc(n * m);
	if(type != Tstring)
		memset(buf, 0xfe, n * m);
	ap = attach(tag, ids, buf, n, atype, fn);
	setsymtab(tag, tag, ZV, STR|CON, ptrs);
	return ap;
}

static inline void
set(Cell *tp, voff id, char type, TVal v)
{
	Cell *cp;
	Array *ap;

	ap = mktab(tp, type);
	cp = setptrtab(id, ap, 0);
	switch(type){
	case Tint: setival(cp, v.i); break;
	case Tuint: setival(cp, v.u); break;
	case Tfloat: setfval(cp, v.f); break;
	case Tstring: setsval(cp, v.s, 0); break;
	default: panic("set: unknown type %o\n", type);
	}
	tempfree(cp);	/* FIXME: race with run.c */
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

/* can't be strict here because these types are not respected much */
static inline int
tagtype(char *val, char ttype, char *p)
{
	int r;
	char type;

	r = 0;
	switch(ttype){
	case '\0': type = val[0] == '#' ? Tuint : Tnil; break;
	case 'J':
	case 'H':
	case 'B':
		werrstr("unhandled tag type %c", ttype);
		r = -1;
		type = Tstring;
		break;
	case 'A': type = Tstring; break;
	case 'Z': type = val[0] == '#' ? Tuint : Tstring; break;
	case 'i': type = val[0] == '#' ? Tuint : Tint; break;
	case 'f': type = val[0] == '#' ? Tuint : Tfloat; break;
	default:
		werrstr("unknown tag type %c", ttype);
		r = -1;
		type = Tnil;
	}
	*p = type;
	return r;
}

/* we have to be careful with assigning numeric types since
 * ptrs are not dynamic yet we can't trust the tag types in
 * the input data */
static inline char
vartype(char *val, char type, TVal *vp, Cell *cp)
{
	char c, *s, *p;
	TVal v;

	s = val;
	if(type != Tstring || cp->tval & NUM){
		if(type == Tuint){
			if(val[0] == '#')
				s++;
			v.u = strtoul(s, &p, 16);
			if(p - s < 8 && (Array *)cp->sval == core.color)
				v.u = v.u << 8;
		}else{
			v.i = strtol(s, &p, 0);
			if(type == Tnil)
				type = Tint;
		}
		if(p != s){
			if((c = *p) == '\0' || isspace(c)){
				switch(type){
				case Tfloat: vp->f = v.i; return type;
				case Tuint: vp->u = v.u; return type;
				case Tint: vp->i = v.i; return type;
				case Tstring:
					for(p++; (c=*p)!=0; p++)
						if(!isspace(c))
							break;
					if(c == 0){
						vp->i = v.i;
						return Tint;
					}
					break;
				default: panic("numeric vartype %d: can\'t happen", type);
				}
			}else if(c == 'e' || c == 'E' || c == '.'){
				v.f = strtof(s, &p);
				if(p != s && ((c = *p) == '\0' || isspace(c))){
					*vp = v;
					return Tfloat;
				}
			}
		}
	}
	cp = lookup(val, symtab);
	if(cp != nil){
		if(cp->tval & FLT){
			vp->f = getfval(cp);
			return Tfloat;
		}else if(cp->tval & NUM){
			vp->i = getival(cp);
			return Tint;
		}else{
			val = getsval(cp);	/* FIXME: sval could change */
			if((cp->tval & DONTFREE) == 0)
				val = estrdup(val);
		}
	}else{
		cp = setsymtab(val, NULL, ZV, STR|CON, core.strs);
		if(cp->nval == val)
			cp->nval = val = estrdup(val);
		else
			val = cp->nval;
	}
	vp->s = val;
	return Tstring;
}

int
setedgetag(char *tag, voff id, char ttype, char *val)
{
	int r;
	char type, etag[32];
	Cell *cp;
	TVal v;

	r = 0;
	DPRINT(Debugawk, "setedgetag %s[%d] = %s", tag, id, val);
	cp = setsymtab(tag, NULL, ZV, 0|UNS, symtab);	/* FIXME: hack */
	if(isptr(cp) && cp->sval != EMPTY && ((Array *)cp->sval)->ids != nil
	|| !isptr(cp) && (cp->tval & UNS) == 0){
		snprint(etag, sizeof etag, "e%s", tag);
		cp = setsymtab(etag, NULL, ZV, NUM, symtab);
		/* only print the warning once */
		if(!isptr(cp) && (cp->tval & UNS) == 0){
			werrstr("%s is a node tag, renaming", tag);
			r--;
			cp->tval |= UNS;
		}
	}
	r += tagtype(val, ttype, &type);
	type = vartype(val, type, &v, cp);
	pushval(cp, id, type, v);
	return r;
}

int
settag(char *tag, voff id, char ttype, char *val)
{
	int r;
	char type;
	Cell *cp;
	TVal v;

	DPRINT(Debugawk, "settag %s[%s] = %s", tag, getname(id), val);
	assert(id >= 0 && id < dylen(core.labels));
	cp = setsymtab(tag, NULL, ZV, 0, symtab);
	r = tagtype(val, ttype, &type);
	type = vartype(val, type, &v, cp);
	pushval(cp, id, type, v);
	return r;
}

int
setnamedtag(char *tag, char *name, char *val)
{
	return settag(tag, getid(name), '\0', val);
}

voff
pushname(char *s)
{
	voff id;
	Value v;
	Cell *c;

	if(autoattachfn == nil){	/* let it cook */
		qlock(&buflock);
		qunlock(&buflock);
	}
	if((id = getid(s)) != -1)
		return id;
	id = dylen(core.labels);
	v.i = id;
	c = setsymtab(s, nil, v, NUM, core.ids);
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
	/* FIXME: make it RO after loading? */
	core.length = attach("LN", core.ids, lenp, nnodes, NUM|USG, setnodelength);
	core.degree = attach("degree", core.ids, degp, nnodes, RO|NUM|P16|USG, nil);
	core.label = attach("node", core.ids, core.labels, nnodes, RO|STR, nil);
	core.color = attach("CL", core.ids, core.colors, nnodes, NUM|USG, setnodecolor);
}

static inline Array *
mkarray(Cell *cp)
{
	Array *ap;

	if(!isarr(cp)){	/* else preallocated during parsing */
		if(freeable(cp))
			xfree(cp->sval);
		cp->tval |= ARR|DONTFREE;
		ap = makesymtab(NSYMTAB);
		cp->sval = (char *)ap;
	}else{
		ap = (Array *)cp->sval;
		if(ap->nelem != 0)
			freesymtab(cp, 0);
	}
	return ap;
}

static inline Array *
initset(char *name)
{
	Cell *cp;

	cp = setsymtab(name, NULL, ZV, STR, symtab);
	return mkarray(cp);
}

static inline void
inittag(char *tag, short type, Array **app)
{
	Cell *cp;

	cp = setsymtab(tag, EMPTY, ZV, CON|type, symtab);
	if(app == nil){
		if(!isarr(cp))
			cp->tval |= ARR|PTR;
		return;
	}
	*app = mkarray(cp);
}

void
initvars(void)	/* called within strawk */
{
	inittag("id", RO, &core.ids);
	inittag("STR", RO|STR, &core.strs);
	inittag("ATTACHED", RO|STR, &core.ptrs);
	inittag("EATTACHED", RO|STR, &core.eptrs);
	inittag("CL", NUM|USG, nil);
	inittag("LN", NUM|USG, nil);
	inittag("node", RO|STR, nil);
	inittag("degree", RO|NUM|P16|USG, nil);
	inittag("fx", NUM|FLT, nil);
	inittag("fy", NUM|FLT, nil);
	inittag("fz", NUM|FLT, nil);
	inittag("x0", NUM|FLT, nil);
	inittag("y0", NUM|FLT, nil);
	inittag("z0", NUM|FLT, nil);
	inittag("cigar", STR|UNS, nil);	/* FIXME: hack */
	setsymtab("LN", "LN", ZV, STR|CON, core.ptrs);
	setsymtab("CL", "CL", ZV, STR|CON, core.ptrs);
	setsymtab("degree", "degree", ZV, STR|CON, core.ptrs);
	core.sel = initset("selected");
	hookdel(core.sel, deselectnode);
	autoattachfn = autoattach;
	qunlock(&buflock);
}
