#include "strpg.h"
#include "fs.h"
#include <bio.h>

int
openfs(File *f, char *path)
{
	Biobuf *bf;

	assert(f->aux == nil && f->path == nil);
	f->path = path;
	if((bf = Bopen(f->path, OREAD)) == nil)
		return -1;
	f->aux = bf;
	return 0;
}

void
closefs(File *f)
{
	Biobuf *bf;

	free(f->s);
	f->s = nil;
	bf = f->aux;
	Bterm(bf);
}

vlong
seekfs(File *f, vlong off)
{
	assert(f->aux != nil);
	return Bseek(f->aux, off, 0);
}

u8int
get8(File *f)
{
	uchar v;
	Biobuf *bf;

	bf = f->aux;
	if(Bread(bf, &v, 1) != 1)
		sysfatal("get8: short read");
	return v;
}

u16int
get16(File *f)
{
	u8int v;

	v = get8(f);
	return (u16int)get8(f) << 8 | v;
}

u32int
get32(File *f)
{
	u16int v;

	v = get16(f);
	return (u32int)get16(f) << 16 | v;
}

u64int
get64(File *ff)
{
	u32int v;

	v = get32(ff);
	return (u64int)get32(ff) << 32 | v;
}

char *
readrecord(File *f)
{
	Biobuf *bf;

	assert(f->aux != nil);
	assert(f != nil && f->path != nil);
	bf = f->aux;
	free(f->s);
	f->s = nil;
	// FIXME: beware of \r
	if((f->s = Brdstr(bf, '\n', 0)) == nil){
		closefs(f);
		return nil;
	}
	if((f->nf = getfields(f->s, f->fld, nelem(f->fld), 1, "\t ")) < 1){
		free(f->s);
		f->s = nil;
		closefs(f);
		werrstr("line %d: invalid record", f->nr+1);
		return nil;
	}
	f->nr++;
	return f->s;
}
