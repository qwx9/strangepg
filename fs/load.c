#include "strpg.h"
#include "threads.h"
#include "fs.h"

static Filefmt *fftab[FFnil];

// FIXME: stupid; fix fs interface
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
	newthread(ff->load, nil, estrdup(path), nil, ff->name, mainstacksize);
	return 0;
}

/* FIXME: unused */
void
unloadfs(Graph *g)
{
	Filefmt *ff;

	ff = fftab[g->type];
	if(g->type <= FFdead || g->type >= nelem(fftab)){
		warn("unloadfs: invalid graph type %d", g->type);
		return;
	}
	ff->nuke(g);
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFindex] = regindex();
	fftab[FFcsv] = regcsv();
}
