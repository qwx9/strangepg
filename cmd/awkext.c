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
 * - Tab → extant tables → extra info to nodeinfo;
 * - multigraph: query graph node ranges or whatever */
/* FIXME: add hooks for the access of certain tables instead of regex?
 * when we are batch loading, we can cheat with the syntax to avoid
 * having to match at all */

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

static int
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
	assert(a != nil);
	return a;
}

static inline Array *
gettabarray(int t)
{
	Array *a;

	rlock(&tablock);
	a = tabs[t].a;
	runlock(&tablock);
	return a;
}

static inline Array *
mkarray(int t)
{
	Array *a;
	Tab *tab;

	if((a = gettabarray(t)) == nil){
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

	if((a = gettabarray(Tnode)) == nil)
		sysfatal("getnodeid %s: uninitialized table", lab);
	if((c = getcell(lab, a)) == nil)
		sysfatal("getnodeid %s: no such node", lab);
	if((id = getival(c)) < 0 || id >= dylen(nodes))
		sysfatal("getnodeid %s: invalid node id %d", lab, id);
	return id;
}

static inline char *
getnodelabel(ioff id)
{
	char lab[24];
	Cell *c;
	Array *a;

	if((a = gettabarray(Tlabel)) == nil)
		sysfatal("getnodelabel %d: uninitialized label table", id);
	snprint(lab, sizeof lab, "%d", id);
	if((c = getcell(lab, a)) == nil)
		sysfatal("getnodelabel %d: no such id", id);
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

	new = 0;
	a = mkarray(i);
	/* FIXME: better type checking; merge with generic tag code */
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
		if(type == Tfloat)	/* FIXME */
			val.u = val.f;
		setattr(i, id, val);
		c = setint(lab, val.u, a, &new);
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
		if(type == Tfloat)	/* FIXME */
			val.u = val.f;
		setattr(i, id, val);
		c = setint(lab, val.u, a, &new);
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
		if(type != Tfloat)	/* FIXME */
			val.f = val.i;
		setattr(i, id, val);
		c = setfloat(lab, val.f, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	default:
		setattr(i, id, val);
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
	if(type == Tstring && symtab != nil && (cp = getcell(val, symtab)) != nil){
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
		for(e=edges+n->eoff,ee=e+n->nedges; e<ee; e++){
			v = *e;
			aid = v >> 2;
			if(id > aid || id == aid && (v & 1) == 1)
				continue;
			p = seprint(s, s + sizeof s, "%d%c1", eid, '\034');
			vv.s = s;
			aid = id << 1 | (v & 1);
			set(Tedge, 0, aid, vv);
			p[-1] = '2';
			aid = v >> 1;
			set(Tedge, 0, aid, vv);
			eid++;
		}
	}
}

static void
fnnodecolor(char *lab, Awknum col)
{
	ioff id;
	Array *a;

	if((a = gettabarray(TCL)) == nil)
		 sysfatal("awk/nodecolor: uninitialized table");
	setint(lab, col, a, nil);
	id = getnodeid(lab);
	setcolor(rnodes[id].col, col);
}

/* FIXME: stricter error checking and recovery */
Cell *
addon(TNode **a, int)
{
	int t;
	char *s;
	Cell *x, *y, *ret;
	TNode *nextarg;
	RNode *r;

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
		nextarg = nextarg->nnext;
		fnnodecolor(s, getival(y));
		tempfree(y);
		break;
	case AINFO:
		strecpy(hoverstr, hoverstr+sizeof hoverstr, getsval(x));
		reqdraw(Reqshallowdraw);
		break;
	case AUNSHOW:
		r = rnodes + getival(x);
		if(r < rnodes || r >= rnodes + dylen(rnodes))
			FATAL("unknown nodeid or out of bounds access: %lld", x->val.i);
		y = execute(nextarg);
		nextarg = nextarg->nnext;
		setcolor(r->col, getival(y));
		tempfree(y);
		if((y = getcell("selinfo", symtab)) == nil)
			break;
		strecpy(selstr, selstr+sizeof selstr, getsval(y));
		reqdraw(Reqshallowdraw);
		break;
	case AREFRESH:
		if((y = getcell("selinfo", symtab)) == nil)
			break;
		strecpy(selstr, selstr+sizeof selstr, getsval(y));
		reqdraw(Reqshallowdraw);
		break;
	default:	/* can't happen */
		FATAL("illegal function type %d", t);
		break;
	}
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
		[Tselect] = {"selected", nil, 1},
	}, *pp;

	map = tab_init();
	for(pp=sptags; pp<sptags+nelem(sptags); pp++){
		i = mktab(pp->name, pp->nodeidx);
		tabs[i].fn = pp->fn;
	}
}
