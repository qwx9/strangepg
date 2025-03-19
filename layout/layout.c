#include "strpg.h"
#include "layout.h"
#include "drw.h"
#include "threads.h"
#include "cmd.h"
#include "fs.h"

int nlaythreads = 4;
int deflayout = -1;

static Target *ttab[LLnil];

struct Layout{
	int ref;
	int flags;
	Target *target;
	void *scratch;
};

static Channel **txc, *rxc;

static void
wing(void *arg)
{
	ulong id;
	Channel *c;
	Layout *l;
	Target *t;

	id = (uintptr)arg;
	c = txc[id];
	for(;;){
		sendul(rxc, Lidle+1);
		if((l = recvp(c)) == nil)
			break;
		l->ref++;
		t = l->target;
		if(t->compute != nil)
			t->compute(l->scratch, &l->flags, id);
		if(--l->ref == 0){
			if(t->cleanup != nil)
				t->cleanup(l->scratch);
			free(l);
		}
	}
}

/* not really mooltigraph scheduler for now */
static void
sac(void *)
{
	ulong x;
	int nidle;
	Layout *l;
	Channel **cp, **ce;

	nidle = 0;
	l = nil;
	for(;;){
		if((x = recvul(rxc)) == 0)
			break;
		x--;
		l = graph.layout;
		switch(x & 3){
		case Lreset:
			if(l != nil){
				l->flags |= LFstop | LFnuke;
				graph.flags |= GFlayme;
			}
			break;
		case Lstop:
			if(l != nil){
				l->flags |= LFstop;
				graph.flags &= ~GFlayme;
			}
			break;
		case Lstart:
			graph.flags |= GFlayme;
			break;
		case Lidle:
			nidle++;
			if(nidle > nlaythreads)
				sysfatal("sac: phase error");
			else if(nidle == nlaythreads && l != nil){
				if((graph.flags & GFdrawme) == 0)
					break;
				if(drawing.flags & DFnope){
					pushcmd("exportlayout(layfile)");
					pushcmd("quit()");
					flushcmd();
					break;
				}
				logmsg("layout: done.\n");
				graph.flags &= ~GFdrawme;
				reqdraw(Reqredraw);
			}
			break;
		}
		if(l == nil || nidle != nlaythreads)
			continue;
		if((l->flags & LFnuke) != 0 && l->target->cleanup != nil && l->scratch != nil){
			l->target->cleanup(l->scratch);
			l->scratch = nil;
		}
		l->flags = 0;
		if((graph.flags & GFlayme) == 0){
			reqdraw(Reqrefresh);
			continue;
		}
		if(dylen(rnodes) < 1 || dylen(redges) < 1){
			warn("newlayout: nothing to layout\n");
			continue;
		}
		resizenodes();	/* FIXME: unfortunate dependency */
		if(l->scratch == nil && l->target->new != nil)
			l->scratch = l->target->new();
		for(cp=txc, ce=cp+nlaythreads; cp<ce; cp++)
			sendp(*cp, l);
		nidle = 0;
		graph.flags &= ~GFlayme;
		graph.flags |= GFdrawme;
		reqdraw(Reqredraw);
		logmsg("computing layout...\n");
	}
}

static void
opencomms(void)
{
	int i;
	Channel **c;

	txc = emalloc(nlaythreads * sizeof *txc);
	for(c=txc, i=0; i<nlaythreads; i++, c++)
		if((*c = chancreate(sizeof(Layout*), 0)) == nil)
			sysfatal("chancreate: %s", error());
	newthread(sac, nil, nil, nil, "sac", mainstacksize);
	for(c=txc, i=0; i<nlaythreads; i++, c++)
		newthread(wing, nil, (void*)(intptr)i, nil, "wing", mainstacksize);
}

/* will queue events while layouting hasn't been initialized */
int
reqlayout(int type)
{
	switch(type){
	case Lstop:
		break;
	case Lreset:
	case Lstart:
		if((graph.flags & GFdrawme) != 0)
			sendul(rxc, Lstop+1);
		break;
	case Lidle:
	default:
		warn("reqlayout: unknown req %d\n", type);
		return -1;
	}
	sendul(rxc, type+1);
	return 0;
}

int
newlayout(int type)
{
	Layout *l;

	if(type >= LLnil){
		werrstr("newlayout: invalid layout %d\n", type);
		return -1;
	}
	if((l = graph.layout) != nil)
		l->ref--;
	l = emalloc(sizeof *l);
	graph.layout = l;
	l->ref = 1;
	if(type < 0){
		if(deflayout < 0)
			type = LLpfr;
		else
			type = deflayout;
	}
	if(type == LLpfr3d)
		drawing.flags |= DF3d;
	l->target = ttab[type];
	if(txc == nil)
		opencomms();
	/* guarantees initialized state for deferred loading */
	if(!gottagofast){
		if(l->scratch == nil && l->target->init != nil)
			l->scratch = l->target->init();
	}
	return 0;
}

void
initlayout(void)
{
	ttab[LLpfr] = regpfr();
	ttab[LLpfr3d] = regpfr3d();
	if((rxc = chancreate(sizeof(ulong), 1)) == nil)
		sysfatal("chancreate: %s", error());
}
