#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include <locale.h>
#include <signal.h>
#include "lib/khashl.h"
#include "strawk/awk.h"

/* FIXME: additional functions:
 * - get extant tables, or tables element is in + extra info to nodeinfo;
 *	 create array from info in Tab then delete it explicitely in main.awk
 * - multigraph: query graph node ranges or whatever */
/* FIXME: add hooks for the access of certain tables instead of regex? */

KHASHL_MAP_INIT(KH_LOCAL, tabmap, tab, char*, int, kh_hash_str, kh_eq_str)

typedef struct Val Val;
typedef struct Tab Tab;

enum{
	Tint,
	Tuint,
	Tfloat,
	Tstring,
};
struct Val{
	int tab;
	int type;
	ioff id;
	V val;
};
static Val *valbuf;

struct Tab{
	int intidx;
	char *name;
	char *fn;
	Array *a;
};
static tabmap *map;
static Tab *tabs;
static RWLock buflock, tablock;

int
gettab(char *s)
{
	khint_t k;

	k = tab_get(map, s);
	if(k == kh_end(map))
		return -1;
	return kh_val(map, k);
}

static inline Array *
gettabarray(int t)
{
	Array *a;

	rlock(&buflock);
	if((a = tabs[t].a) == nil)
		sysfatal("gettabarray: uninitialized table %d",t);
	runlock(&buflock);
	return a;
}

static inline int
mktab(char *tab, int intidx)
{
	int i, abs;
	char *s;
	Tab t;
	khint_t k;

	rlock(&tablock);
	k = tab_put(map, tab, &abs);
	if(!abs){
		i = kh_val(map, k);
		runlock(&tablock);
		return i;
	}
	runlock(&tablock);
	wlock(&tablock);
	s = estrdup(tab);
	t = (Tab){intidx, s, nil, nil};
	i = dylen(tabs);
	dypush(tabs, t);
	kh_key(map, k) = s;
	kh_val(map, k) = i;
	wunlock(&tablock);
	return i;
}

static inline Array *
getarray(char *arr)
{
	Cell *c;
	Array *a;
	Value v = {.i = 0};

	if((c = lookup(arr, symtab)) == nil){
		c = setsymtab(arr, nil, v, ARR, symtab);
		a = makesymtab(NSYMTAB);
		c->sval = (char *)a;
		c->tval |= ARR;
	}else if(!isarr(c)){
		if (freeable(c))
			xfree(c->sval);
		a = makesymtab(NSYMTAB);
		c->tval &= ~(STR|NUM|DONTFREE);
		c->tval |= ARR;
		c->sval = (char *)a;
	}else
		a = (Array *)c->sval;
	return a;
}

static inline Array *
mkarray(int t)
{
	Array *a;
	Tab *tab;

	rlock(&tablock);
	tab = tabs + t;
	a = tab->a;
	runlock(&tablock);
	if(a == nil){
		wlock(&tablock);
		tab = tabs + t;
		assert(tab->name != nil);
		a = tab->a = getarray(tab->name);
		wunlock(&tablock);
	}
	return a;
}

static inline Cell *
getcell(char *lab, Array *a)
{
	assert(a != nil);
	return lookup(lab, a);
}

static inline ioff
getnodeid(char *lab)
{
	ioff id;
	Cell *c;
	Array *a;

	a = gettabarray(Tnode);
	if((c = getcell(lab, a)) == nil)
		sysfatal("getnodeid: no such node %s", lab);
	if((id = getival(c)) < 0 || id >= dylen(nodes))
		sysfatal("getnodeid: invalid node %s id %d", lab, id);
	return id;
}

static inline char *
getnodelabel(ioff i)
{
	char lab[24];
	Cell *c;
	Array *a;

	a = gettabarray(Tlabel);
	snprint(lab, sizeof lab, "%d", i);
	if((c = getcell(lab, a)) == nil)
		sysfatal("getnodelabel: no such id %s (%d)", lab, i);
	return c->sval;
}

static inline Cell *
setstr(char *lab, char *s, Array *a, int *new)
{
	Cell *c;
	Value v;

	if((c = getcell(lab, a)) == nil){
		v.u = 0;
		c = setsymtab(lab, nil, v, STR, a);
		if(new != nil)
			*new = 1;
	}else if(freeable(c))
		xfree(c->sval);
	c->sval = s;
	return c;
}

static inline Cell *
setint(char *lab, s64int i, Array *a, int *new)
{
	Cell *c;
	Value v;

	if((c = getcell(lab, a)) == nil){
		v.i = i;
		c = setsymtab(lab, nil, v, NUM, a);
		if(new != nil)
			*new = 1;
	}else
		setival(c, i);
	return c;
}

