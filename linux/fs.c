#include "strpg.h"
#include "fs.h"
#include <stdio.h>
#include <sys/stat.h>

int
sysopen(File *f, int omode)
{
	int fd;
	char *mode;
	FILE *bf;

	assert(f->path != nil);
	mode = "nein";
	switch(omode){
	case OREAD: mode = "r"; break;
	case OWRITE: mode = "w"; break;
	case ORDWR: mode = "rw"; break;
	default: sysfatal("sysopen: unknown file mode");
	}
	if((bf = fopen(f->path, mode)) == nil)
		return -1;
	f->aux = bf;
	return 0;
}

void
sysclose(File *f)
{
	fclose(f->aux);
	f->aux = nil;
}

void
sysflush(File *f)
{
	assert(f->aux != nil);
	fflush(f->aux);
}

/* there has to be a better wstat equivalent than this */
int
syswstatlen(File *f, vlong n)
{
	FILE *bf;
	struct stat st;

	bf = f->aux;
	fstat(fileno(bf), &st);
	if(n <= st.st_size)
		return 0;
	sysseek(f, n);
	syswrite(f, &n, 1);
	return 0;
}

vlong
sysseek(File *f, vlong off)
{
	return fseek(f->aux, off, 0);
	return 0;
}

vlong
sysftell(File *f)
{
	return ftell(f->aux);
}

int
syswrite(File *f, void *buf, int n)
{
	return fwrite(buf, n, 1, f->aux);
}

int
sysread(File *f, void *buf, int n)
{
	return fread(buf, n, 1, f->aux);
}

/* FIXME: proper errno + string translation usage for unix */
char *
readrecord(File *f)
{
	ssize_t n;
	size_t sz;

	assert(f != nil && f->path != nil);
	assert(f->aux != nil);
	f->foff = sysftell(f);
	if(f->s != nil)
		free(f->s);
	f->s = nil;
	sz = 0;
	if((n = getline(&f->s, &sz, f->aux)) < 0)
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
