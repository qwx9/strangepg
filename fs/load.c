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
	Graph *g;

	if(type < 0 || type >= nelem(fftab)){
		werrstr("invalid fs type");
		return -1;
	}
	ff = fftab[type];
	assert(ff != nil);
	if(ff->load == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	if((g = ff->load(path)) == nil)
		return -1;
	g->type = type;
	return newlayout(g, -1);
}

void
nukefs(Graph *g)
{
	Filefmt *ff;

	ff = fftab[g->type];
	if(g->type < 0 || g->type >= nelem(fftab)){
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
