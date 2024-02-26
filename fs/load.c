#include "strpg.h"
#include "fs.h"

static Filefmt *fftab[FFnil];

File *
graphopenfs(Graph *g, char *path, int mode)
{
	assert(g->f == nil);
	g->f = emalloc(sizeof *g->f);
	if(openfs(g->f, path, mode) < 0)
		return nil;
	return g->f;
}

int
loadfs(char *path, int type)
{
	Filefmt *ff;

	if(type <= FFdead || type >= nelem(fftab)){
		werrstr("invalid fs type");
		return -1;
	}
	ff = fftab[type];
	assert(ff != nil);
	if(ff->load == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	if(proccreate(ff->load, path, mainstacksize) < 0)
		return -1;
	return 0;
}

void
nukefs(Graph *g)
{
	Filefmt *ff;

	ff = fftab[g->type];
	if(g->type <= FFdead || g->type >= nelem(fftab)){
		warn("nukefs: invalid graph type %d", g->type);
		return;
	}
	ff->nuke(g);
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFindex] = regindex();
}
