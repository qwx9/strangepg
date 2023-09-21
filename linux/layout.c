#include "strpg.h"
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static void*
layproc(void *gp)
{
	Graph *g;
	Layout *ll;

	g = gp;
	ll = g->layout.ll;
	g->layout.tid = 0;
	dprint(Debuglayout, "new job job %d layout %s g %#p\n", getpid(), ll->name, g);
	coffeetime();
	ll->compute(g);
	coffeeover();
	g->layout.tid = -1;
	reqdraw(Reqrefresh);
	pthread_exit(NULL);
}

void
stoplayout(Graph *g)
{
	if(g->layout.tid < 0)
		return;
	pthread_cancel(*(pthread_t*)g->layout.aux);
	g->layout.tid = -1;
	memset(g->layout.aux, 0, sizeof(pthread_t));
}

void
runlayout(Graph *g)
{
	int r;
	pthread_t th;

	if(g->layout.aux == nil)
		g->layout.aux = emalloc(sizeof th);
	if((r = pthread_create(g->layout.aux, NULL, layproc, g)) != 0)
		warn("runlayout: pthread_create failed with err=%d", r);
}
