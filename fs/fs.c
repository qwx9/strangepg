#include "strpg.h"
#include "fsprv.h"

static Filefmt *ff[FFnil];

Graph*
loadfs(int type, char *path)
{
	Filefmt *f;

	if(type < 0 || type >= FFnil){
		werrstr("invalid fs type");
		return nil;
	}
	f = ff[type];
	assert(f != nil);
	if(f->load == nil){
		werrstr("unimplemented fs type");
		return nil;
	}
	return f->load(path);
}

void
initfs(void)
{
	ff[FFgfa] = reggfa();
}
