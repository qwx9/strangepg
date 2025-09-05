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

extern QLock symlock;

/* NOTE: pushcmd here is asking for trouble. fs/gfa and others
 * may be saturating the cmd buffer and we're the only reader. */

QLock buflock;

/* FIXME: remove */
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
	return a;
}

/* FIXME: remove (?) */
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

static inline void
fnloadbatch(void)
{
	loadbatch();
}

static inline void
fnloadall(void)
{
	loadvars();
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
		if((id = getid(getsval(x))) < 0)
			FATAL("%s", error());
		if((idx = getnodeidx(id)) < 0){
			DPRINT(Debuggraph, "explode: ignoring inactive node %d", id);
			continue;
		}
		explode(idx);
	}
	return nil;
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
	col = setdefalpha(getival(y));
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
	ioff id;

	id = getival(x);	/* cnode id */
	if(expand(id) < 0)
		logerr(va("expand %d: %s\n", id, error()));
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
	vlong t;
	ioff id, *ops;
	Cell *c;
	Array *a;

	t = μsec();
	if(buildct() < 0)	/* FIXME: do this asynchronously after mkgraph or import? */
		FATAL("collapse: %s", error());
	TIME("collapse", "buildct", t);
	all = 0;
	ops = nil;
	/* FIXME: most of this doesn't need to be C */
	if(nextarg != nil){
		if((id = getid(getsval(x))) < 0)
			FATAL("%s", error());
		ops = pushcollapseop(id, ops);
		for(;nextarg!=nil; nextarg=nextarg->nnext){
			tempfree(x);
			x = execute(nextarg);
			if((id = getid(getsval(x))) < 0)
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
	TIME("collapse", "collect", t);
	r = all ? collapseup(ops) : collapsedown(ops);
	dyfree(ops);
	if(r < 0)
		FATAL("collapse: %s", error());
	TIME("collapse", "collapseop", t);
	if(coarsen() < 0)
		FATAL("collapse: %s", error());
	TIME("collapse", "coarsen", t);
	/* strawk thread should not push commands -- but this should
	 * be awk code anyway */
	awkprint("U\n");	/* FIXME: kludge; unnecessary if this is in awk */
	USED(ops);
	USED(t);
	return nil;
}

/* FIXME: stdin commands only get sent/updated on an event like mouse
 * move in window */

static void
fnexportct(Cell *x)
{
	char *s;

	s = getsval(x);
	if(strlen(s) < 1)
		FATAL("invalid file path: %s", s);
	if(exportct(s) < 0)
		FATAL("%s", error());
}

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
		l = getname(u->id);
		p = seprint(buf, buf + sizeof buf, "S\t%s\t*\tLN:i:%lld\n",
			l, u->length);
		for(e=edges+u->eoff, ee=e+u->nedges; e<ee; e++){
			x = *e;
			v = nodes + (x >> 2);
			vl = getname(v->id);
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
	ret = gettemp(NUM);
	setival(ret, 0);
	switch(t){
	case ACOLLAPSE: nextarg = fncollapse(x, nextarg); break;
	case ACOMMIT: fncommit(); break;
	case AEXPAND1: fnexpand1(x); break;
	case AEXPANDALL: expandall(); break;
	case AEXPLODE: nextarg = fnexplode(x, nextarg); break;
	case AEXPORTCOARSE: fnexportct(x); break;
	case AEXPORTGFA: fnexportgfa(x); break;
	case AEXPORTSVG: fnexportsvg(x); break;
	case AINFO: fninfo(x); break;
	case ALOAD: fnloadall(); break;
	case ALOADBATCH: fnloadbatch(); break;
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
	initqlock(&buflock);
}