static inline Cell *
setfloat(char *lab, double f, Array *a, int *new)
{
	Cell *c;
	Value v;

	if((c = getcell(lab, a)) == nil){
		v.f = f;
		c = setsymtab(lab, nil, v, NUM|FLT, a);
		if(new != nil)
			*new = 1;
	}else
		setfval(c, f);
	return c;
}

/* not using Tab pointers to avoid locking; assumes string values are already
 * strdup'ed where appropriate */
static void
set(int i, int type, ioff id, V val)
{
	int intidx, new;
	char l[64], *lab;
	Cell *c;
	Array *a;

	/* FIXME: more sanity checks, including i */
	new = 0;
	a = mkarray(i);
	/* FIXME: may error after setattr, checks should be in setattr itself */
	setattr(i, id, val);
	switch(i){
	case Tnode:
		c = setint(val.s, id, a, &new);
		if(new){	/* kludge to reuse buffers */
			free(c->nval);
			c->nval = val.s;
		}
		i = Tlabel;
		a = mkarray(i);
		/* wet floor */
	case Tlabel:
		snprint(l, sizeof l, "%d", id);
		setstr(l, val.s, a, nil);	/* alloced key to transient node hashmap */
		break;
	case Tedge:
		/* FIXME: prevent direct access to this and other protected tabs: if
		 * not new, prevent write access */
		setint(val.s, id, a, nil);
		break;
	case TCL:
		lab = getnodelabel(id);
		if(type != Tint && type != Tuint){
			logerr(va("set CL[%s]: invalid non-integer color", lab));
			break;
		}
		c = setint(lab, val.i, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	case TLN:
		if(type != Tuint && type != Tint){
			DPRINT(Debuginfo, "not assigning string value %s to CL[%d]", val.s, id);
			return;
		}
		lab = getnodelabel(id);
		c = setint(lab, val.i, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	case Tfx:
	case Tfy:
	case Tfz:
	case Tx0:
	case Ty0:
	case Tz0:
		lab = getnodelabel(id);
		c = setfloat(lab, val.f, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	default:
		rlock(&tablock);
		intidx = tabs[i].intidx;
		runlock(&tablock);
		if(intidx){
			snprint(l, sizeof l, "%d", id);
			lab = l;
		}else
			lab = getnodelabel(id);
		switch(type){
		case Tint: setint(lab, val.i, a, nil); break;
		case Tuint: setint(lab, val.u, a, nil); break;
		case Tfloat: setfloat(lab, val.f, a, nil); break;
		case Tstring: setstr(lab, val.s, a, nil); break;
		default: FATAL("loadbatch: unknown type %d", type);
		}
	}
}

static void
fnloadbatch(void)
{
	Val *v, *vs, *ve;

	wlock(&buflock);
	vs = valbuf;
	valbuf = nil;
	wunlock(&buflock);
	for(v=vs, ve=v+dylen(v); v<ve; v++)
		set(v->tab, v->type, v->id, v->val);
	dyfree(vs);
	USED(vs);
}

static inline void
pushval(int tab, int type, ioff id, V val)
{
	Val v;

	v = (Val){tab, type, id, val};
	wlock(&buflock);
	dypush(valbuf, v);
	wunlock(&buflock);
	if(dylen(valbuf) >= 64*1024){
		pushcmd("loadbatch()");
		flushcmd();
	}
}

static inline int
vartype(char *val, V *v, int iscolor)
{
	int type;
	char c, *s, *p;
	s64int i;
	double f;
	Cell *cp;

	type = Tstring;
	s = val;
	if(s[0] == '#'){
		s++;
		i = strtoull(s, &p, 16);
		if(iscolor && p - s < 8)	/* sigh */
			i = i << 8 | 0xc0;
	}else
		i = strtoll(s, &p, 0);
	if(p != s){
		if((c = *p) == '\0' || isspace(c)){
			v->i = i;
			return Tint;
		}else if(c == 'e' || c == 'E' || c == '.'){
			f = strtod(s, &p);
			if(p != s && ((c = *p) == '\0' || isspace(c))){
				v->f = f;
				return Tfloat;
			}
		}
	}
	if(type == Tstring && (cp = getcell(val, symtab)) != nil){
		if(cp->tval & FLT){
			v->f = getfval(cp);
			return Tfloat;
		}else if(cp->tval & NUM){
			v->i = getival(cp);
			return Tint;
		}else
			val = getsval(cp);
	}
	v->s = estrdup(val);	/* always freeable, even if sym */
	return Tstring;
}

/* FIXME: kind of shitty api with intidx */
/* called by other threads; DOES strdup strings */
void
settag(char *tag, ioff id, char *val, int intidx)
{
	int t, type;
	V v;

	t = mktab(tag, intidx);
	type = vartype(val, &v, t == TCL);
	DPRINT(Debugawk, "settag %s[%d] = %s (%d)", tag, id, val, type);
	pushval(t, type, id, v);
}

/* called by other threads; does NOT strdup, caller's responsibility */
void
setspectag(int i, ioff id, V val)
{
	DPRINT(Debugawk, "setspectag %s[%d] = %llx", tabs[i].name, id, val.i);
	pushval(i, 0, id, val);
}

/* only call after all nodes have been created: csv, etc.; strdups strings */
void
setnamedtag(char *tag, char *name, char *val)
{
	int i, type;
	ioff id;
	V v;

	i = mktab(tag, 1);	/* assuming index by node */
	id = getnodeid(name);
	type = vartype(val, &v, i == TCL);
	DPRINT(Debugawk, "setnamedtag %s[%s:%d] = %s (%d)", tag, name, id, val, type);
	pushval(i, type, id, v);
}

static void
fnloadall(void)
{
	static char already;
	ioff id, eid, aid, *e, *ee, v;
	char s[16], *p;
	Node *n, *ne;
	RNode *r;
	V vv;

	if(already++)
		return;
	fnloadbatch();
	for(id=eid=0, r=rnodes, n=nodes, ne=n+dylen(n); n<ne; n++, r++, id++){
		if(r->col[3] == 0.0f){
			vv.i = somecolor(id, nil);
			set(TCL, NUM, id, vv);
		}
		for(e=edges+n->eoff,ee=e+n->nedges; e<ee; e++, eid++){
			p = seprint(s, s + sizeof s, "%d\x1c\x31", id);
			v = *e;
			vv.s = s;
			aid = id << 1 & (v & 1);
			set(Tedge, 0, aid, vv);
			p[-1] = '2';
			aid = v >> 1;
			set(Tedge, 0, aid, vv);
		}
	}
}

static void
fndeselect(void)
{
	/*
	- get selected array
	- get CL array
	- loop through selected then lookup element in arrahy
	- reset color
	*/
}

static void
fnselect(ioff id)
{
	/*
	- highlight node
	- cache selected array
	- add to selected array
	*/
}

static void
fnnodecolor(char *lab, Awknum col)
{
	ioff id;
	Array *a;

	a = gettabarray(TCL);
	setint(lab, col, a, nil);
	id = getnodeid(lab);
	setcolor(rnodes[id].col, col);
}

Cell *
addon(TNode **a, int)
{
	int t;
	char *s;
	Cell *x, *y, *ret;
	TNode *nextarg;

	t = ptoi(a[0]);
	x = execute(a[1]);
	nextarg = a[1]->nnext;
	ret = gettemp();
	setival(ret, 0);
	switch(t){
	case ALOAD:
		fnloadall();
		break;
	case ALOADBATCH:
		fnloadbatch();
		break;
	case ANODECOLOR:
		s = getsval(x);
		y = execute(nextarg);
		fnnodecolor(s, getival(y));
		tempfree(y);
		nextarg = nextarg->nnext;
		break;
	case ASELECT:
		//fnselect(x->val.i);
		break;
	case ADESELECT:
		break;
		// FIXME:
		// ...
		break;
	default:	/* can't happen */
		FATAL("illegal function type %d", t);
		break;
	}
	tempfree(x);
	if(nextarg != nil){
		WARNING("warning: function has too many arguments");
		for(; nextarg; nextarg = nextarg->nnext){
			y = execute(nextarg);
			tempfree(y);
		}
	}
	tempfree(x);
	return ret;
}

void
initext(void)
{
	int i;
	struct {
		char *name;
		char *fn;
		int nodeidx;
	} sptags[] = {
		[TLN] = {"LN", nil, 1},
		[Tfx] = {"fx", "fixx", 1},
		[Tfy] = {"fy", "fixy", 1},
		[Tfz] = {"fz", "fixz", 1},
		[Tx0] = {"x0", "initx", 1},
		[Ty0] = {"y0", "inity", 1},
		[Tz0] = {"z0", "initz", 1},
		[Tnode] = {"node", nil, 1},
		[Tedge] = {"edge", nil, 0},
		[Tlabel] = {"label", nil, 0},
		[TCL] = {"CL", "nodecolor", 1},
		[Tselect] = {"selected", "select"},
	}, *pp;

	map = tab_init();
	for(pp=sptags; pp<sptags+nelem(sptags); pp++){
		i = mktab(pp->name, pp->nodeidx);
		tabs[i].fn = pp->fn;
	}
}
