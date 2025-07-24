#include "strpg.h"
#include "threads.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "graph.h"
#include "cmd.h"
#include <locale.h>
#include <signal.h>
#include "lib/khashl.h"
#include "strawk/awk.h"

QLock symlock;	/* shared with awk process to not outrun compilation */

/* NOTE: pushcmd here is asking for trouble. fs/gfa and others
 * may be saturating the cmd buffer and we're the only reader. */

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
static RWLock tablock;
static QLock buflock;

/* FIXME: can replace with awk array and lookup */
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

	k = tab_put(map, tab, &abs);
	if(!abs){
		i = kh_val(map, k);
		return i;
	}
	s = estrdup(tab);
	t = (Tab){intidx, s, nil, nil};
	wlock(&tablock);
	i = dylen(tabs);
	dypush(tabs, t);
	wunlock(&tablock);
	kh_key(map, k) = s;
	kh_val(map, k) = i;
	return i;
}

static inline Array *
getarray(char *arr)
{
	Cell *c;
	Array *a;
	Value v = {.i = 0};

	qlock(&symlock);
	c = lookup(arr, symtab);
	qunlock(&symlock);
	if(c == nil){
		qlock(&symlock);
		c = setsymtab(arr, nil, v, ARR, symtab);
		qunlock(&symlock);
		a = makesymtab(NSYMTAB);
		c->sval = (char *)a;
		c->tval |= ARR;
	}else if(!isarr(c)){
		if (freeable(c))
			xfree(c->sval);
		a = makesymtab(NSYMTAB);
		c->tval &= ~(STR|NUM|FLT|DONTFREE);
		c->tval |= ARR;
		c->sval = (char *)a;
	}else{
		qlock(&symlock);
		a = (Array *)c->sval;
		qunlock(&symlock);
	}
	assert(a != nil);
	return a;
}

/* FIXME: silly to do on every call */
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
	Cell *c;

	assert(a != nil);
	qlock(&symlock);
	c = lookup(lab, a);
	qunlock(&symlock);
	return c;
}

static inline ioff
getnodeid(char *lab)
{
	ioff id;
	Cell *c;
	Array *a;

	if((a = gettabarray(Tnode)) == nil){
		werrstr("uninitialized table %s", lab);
		return -1;
	}
	if((c = getcell(lab, a)) == nil){
		werrstr("no such node %s", lab);
		return -1;
	}
	if((id = getival(c)) < 0){
		werrstr("invalid node %s", lab);
		return -1;
	}
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
		qlock(&symlock);
		c = setsymtab(lab, nil, v, STR, a);
		qunlock(&symlock);
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
		qlock(&symlock);
		c = setsymtab(lab, nil, v, NUM, a);
		qunlock(&symlock);
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
		qlock(&symlock);
		c = setsymtab(lab, nil, v, NUM|FLT, a);
		qunlock(&symlock);
		if(new != nil)
			*new = 1;
	}else
		setfval(c, f);
	return c;
}

