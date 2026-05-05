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

static int	foreachsel(int);

/* FIXME: have upfn use Cell as well */
void
deselectnode(Cell *cp)	/* deletion hook, do not call directly */
{
	ioff id;
	Value v;

	if((cp->tval & (NUM|STR)) == STR)
		return;
	id = getival(cp);
	v.u = core.colors[id];
	setnodecolor(id, v);
}

int
selectnodebyidx(ioff idx, int multi, int toggle)
{
	ioff id;
	char buf[64];
	Value v;

	if((id = getrealid(idx)) < 0)
		return -1;
	snprint(buf, sizeof buf, "%d", id);
	if(lookup(buf, core.sel) == nil){	/* needs fast path for drag */
		v.i = id;
		setsymtab(buf, EMPTY, v, NUM, core.sel);
		if(!multi && !toggle){
			pushcmd("reselectnode(%d)", id);
			flushcmd();
		}
		return 1;
	}else if(toggle){
		pushcmd("toggleselect(%d)", id);
		flushcmd();
	}
	return 0;
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
	rlock(&ap->lock);
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
	runlock(&ap->lock);
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

static TNode *
fnexplode(Cell *x, TNode *next)
{
	ioff id;
	float Δ;

	if(x == nil){
		foreachsel(AEXPLODE);
		return nil;
	}
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

static inline void
doexpand(Cell *x, int full)
{
	ioff id;

	id = getival(x);
	if(expand(id, full) < 0)
		FATAL("expand: %s", error());
}

static inline void
docollapse(Cell *x, int full)
{
	ioff id;

	id = getival(x);	/* cnode id */
	if(collapse(id, full) < 0)
		FATAL("collapse: %s", error());
}

static inline void
commitexpand(void)
{
	switch(uncoarsen()){
	case -1: FATAL("expand: %s", error()); break;
	case -2: WARNING("expand: %s", error()); break;
	}
}

static inline void
commitcollapse(void)
{
	switch(coarsen()){
	case -1: FATAL("collapse: %s", error()); break;
	case -2: WARNING("collapse: %s", error()); break;
	}
	/* strawk thread should not push commands -- but this should
	 * be awk code anyway */
	awkprint("U\n");	/* FIXME: kludge; unnecessary if this is in awk */
}

static TNode *
fnexpand(Cell *x, TNode *next, int full)
{
	vlong t;

	t = μsec();
	if(x == nil){
		if(foreachsel(full ? AFULLEXPAND : AEXPAND) == 0)
			expandall(full);
		TIME("awkext", "foreach", t);
		commitexpand();
		TIME("awkext", "uncoarsen", t);
		return nil;
	}
	doexpand(x, full);
	TIME("awkext", "doexpand", t);
	if(next != nil)
		return fnexpand(execute(next), next->nnext, full);
	else
		commitexpand();
	TIME("awkext", "commit", t);
	return nil;
}

/* FIXME: just always work on selection or everything, have a selectnodes()
 * that is the variadic function, or automatically select from results of
 * a calculation or expression, like CL[i] == red => select matching in
 * main.awk */
static TNode *
fncollapse(Cell *x, TNode *next, int full)
{
	vlong t;

	t = μsec();
	if(x == nil){
		if(foreachsel(full ? AFULLCOLLAPSE : ACOLLAPSE) == 0)
			collapseall(full);
		TIME("fncollapse", "foreach", t);
		commitcollapse();
		TIME("fncollapse", "commit", t);
		return nil;
	}
	docollapse(x, full);
	TIME("fncollapse", "docollapse", t);
	if(next != nil)
		return fncollapse(execute(next), next->nnext, full);
	else
		commitcollapse();
	TIME("fncollapse", "commit", t);
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
	logmsg("exportcoarse: done\n");
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

static void
fnarm(void)
{
	armgraph();
}

static int
foreachsel(int type)
{
	int i, n;
	ioff id;
	Array *a;
	Cell *c, *x;

	a = core.sel;
	for(n=i=0; i<a->size; i++)
		for(c=a->tab[i]; c!=nil; c=c->cnext){
			id = atoi(c->nval);
			x = gettemp(NUM);
			setival(x, id);
			switch(type){
			case ACOLLAPSE: docollapse(x, 0); break;
			case AEXPAND: doexpand(x, 0); break;
			case AEXPLODE: fnexplode(x, nil); break;
			case AFULLCOLLAPSE: docollapse(x, 1); break;
			case AFULLEXPAND: doexpand(x, 1); break;
			default:	/* can't happen */
				FATAL("illegal function type %d", type);
				break;
			}
			n++;
		}
	return n;
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
	if(isrec(x))	/* empty arguments list */
		x = nil;
	nextarg = a[1]->nnext;
	ret = gettemp(NUM);
	setival(ret, 0);
	switch(t){
	case AARM: fnarm(); break;
	case ACOLLAPSE: nextarg = fncollapse(x, nextarg, 0); break;
	case AEXPAND: nextarg = fnexpand(x, nextarg, 0); break;
	case AEXPLODE: nextarg = fnexplode(x, nextarg); break;
	case AEXPORTCOARSE: fnexportct(x); break;
	case AEXPORTGFA: fnexportgfa(x); break;
	case AEXPORTSVG: fnexportsvg(x); break;
	case AFULLCOLLAPSE: nextarg = fncollapse(x, nextarg, 1); break;
	case AFULLEXPAND: nextarg = fnexpand(x, nextarg, 1); break;
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
	if(x != nil)
		tempfree(x);
	return ret;
}

void
initext(void)
{
	initqlock(&buflock);
	qlock(&buflock);
}
