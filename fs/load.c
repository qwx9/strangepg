#include "strpg.h"
#include "threads.h"
#include "fs.h"
#include "layout.h"

static Filefmt *fftab[FFnil];

static void
cleanup(void *path)
{
	free(path);
}

static int
forkfs(char *path, int type, int issave)
{
	void (*fp)(void *);
	Filefmt *ff;

	if(type <= FFdead || type >= nelem(fftab)){
		werrstr("invalid fs type");
		return -1;
	}
	ff = fftab[type];
	if(ff == nil){
		werrstr("unimplemented type");
		return -1;
	}
	fp = issave ? ff->save : ff->load;
	if(fp == nil){
		werrstr("%s: %s unimplemented", ff->name,
			issave ? "load" : "save");
		return -1;
	}
	if(path != nil)
		path = estrdup(path);
	newthread(fp, cleanup, path, path, ff->name, mainstacksize);
	return 0;
}

int
savefs(char *path, int type, int direct)
{
	Filefmt *ff;

	if(!direct)
		return forkfs(path, type, 0);
	if(type <= FFdead || type >= nelem(fftab)){
		werrstr("invalid fs type");
		return -1;
	}
	ff = fftab[type];
	assert(ff != nil);
	if(ff->save == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	ff->save(path);
	return 0;
}

int
loadfs(char *path, int type, int direct)
{
	Filefmt *ff;

	if(!direct)
		return forkfs(path, type, 0);
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
	ff->load(path);
	return 0;
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFcsv] = regcsv();
	fftab[FFctab] = regctab();
}
