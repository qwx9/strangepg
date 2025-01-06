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

/* FIXME: quoting problem in setnamedtag (for csv): not sure how to handle
 * that, maybe just do some parsing ourselves? */
/* FIXME: additional functions:
 * - get extant tables, or tables element is in + extra info to nodeinfo;
 *	 create array from info in Tab then delete it explicitely in main.awk
 * - nodecolor: set color in rnodes, etc., getting rid of all the workarounds
 *   in cmd (need global nodes first)
 * - multigraph: query graph node ranges or whatever */
/* FIXME: add hooks for the access of certain tables instead of regex? */
/* FIXME: rnodes is global; make nodes[] global as well; use range of node ids
 * to differenciate between graphs; multigraph support: just rename
 *conflicting nodes, add a _n suffix or sth */
/* FIXME: string values that actually refer to variables is unhandled when
 * loading gfa (csv uses setnamedtag): minor issue but resolved with global
 * nodes[] and using setattr in settag, etc. */
/* FIXME: go code should be issued once all files are done loading */

// FIXME: check with valgrind

KHASHL_MAP_INIT(KH_LOCAL, tabmap, tab, char*, int, kh_hash_str, kh_eq_str)

typedef struct Val Val;
typedef struct Tab Tab;

struct Val{
	int tab;
	int type;
	Value id;
	Value val;
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
static RWLock buflock;

int
gettab(char *s)
{
	khint_t k;

	k = tab_get(map, s);
	if(k == kh_end(map))
		return -1;
	return kh_val(map, k);
}

static inline int
mktab(char *tab, int intidx)
{
	int i, abs;
	char *s;
	Tab t;
	khint_t k;

	wlock(&buflock);
	k = tab_put(map, tab, &abs);
	if(abs){
		s = estrdup(tab);
		t = (Tab){intidx, s, nil, nil};
		i = dylen(tabs);
		dypush(tabs, t);
		kh_key(map, k) = s;
		kh_val(map, k) = i;
	}else
		i = kh_val(map, k);
	wunlock(&buflock);
	return i;
}

static inline char *
getnodelabel(ioff i)
{
	static char lab[16];
	Cell *c;
	Array *a;

	snprint(lab, sizeof lab, "%d", i);
	if((c = lookup("label", symtab)) == nil){
		werrstr("no extant labels");
		return nil;
	}else if(!isarr(c)){
		werrstr("bug: scalar label");
		return nil;
	}
	a = (Array *)c->sval;
	if((c = lookup(lab, a)) == nil){
		werrstr("doesn\'t exist");
		return nil;
	}
	return c->sval;
}

static inline Array *
getarray(char *arr, int *new)
{
	Cell *c;
	Array *a;
	Value v = {.i = 0};

	if((c = lookup(arr, symtab)) == nil){
		c = setsymtab(arr, NULL, v, ARR, symtab);
		a = makesymtab(NSYMTAB);
		c->sval = (char *)a;
		c->tval |= ARR;
		if(new != nil)
			*new = 1;
	}else if(!isarr(c)){
		if (freeable(c))
			xfree(c->sval);
		a = makesymtab(NSYMTAB);
		c->tval &= ~(STR|NUM|DONTFREE);
		c->tval |= ARR;
		c->sval = (char *)a;
		if(new != nil)
			*new = 1;
	}else
		a = (Array *)c->sval;
	return a;
}

static inline Cell *
getcell(char *lab, Array *a)
{
	assert(a != nil);
	return lookup(lab, a);
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
set(int i, int type, Value id, Value val)
{
	int new;
	char l[16], *lab;
	Cell *c;

	new = 0;
	if(tabs[i].a == nil){
		assert(tabs[i].name != nil);
		tabs[i].a = getarray(tabs[i].name, nil);
	}
	switch(i){
	case Tnode:
		c = setint(val.s, id.i, tabs[i].a, &new);
		if(new){	/* kludge to reuse buffers */
			free(c->nval);
			c->nval = val.s;
		}
		i = Tlabel;
		if(tabs[i].a == nil){
			assert(tabs[i].name != nil);
			tabs[i].a = getarray(tabs[i].name, nil);
		}
		/* wet floor */
	case Tlabel:
		snprint(l, sizeof l, "%lld", id.i);
		setstr(l, val.s, tabs[i].a, nil);	/* alloced key to transient node hashmap */
		break;
	case Tedge:
		setint(val.s, id.i, tabs[i].a, nil);
		break;
	case TCL:
	case TLN:
		if((lab = getnodelabel(id.i)) == nil)
			sysfatal("[awk] set %s[%lld]: %s", tabs[i].name, id.i, error());
		c = setint(lab, val.i, tabs[i].a, &new);
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
		if((lab = getnodelabel(id.i)) == nil)
			sysfatal("[awk] set %s[%lld]: %s", tabs[i].name, id.i, error());
		c = setfloat(lab, val.f, tabs[i].a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	default:
		if(tabs[i].intidx){
			snprint(l, sizeof l, "%lld", id.i);
			lab = l;
		}else if((lab = getnodelabel(id.i)) == nil)
			sysfatal("[awk] set %s[%lld] type %d: %s", tabs[i].name, id.i, type, error());
		switch(type){
		case STR: setstr(lab, val.s, tabs[i].a, nil); break;
		case NUM: setint(lab, val.i, tabs[i].a, nil); break;
		case FLT: setfloat(lab, val.f, tabs[i].a, nil); break;
		default: FATAL("loadbatch: unknown type %d", type);
		}
	}
}

static void
loadbatch(void)
{
	Val *v, *vs, *ve;

	if(valbuf == nil)
		return;
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
pushval(int tab, int type, Value id, Value val)
{
	Val v;

	v = (Val){tab, type, id, val};
	wlock(&buflock);
	dypush(valbuf, v);
	wunlock(&buflock);
}

/* FIXME: kind of shitty api with intidx */
/* DOES strdup strings */
void
settag(char *tag, ioff id, char fmt, char *val, int intidx)
{
	int i, type;
	Value vid, vval;

	i = mktab(tag, intidx);
	switch(fmt){
	case 'i': type = NUM; vval.i = atoi(val); break;
	case 'f': type = FLT; vval.f = atof(val); break;
	default:
		warn("settag %s=%s: unknown type %c, defaulting to string\n",
			tag, val, fmt);
		/* wet floor */
	case 'A':
	case 'Z':
	case 'J':
	case 'H':
	case 'B': type = STR; vval.s = estrdup(val); break;
	}
	vid.i = id;
	dprint(Debugfs, "settag %s[%d] = %s (%c)", tag, id, val, fmt);
	pushval(i, type, vid, vval);
}

/* does NOT strdup, caller's responsibility */
void
setspectag(int i, ioff id, char *val)
{
	Value vid, vval;

	vid.i = id;	/* overload, we already know table's type */
	vval.s = val;

	dprint(Debugfs, "setspectag %s[%d] = %s", tabs[i].name, id, val);
	pushval(i, 0, vid, vval);
}

/* don't know what the id is or if the value is a constant or another var */
void
setnamedtag(char *tag, char *name, char *val)
{
	int i;

	i = mktab(tag, 1);	/* assuming index by node */
	/* FIXME: this doesn't solve the quoting problem: could be a string,
	 * or a number, or a variable */
	if(tabs[i].fn != nil)
		pushcmd("%s(\"%s\",%s)", tabs[i].fn, name, val);
	else
		pushcmd("%s[\"%s\"]=\"%s\"", tag, name, val);
}

static void
load(void)
{
	static char already;
	ioff id, eid, *e, *ee, v;
	char s[16], *p;
	Node *n, *ne;
	Graph *g, *ge;
	Value vid, vv;

	if(already++)
		return;
	loadbatch();
	for(g=graphs, ge=g+dylen(g); g<ge; g++){
		for(id=eid=0, n=g->nodes, ne=n+dylen(n); n<ne; n++, id++){
			vid.i = id;
			vv.u = n->attr.color;
			set(TCL, 0, vid, vv);
			vv.u = n->attr.length;
			set(TLN, 0, vid, vv);
			if((n->attr.flags & FNinitx) != 0){
				vv.f = n->attr.pos0.x;
				set(Tx0, 0, vid, vv);
				if((n->attr.flags & FNfixedx) != 0)
					set(Tfx, 0, vid, vv);
			}
			if((n->attr.flags & FNinity) != 0){
				vv.f = n->attr.pos0.y;
				set(Ty0, 0, vid, vv);
				if((n->attr.flags & FNfixedy) != 0)
					set(Tfy, 0, vid, vv);
			}
			if((n->attr.flags & FNinitz) != 0){
				vv.f = n->attr.pos0.z;
				set(Tz0, 0, vid, vv);
				if((n->attr.flags & FNfixedz) != 0)
					set(Tfz, 0, vid, vv);
			}
			for(e=g->edges+n->eoff,ee=e+n->nedges; e<ee; e++, eid++){
				p = seprint(s, s + sizeof s, "%d\x1c\x31", id);
				v = *e;
				vv.s = s;
				vid.i = id << 1 & (v & 1);
				set(Tedge, 0, vid, vv);
				p[-1] = '2';
				vid.i = v >> 1;
				set(Tedge, 0, vid, vv);
			}
		}
	}
}

Cell *
addon(TNode **a, int)
{
	int t;
	Cell *x, *y, *ret;
	TNode *nextarg;

	t = ptoi(a[0]);
	x = execute(a[1]);
	nextarg = a[1]->nnext;
	ret = gettemp();
	setival(ret, 0);
	switch(t){
	case ALOAD:
		load();
		break;
	case ALOADBATCH:
		loadbatch();
		break;
	default:	/* can't happen */
		FATAL("illegal function type %d", t);
		break;
	}
	tempfree(x);
	if(nextarg != NULL){
		WARNING("warning: function has too many arguments");
		for(; nextarg; nextarg = nextarg->nnext){
			y = execute(nextarg);
			tempfree(y);
		}
	}
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
	}, *pp;

	map = tab_init();
	for(pp=sptags; pp<sptags+nelem(sptags); pp++){
		i = mktab(pp->name, pp->nodeidx);
		tabs[i].fn = pp->fn;
	}
}
