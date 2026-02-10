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
			free(l);	/* FIXME: Layout* is never actually reallocated? */
		}
	}
}

/* not really mooltigraph scheduler for now */
static void
sac(void *)
{
	ulong x;
	vlong t;
	int nidle;
	Layout *l;
	Channel **cp, **ce;

	nidle = 0;
	if(debug & Debugperf)
		t = μsec();
	for(;;){
		if((x = recvul(rxc)) == 0)
			break;
		x--;
		/* FIXME: too convoluted, too many assumptions; make the
		 * state machine more explicit -- make it an actual fsm */
		l = graph.layout;
		switch(x){
		case Lexport:
			DPRINT(Debuglayout, "sac: export");
			if(graph.flags & (GFlayme | GFdrawme)){
				pushcmd("exportlayout(layfile)");
				flushcmd();
			}
			continue;
		case Lreset:
			DPRINT(Debuglayout, "sac: reset");
			if(l != nil){
				l->flags |= LFstop | LFnuke;
				graph.flags |= GFlayme;
			}
			break;
		case Lfreeze:
			DPRINT(Debuglayout, "sac: freeze");
			graph.flags |= GFfrozen;
			if(nidle == nlaythreads){
				drawing.flags |= DFnolayout;
				continue;
			}
			/* wet floor */
		case Lstop:
			DPRINT(Debuglayout, "sac: stop");
			if(l != nil){
				l->flags |= LFstop;
				graph.flags &= ~GFlayme;
			}
			break;
		case Lthaw:
			DPRINT(Debuglayout, "sac: thaw");
			graph.flags &= ~GFfrozen;
			/* FIXME: thread unsafe non-atomic accesses; should be channel
			 * messages or just private state changes, or individual bitflags
			 * no one else writes to */
			drawing.flags &= ~DFnolayout;
			if(graph.flags & (GFlayme | GFdrawme) == 0)
				break;
			if(l != nil)
				l->flags |= LFclean;
			/* wet floor */
		case Lstart:
			DPRINT(Debuglayout, "sac: start");
			graph.flags |= GFlayme;
			break;
		case Lidle:
			nidle++;
			DPRINT(Debuglayout, "sac: idle %d/%d", nidle, nlaythreads);
			if(nidle > nlaythreads)
				sysfatal("sac: phase error");
			else if(nidle == nlaythreads && l != nil){
				if(graph.flags & GFfrozen){
					drawing.flags |= DFnolayout;
					continue;
				}else if(drawing.flags & DFarmed)	/* FIXME: better way? */
					drawing.flags &= ~DFnolayout;
				if((graph.flags & GFdrawme) == 0)
					break;
				TIME("layout", "total", t);
				logmsg("layout: done.\n");
				if(drawing.flags & DFnope){
					pushcmd("exportlayout(layfile)");
					pushcmd("quit()");
					flushcmd();
					break;
				}
				graph.flags &= ~GFdrawme;
				reqdraw(Reqredraw);
			}
			break;
		}
		if(l == nil || nidle != nlaythreads || (graph.flags & GFfrozen))
			continue;
		if(l->flags & (LFnuke | LFclean) && l->target->cleanup != nil && l->scratch != nil){
			l->target->cleanup(l->scratch);
			l->scratch = nil;
		}
		if((graph.flags & GFlayme) == 0){
			reqdraw(Reqrefresh);
			continue;
		}
		if(dylen(rnodes) < 1 || dylen(redges) < 1){
			warn("newlayout: nothing to layout\n");
			continue;
		}
		if(l->scratch == nil && l->target->new != nil){
			DPRINT(Debuglayout, "sac: reinit f=%d", l->flags);
			l->scratch = l->target->new(l->flags & LFnuke);
		}
		DPRINT(Debuglayout, "sac: launch f=%d", l->flags);
		l->flags = 0;
		if(debug & Debugperf)
			t = μsec();
		for(cp=txc, ce=cp+nlaythreads; cp<ce; cp++)
			sendp(*cp, l);
		nidle = 0;
		graph.flags = graph.flags & ~GFlayme | GFdrawme;
		reqdraw(Reqredraw);
		logmsg("computing layout...\n");
	}
}

/* FIXME: unbuffered channels are necessary to avoid races but may also
 * induce a lot of waiting, resulting in a freeze (sendp in sac) */
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

void
exportforever(void)
{
	for(;;){
		sleep(10000);
		reqlayout(Lexport);
	}
}

/* will queue events while layouting hasn't been initialized */
int
reqlayout(int type)
{
	switch(type){
	case Lfreeze:
	case Lthaw:
	case Lexport:
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
	l->flags = LFstop;
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
	return 0;
}

void
initlayout(void)
{
	ttab[LLpfr] = regpfr();
	ttab[LLpfr3d] = regpfr3d();
	if((rxc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %s", error());
}