/* FIXME: kind of feels like it's just native awk code... */
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
	DPRINT(Debugawk, "set %s type %d id %d", tabs[i].name, type, id);
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
			logerr(va("set CL[%s]: invalid non-integer color: %s\n", lab, type == Tstring ? val.s : ""));
			break;
		}
		if(setattr(i, id, val) < 0)
			DPRINT(Debugawk, "set CL: %s", error());
		c = setint(lab, val.u, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	case TLN:
		if(type != Tuint && type != Tint){
			DPRINT(Debuginfo, "not assigning string value %s to LN[%d]", val.s, id);
			return;
		}
		lab = getnodelabel(id);
		if(setattr(i, id, val) < 0)
			DPRINT(Debugawk, "set LN: %s", error());
		c = setint(lab, val.u, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	case Tdegree:
		if(type != Tuint && type != Tint){
			DPRINT(Debuginfo, "not assigning string value %s to degree[%d]", val.s, id);
			return;
		}
		lab = getnodelabel(id);
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
		if(setattr(i, id, val) < 0)
			DPRINT(Debugawk, "set fx/f0: %s", error());
		c = setfloat(lab, val.f, a, &new);
		if(new){
			free(c->nval);
			c->nval = lab;
		}
		break;
	default:
		if(setattr(i, id, val) < 0)
			DPRINT(Debugawk, "set: %s", error());
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

	qlock(&buflock);
	vs = valbuf;
	valbuf = nil;
	qunlock(&buflock);
	for(v=vs, ve=v+dylen(v); v<ve; v++)
		set(v->tab, v->type, v->id, v->val);
	dyfree(vs);
	USED(vs);
}

static inline void
pushval(int tab, int type, ioff id, V val)
{
	ssize n;
	Val v;

	v = (Val){tab, type, id, val};
	qlock(&buflock);
	dypush(valbuf, v);
	n = dylen(valbuf);
	qunlock(&buflock);
	if(n >= 64*1024){
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

	s = val;
	if(s[0] == '#'){
		s++;
		i = strtoull(s, &p, 16);
		if(iscolor && p - s < 8)	/* sigh */
			i = i << 8;
		type = Tuint;
	}else{
		i = strtoll(s, &p, 0);
		type = Tint;
	}
	if(p != s){
		if((c = *p) == '\0' || isspace(c)){
			v->i = i;
			return type;
		}else if(c == 'e' || c == 'E' || c == '.'){
			f = strtod(s, &p);
			if(p != s && ((c = *p) == '\0' || isspace(c))){
				v->f = f;
				return Tfloat;
			}
		}else
			type = Tstring;
	}else
		type = Tstring;
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
	if((id = getnodeid(name)) < 0)
		FATAL("%s", error());
	type = vartype(val, &v, i == TCL);
	if(debug & Debugawk){
		switch(type){
		case Tint: DPRINT(Debugawk, "setnamedtag %s[\"%s\":%d] = %lld (%s)", tag, name, id, v.i, val); break;
		case Tuint: DPRINT(Debugawk, "setnamedtag %s[\"%s\":%d] = %llx (%s)", tag, name, id, v.u, val); break;
		case Tfloat: DPRINT(Debugawk, "setnamedtag %s[\"%s\":%d] = %f (%s)", tag, name, id, v.f, val); break;
		case Tstring: DPRINT(Debugawk, "setnamedtag %s[\"%s\":%d] = \"%s\" (%s)", tag, name, id, v.s, val); break;
		}
	}
	pushval(i, type, id, v);
}

static void
fnloadall(void)
{
	static char already;
	ioff id;
	Node *n, *ne;
	RNode *r;
	V vv;

	if(already)
		return;
	already++;
	fnloadbatch();
	for(id=0, r=rnodes, n=nodes, ne=n+dylen(n); n<ne; n++, r++, id++){
		if(r->col[3] == 0.0f){
			vv.u = somecolor(id, nil);
			set(TCL, Tuint, id, vv);
		}
		/* FIXME: edges */
	}
	resizenodes();
}

/* FIXME: make variadic funs work in strawk itself,
 * then we won't need this shit; also for coarsening */
static TNode *
fnexplode(Cell *x, TNode *nextarg)
{
	int i;
	ioff idx, id;
	Cell *c;
	Array *a;

	if(nextarg == nil){
		a = getarray("selected");
		qlock(&symlock);
		for(i=0; i<a->size; i++){
			for(c=a->tab[i]; c!=nil; c=c->cnext){
				qunlock(&symlock);
				id = atoi(c->nval);
				if((idx = getnodeidx(id)) < 0)
					continue;
				explode(idx);
				qlock(&symlock);
			}
		}
		qunlock(&symlock);
		return nil;
	}
	for(;nextarg!=nil; nextarg=nextarg->nnext){
		tempfree(x);
		x = execute(nextarg);
		if((id = getnodeid(getsval(x))) < 0)
			FATAL("%s", error());
		if((idx = getnodeidx(id)) < 0){
			DPRINT(Debuggraph, "explode: ignoring inactive node %d", id);
			continue;
		}
		explode(idx);
	}
	return nil;
}

static TNode *
fnnodecolor(Cell *x, TNode *next)
{
	ioff id;
	char *lab;
	Cell *y;
	Array *a;
	V v;

	/* FIXME: more input validation */
	lab = getsval(x);
	y = execute(next);
	next = next->nnext;
	v.u = getival(y);
	if((a = gettabarray(TCL)) == nil)
		 sysfatal("awk/nodecolor: uninitialized table");
	if((id = getnodeid(lab)) < 0)
		FATAL("%s", error());
	setint(lab, v.u, a, nil);
	if(setattr(TCL, id, v) < 0)
		DPRINT(Debuggraph, "nodecolor: ignoring inactive node %d", id);
	tempfree(y);
	reqdraw(Reqrefresh);
	return next;
}

static void
fninfo(Cell *x)
{
	/* FIXME: more input validation; disable two-way comms, ie. don't
	 * allow interactive usage of internals; private functions? */
	strecpy(hoverstr, hoverstr+sizeof hoverstr, getsval(x));
	reqdraw(Reqshallowdraw);
}

static TNode *
fnunshow(Cell *x, TNode *next)
{
	ioff idx;
	u32int col;
	RNode *r;
	Cell *y;

	/* FIXME: more input validation, also which next is next */
	y = execute(next);
	next = next->nnext;
	if((idx = getnodeidx(getival(x))) < 0)
		return next;
	r = rnodes + idx;
	col = setalpha(getival(y));
	setcolor(r->col, col);
	tempfree(y);
	if((y = getcell("selinfo", symtab)) == nil)
		return next;
	strecpy(selstr, selstr+sizeof selstr, getsval(y));
	reqdraw(Reqrefresh);
	return next;
}

static void
fnrefresh(void)
{
	Cell *y;

	if((y = getcell("selinfo", symtab)) == nil)
		return;
	strecpy(selstr, selstr+sizeof selstr, getsval(y));
	reqdraw(Reqshallowdraw);
}

static void
fnrealedge(Cell *x, Cell *ret)
{
	ioff idx;
	u64int uv;

	idx = getival(x);
	uv = getrealedge(idx);
	setival(ret, uv);
}

static void
fnexpand1(Cell *x)
{
	double t;
	ioff id;

	id = getival(x);	/* cnode id */
	t = μsec();
	if(expand(id) < 0)
		logerr(va("expand %d: %s\n", id, error()));
	warn("expandone: %.2f ms\n", (μsec() - t) / 1000.0f);
}

/* erst die falafel, dann der wein */
static void
fncommit(void)
{
	if(commit() < 0)
		FATAL("%s", error());
}

/* FIXME: just always work on selection or everything, have a selectnodes()
 * that is the variadic function, or automatically select from results of
 * a calculation or expression, like CL[i] == red => select matching in
 * main.awk */
static TNode *
fncollapse(Cell *x, TNode *nextarg)
{
	int i, r, all;
	double t;
	ioff id, *ops;
	Cell *c;
	Array *a;

	t = μsec();
	if(buildct() < 0)	/* FIXME: do this asynchronously after mkgraph or import? */
		FATAL("collapse: %s", error());
	warn("collapsefn: buildct: %.2f ms\n", (μsec() - t) / 1000.0f);
	t = μsec();
	all = 0;
	ops = nil;
	/* FIXME: most of this doesn't need to be C */
	if(nextarg != nil){
		if((id = getnodeid(getsval(x))) < 0)
			FATAL("%s", error());
		ops = pushcollapseop(id, ops);
		for(;nextarg!=nil; nextarg=nextarg->nnext){
			tempfree(x);
			x = execute(nextarg);
			if((id = getnodeid(getsval(x))) < 0)
				FATAL("%s", error());
			ops = pushcollapseop(id, ops);
		}
	}else{
		a = getarray("selected");	/* sucks */
		qlock(&symlock);
		for(i=0; i<a->size; i++){
			for(c=a->tab[i]; c!=nil; c=c->cnext){
				qunlock(&symlock);
				id = atoi(c->nval);
				ops = pushcollapseop(id, ops);
				qlock(&symlock);
			}
		}
		qunlock(&symlock);
		if(ops == nil){
			all = 1;
			ops = collapseall();
		}
	}
	warn("collapsefn: collect: %.2f ms\n", (μsec() - t) / 1000.0f);
	t = μsec();
	r = all ? collapseup(ops) : collapsedown(ops);
	dyfree(ops);
	if(r < 0)
		FATAL("collapse: %s", error());
	warn("collapsefn: collapse: %.2f ms\n", (μsec() - t) / 1000.0f);
	t = μsec();
	if(coarsen() < 0)
		FATAL("collapse: %s", error());
	warn("collapsefn: coarsen: %.2f ms\n", (μsec() - t) / 1000.0f);
	/* strawk thread should not push commands -- but this should
	 * be awk code anyway */
	awkprint("U\n");	/* FIXME: kludge; unnecessary if this is in awk */
	return nil;
}

/* FIXME: stdin commands only get sent/updated on an event like mouse
 * move in window */

/* FIXME: fs: fsprint()/fseprint() functions or sth */
/* FIXME: should be in gfa.c or in awk */
static int
printgfa(char *path)	/* FIXME */
{
	char buf[2048], *p, *l, *vl;
	ioff x, *e, *ee;
	Node *u, *ue, *v;
	File *fs;

	if((fs = openfs(path, OWRITE)) == nil)
		return -1;
	p = seprint(buf, buf + sizeof buf, "H\tVN:Z:1.0\n");
	if(writefs(fs, buf, p - buf) < 0){
		freefs(fs);
		return -1;
	}
	for(u=nodes, ue=u+dylen(u); u<ue; u++){
		l = getnodelabel(u->id);
		p = seprint(buf, buf + sizeof buf, "S\t%s\t*\tLN:i:%d\n",
			l, u->length);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			v = nodes + (x >> 2);
			vl = getnodelabel(v->id);
			p = seprint(p, buf + sizeof buf, "L\t%s\t%c\t%s\t%c\t*\n",
				l, x&1?'-':'+', vl, x&2?'-':'+');
		}
		/* FIXME: detect trunctation or avoid it */
		if(writefs(fs, buf, p - buf) < 0){
			freefs(fs);
			return -1;
		}
	}
	freefs(fs);
	return 0;
}

static void
fnexportgfa(Cell *x)
{
	char *s;

	s = getsval(x);
	if(strlen(s) < 1)
		FATAL("invalid file path: %s", s);
	if(printgfa(s) < 0)
		FATAL("%s", error());
	logmsg("exportgfa: done\n");
}

static void
fnexportsvg(Cell *x)
{
	char *s;

	s = getsval(x);
	if(strlen(s) < 1)
		FATAL("invalid file path: %s", s);
	if(exportsvg(s) < 0)
		FATAL("%s", error());
}

/* NOTE: careful with FATAL in functions that may be part of scripts */
/* FIXME: stricter error checking and recovery */
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
	case ACOLLAPSE: nextarg = fncollapse(x, nextarg); break;
	case ACOMMIT: fncommit(); break;
	case AEXPAND1: fnexpand1(x); break;
	case AEXPANDALL: expandall(); break;
	case AEXPLODE: nextarg = fnexplode(x, nextarg); break;
	case AEXPORTGFA: fnexportgfa(x); break;
	case AEXPORTSVG: fnexportsvg(x); break;
	case AINFO: fninfo(x); break;
	case ALOAD: fnloadall(); break;
	case ALOADBATCH: fnloadbatch(); break;
	case ANODECOLOR: nextarg = fnnodecolor(x, nextarg); break;
	case AREALEDGE: fnrealedge(x, ret); break;
	case AREFRESH: fnrefresh(); break;
	case AUNSHOW: nextarg = fnunshow(x, nextarg); break;
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
		[Tdegree] = {"degree", nil, 1},
	}, *pp;

	initqlock(&buflock);
	initqlock(&symlock);
	initrwlock(&tablock);
	map = tab_init();
	for(pp=sptags; pp<sptags+nelem(sptags); pp++){
		i = mktab(pp->name, pp->nodeidx);
		tabs[i].fn = pp->fn;
	}
}
