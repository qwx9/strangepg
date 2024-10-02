#include "strpg.h"
#include "layout.h"
#include "drw.h"
#include "threads.h"

int nlaythreads = 4;
int deflayout = LLpfr;

static Shitkicker *sktab[LLnil];

/* FIXME: yuck, all this shit only because libchan doesn't fucking
 * implement blocking alt() */
enum{
	Layintr,
	Layidle,
	Laystop,
	Laynew,
	Layagain,
	Layhalt,
};
struct Layout{
	int flags;
	int nidle;
	Channel *c;
	Channel *wc;
	Shitkicker *sk;
	void *scratch;
};

static void
workproc(void *arg)
{
	ulong i;
	Layout *l;
	Shitkicker *sk;
	Clk clk = {.lab = "layworker"};

	l = arg;
	assert(l != nil);
	for(;;){
		sendul(l->c, Layidle);
		if((i = recvul(l->wc)) == 0)
			return;
		sk = l->sk;
		assert(sk != nil);
		if(sk->compute != nil){
			/* FIXME: no sampling == useless */
			CLK0(clk);
			sk->compute(l->scratch, &l->flags, i-1);
			CLK1(clk);
		}
	}
}

static void
layproc(void *arg)
{
	int i, new;
	ulong r;
	Graph *g;
	Layout *l;
	Shitkicker *sk, *oldsk;

	g = arg;
	sk = nil;
	new = 1;
	l = g->layout;
	for(;;){
		/* FIXME: yuck, if only libchan had alt */
		switch((r = recvul(l->c))){
		case Layintr:
			break;
		/* distinguishing halt vs. stop this ways keeps responsibility
		 * of updating sk here, leaving no bookkeeping in the main thread */
		case Layhalt:
			new = 1;
			/* wet floor */
		case Laystop:
			l->flags |= LFstop;
			sk = nil;
			reqdraw(Reqshape);
			g->flags &= ~GFdrawme;
			break;
		case Layagain:
			sk = l->sk != nil ? l->sk : sktab[deflayout];
			goto start;
		case Layidle:
			l->nidle++;
			if(l->nidle == nlaythreads)
				g->flags &= ~GFdrawme;
			if(!new)
				break;
			goto start;
		case Laynew:
		default:
			r -= Laynew;
			assert(r < LLnil);
			sk = sktab[r];
			/* wet floor */
		start:
			if(l->nidle < nlaythreads)	/* start once all workers are up */
				break;
			if(sk == nil){
				reqdraw(Reqshape);
				break;
			}
			l->flags = 0;
			oldsk = l->sk;
			if((new || sk != oldsk) && oldsk != nil){
				if(oldsk->cleanup != nil && l->scratch != nil){
					oldsk->cleanup(l->scratch);
					l->scratch = nil;
				}
			}
			l->sk = sk;
			if((new || sk != oldsk) && sk->new != nil)
				l->scratch = sk->new(g);
			else if(sk->update != nil)
				sk->update(g, l->scratch);
			g->flags |= GFdrawme;
			l->nidle = 0;
			reqdraw(Reqredraw);
			for(i=0; i<nlaythreads; i++)
				if(sendul(l->wc, i+1) < 0)
					return;
			new = 0;
			break;
		}
	}
}

static Layout *
newlayout(Graph *g)
{
	int i;
	Layout *l;

	l = emalloc(sizeof *l);
	if((l->c = chancreate(sizeof(ulong), 8)) == nil
	|| (l->wc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %s", error());
	g->layout = l;
	newthread(layproc, nil, g, nil, "layproc", mainstacksize);
	for(i=0; i<nlaythreads; i++)
		newthread(workproc, nil, l, nil, "werker", mainstacksize);
	return l;
}

/* FIXME */
static inline int
layoutreq(Layout *l, int req)
{
	if(sendul(l->c, req) < 0)
		return -1;
	return 0;
}

/* FIXME: sucks */
int
haltlayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("resetlayout: %#p uninitialized layout\n", g);
		return -1;
	}
	return layoutreq(l, Layhalt);
}

int
stoplayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("resetlayout: %#p uninitialized layout\n", g);
		return -1;
	}
	return layoutreq(l, Laystop);
}

int
updatelayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("resetlayout: %#p uninitialized layout\n", g);
		return -1;
	}
	return layoutreq(l, Layagain);
}

int
resetlayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("resetlayout: %#p uninitialized layout\n", g);
		return -1;
	}
	if(layoutreq(l, Layhalt) < 0)
		return -1;
	return runlayout(g, -1);
}

int
togglelayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("updatelayout: %#p uninitialized layout\n", g);
		return -1;
	}
	if((l->flags & LFstop) != 0)
		return layoutreq(l, Layagain);
	return layoutreq(l, Laystop);
}

int
runlayout(Graph *g, int type)
{
	Layout *l;

	if(dylen(g->nodes) < 1 || dylen(g->edges) < 1){
		werrstr("nothing to layout");
		return -1;
	}
	if((l = g->layout) == nil && (l = newlayout(g)) == nil)
		return -1;
	/* FIXME: yuck */
	if(type < 0)
		type = Layagain;
	else
		type += Laynew;
	if(sendul(l->c, type) < 0)
		return -1;
	return 0;
}

void
initlayout(void)
{
	sktab[LLconga] = regconga();
	sktab[LLrandom] = regrandom();
	sktab[LLfr] = regfr();
	sktab[LLlinear] = reglinear();
	sktab[LLpfr] = regpfr();
	sktab[LLpfr3d] = regpfr3d();
	sktab[LLcirc] = regcirc();
}
