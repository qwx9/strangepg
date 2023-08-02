#include "strpg.h"
#include "fs.h"

static Filefmt *fftab[FFnil];

int
chlevel(Graph *g, int n)
{
	Filefmt *ff;

	assert(g->type >= 0 && g->type < nelem(fftab));
	stoplayout(g);
	ff = fftab[g->type];
	if(g == nil || ff->chlev == nil || n < 0 || n >= dylen(g->levels))
		return -1;
	if(g->infile == nil || dylen(g->levels) <= 0){
		werrstr("no loaded levels");
		return -1;
	}
	if(ff->chlev(g, n) < 0)
		return -1;
	return 0;
}

File *
graphopenfs(char *path, int mode, Graph *g)
{
	assert(g->infile == nil);
	g->infile = emalloc(sizeof *g->infile);
	if(openfs(g->infile, path, mode) < 0)
		return nil;
	return g->infile;
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
	if(ff->chlev != nil
	&& ff->chlev(g, g->level) < 0)
		return -1;
	return newlayout(g, -1);
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFindex] = regindex();
}
