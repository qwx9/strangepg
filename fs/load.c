#include "strpg.h"
#include "threads.h"
#include "fs.h"
#include "layout.h"

static Filefmt *fftab[FFnil];

int
loadfs(char *path, int type, int direct)
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
	if(path != nil)
		path = estrdup(path);
	if(direct)
		ff->load(path);
	else
		newthread(ff->load, nil, path, nil, ff->name, mainstacksize);
	return 0;
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFcsv] = regcsv();
	fftab[FFctab] = regctab();
}
