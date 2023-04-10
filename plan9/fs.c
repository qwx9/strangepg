#include "strpg.h"
#include "fsprv.h"
#include <bio.h>

static int
bopen(File *ff)
{
	Biobuf *bf;

	if((bf = Bopen(ff->path, OREAD)) == nil)
		return -1;
	ff->aux = bf;
	return 0;
}

static void
bterm(File *ff)
{
	Biobuf *bf;

	free(ff->s);
	ff->s = nil;
	bf = ff->aux;
	Bterm(bf);
}

char *
readrecord(File *ff)
{
	Biobuf *bf;

	dprint("readrecord %#p path \"%s\"\n", ff, ff->path);
	assert(ff != nil && ff->path != nil);
	if(ff->aux == nil && bopen(ff) < 0){
		ff->err++;
		return nil;
	}
	bf = ff->aux;
	free(ff->s);
	ff->s = nil;
	if((ff->s = Brdstr(bf, '\n', 0)) == nil){
		bterm(ff);
		return nil;
	}
	if((ff->nf = getfields(ff->s, ff->fld, nelem(ff->fld), 1, "\t ")) < 1){
		free(ff->s);
		ff->s = nil;
		bterm(ff);
		werrstr("line %d: invalid record", ff->nr+1);
		return nil;
	}
	ff->nr++;
	return ff->s;
}
