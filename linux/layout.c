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
	g->layout.tid = 0;
	ll = g->layout.ll;
	dprint("new job job %d layout %s g %#p\n", getpid(), ll->name, g);
	ll->compute(g);
	renderlayout(g);
	g->layout.tid = -1;
	pthread_exit(NULL);
}

void
stoplayout(Graph *g)
{
	if(g->layout.tid < 0)
		return;
	pthread_kill(*(pthread_t*)g->layout.aux, SIGTERM);
	g->layout.tid = -1;
	memset(g->layout.aux, 0, sizeof(pthread_t));
}

void
runlayout(Graph *g)
{
	int r;
	pthread_t th;

	stoplayout(g);
	if(g->layout.aux == nil)
		g->layout.aux = emalloc(sizeof th);
	if((r = pthread_create(g->layout.aux, NULL, layproc, g)) != 0)
		warn("runlayout: pthread_create failed with err=%d", r);
}