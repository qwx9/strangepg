#include "strpg.h"
#include "threads.h"
#include "drw.h"
#include "fs.h"
#include "layout.h"
#include "ui.h"
#include "graph.h"
#include "cmd.h"
#include "strawk/awk.h"
#include "var.h"

/* NOTE: pushcmd here is asking for trouble. fs/gfa and others
 * may be saturating the cmd buffer and we're the only reader. */

QLock buflock;

int
selectnodebyidx(ioff idx, int toggle)
{
	ioff id;
	char buf[64];
	Cell *cp;
	Value v;

	if((id = getrealid(idx)) < 0)
		return -1;
	snprint(buf, sizeof buf, "%d", id);
	v.i = id;
	cp = setsymtab(buf, EMPTY, v, CON|NUM, core.sel);
	if(cp->nval != buf){
		if(toggle){
			cp = lookup("selected", symtab);	/* FIXME */
			freeelem(cp, buf);
		}
		return 0;
	}
	cp->nval = estrdup(buf);
	cp->tval &= ~CON;
	return 1;
}

void
dragselection(float Δx, float Δy, void (*fn)(ioff, float, float))
{
	int i;
	ioff idx;
	Value v;
	
	Cell *cp;
	Array *ap;

	ap = core.sel;
	for(i=0; i<ap->size; i++){
		for(cp=ap->tab[i]; cp!=nil; cp=cp->cnext){
			//v.u = strtoull(cp->nval, nil, 10);
			v.i = getival(cp);
			if((idx = getnodeidx(v.i)) < 0){
				DPRINT(Debugui, "dragselection: %s", error());
				continue;
			}
			fn(idx, Δx, Δy);
		}
	}
}

void
showselection(void)
{
	Array *ap;

	ap = core.sel;
	if(ap->nelem > 0){
		pushcmd("showselected()");
		flushcmd();
	}
}

int
selectionsize(void)
{
	return core.sel->nelem;
}

static inline void
fnloadbatch(void)
{
	loadbatch();
}

static TNode *
fnexplode1(Cell *x, TNode *next)
{
	ioff id;
	float Δ;
	
	id = getival(x);
	if(next != nil){
		x = execute(next);
		Δ = getfval(x);
		tempfree(x);
		next = next->nnext;
	}else
		Δ = 8.0f;
	explode(id, Δ);
	return next;
}

static void
fninfo(Cell *x)
{
	strecpy(hoverstr, hoverstr+sizeof hoverstr, getsval(x));
	reqdraw(Reqshallowdraw);
}

static void
fnrefresh(void)
{
	Cell *y;

	y = lookup("selinfo", symtab);
	if(y == nil)
		return;
	strecpy(selstr, selstr+sizeof selstr, getsval(y));
	reqdraw(Reqshallowdraw);
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
fncommit(Cell *x)
{
	ioff shutup;

	shutup = getival(x);
	if(commit(shutup) < 0)
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
		a = core.sel;
		for(i=0; i<a->size; i++){
			for(c=a->tab[i]; c!=nil; c=c->cnext){
				id = atoi(c->nval);
				ops = pushcollapseop(id, ops);
			}
		}
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
	case ACOMMIT: fncommit(x); break;
	case AEXPAND1: fnexpand1(x); break;
	case AEXPANDALL: expandall(); break;
	case AEXPLODE1: nextarg = fnexplode1(x, nextarg); break;
	case AEXPORTCOARSE: fnexportct(x); break;
	case AEXPORTGFA: fnexportgfa(x); break;
	case AEXPORTSVG: fnexportsvg(x); break;
	case AINFO: fninfo(x); break;
	case ALOADBATCH: fnloadbatch(); break;
	case AREFRESH: fnrefresh(); break;
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
	qlock(&buflock);
}
