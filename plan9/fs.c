#include "strpg.h"
#include "fs.h"
#include <bio.h>

int
sysopen(File *f, int mode)
{
	Biobuf *bf;

	assert(f->path != nil);
	if((bf = Bopen(f->path, mode)) == nil)
		return -1;
	f->aux = bf;
	return 0;
}

void
sysclose(File *f)
{
	Bterm(f->aux);
	f->aux = nil;
}

void
sysflush(File *f)
{
	assert(f->aux != nil);
	Bflush(f->aux);
}

int
syswstatlen(File *f, vlong n)
{
	Dir d;
	Biobuf *bf;

	assert(f->aux != nil);
	nulldir(&d);
	d.length = n;
	bf = f->aux;
	if(dirfwstat(Bfildes(bf), &d) < 0)
		return -1;
	Bflush(bf);
	return 0;
}

vlong
sysseek(File *f, vlong off)
{
	return Bseek(f->aux, off, 0);
}

vlong
systell(File *f)
{
	return Bseek(f->aux, 0, 1);
}

int
syswrite(File *f, void *buf, int n)
{
	return Bwrite(f->aux, buf, n);
}

int
sysread(File *f, void *buf, int n)
{
	return Bread(f->aux, buf, n);
}

// FIXME: generic + os-specific
char *
readrecord(File *f)
{
	Biobuf *bf;

	assert(f->aux != nil);
	assert(f != nil && f->path != nil);
	bf = f->aux;
	free(f->s);
	f->foff = Boffset(bf);
	f->s = nil;
	// FIXME: beware of \r
	if((f->s = Brdstr(bf, '\n', 0)) == nil)
		return nil;
	if((f->nf = getfields(f->s, f->fld, nelem(f->fld), 1, "\t ")) < 1){
		free(f->s);
		f->s = nil;
		werrstr("line %d: invalid record", f->nr+1);
		return nil;
	}
	f->nr++;
	return f->s;
}
