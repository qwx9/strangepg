#include "strpg.h"
#include "layout.h"
#include "drw.h"
#include "threads.h"

int nlaythreads = 4;
int deflayout = LLfr;

static Shitkicker *sktab[LLnil];

/* FIXME: yuck, all this shit only because libchan doesn't fucking
 * implement blocking alt() */
enum{
	Layidle = 1,
	Laystop,
	Layagain,
	Laynew,
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
	int i, new, again;
	ulong r;
	Graph *g;
	Layout *l;
	Shitkicker *sk, *oldsk;
	Clk clk = {.lab = "layproc"};

	g = arg;
	sk = nil;
	new = again = 1;
	l = g->layout;
	for(;;){
		/* FIXME: yuck, if only libchan had alt */
		switch((r = recvul(l->c))){
		case 0: break;
		case Laystop:
			l->flags |= LFstop;
			sk = nil;
			new = 1;
			reqdraw(Reqshape);
			break;
		default:
			again = 1;
			if(r == Layagain)
				sk = l->sk != nil ? l->sk : sktab[deflayout];
			else{
				r -= Laynew;
				assert(r < LLnil);
				sk = sktab[r];
			}
			goto start;
		case Layidle:
			l->nidle++;
			/* wet floor */
		start:
			if(l->nidle < nlaythreads)
				break;
			l->flags = 0;
			if(!again || sk == nil){
				g->flags &= ~GFdrawme;
				reqdraw(Reqshape);
				break;
			}
			oldsk = l->sk;
			if((new || sk != oldsk) && oldsk != nil){
				g->flags &= ~GFlayme;
				if(oldsk->cleanup != nil && l->scratch != nil){
					l->sk->cleanup(l->scratch);
					l->scratch = nil;
				}
			}
			l->sk = sk;
			if((new || sk != oldsk) && sk->new != nil){
				/* FIXME: no sampling == useless */
				CLK0(clk);
				l->scratch = sk->new(g);
				CLK1(clk);
				g->flags |= GFlayme;
			}else if(sk->update != nil)
				sk->update(g, l->scratch);
			again = new = 0;
			g->flags |= GFdrawme;
			l->nidle = 0;
			startdrawclock();
			for(i=0; i<nlaythreads; i++)
				if(sendul(l->wc, i+1) < 0)
					return;
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

int
stoplayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil)
		return 0;
	if(sendul(l->c, Laystop) < 0)
		return -1;
	return 0;
}

int
resetlayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("resetlayout: %#p uninitialized layout\n", g);
		return -1;
	}
	if(stoplayout(g) < 0)
		return -1;
	return runlayout(g, -1);
}

int
updatelayout(Graph *g)
{
	Layout *l;

	if((l = g->layout) == nil || l->sk == nil){
		werrstr("updatelayout: %#p uninitialized layout\n", g);
		return -1;
	}
	return runlayout(g, -1);
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
	sktab[LLpline] = regpline();
}
