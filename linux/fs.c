#include "strpg.h"
#include "fs.h"
#include <sys/stat.h>

static char*
modestr(int omode)
{
	char *mode;

	mode = "nein";
	switch(omode){
	case OREAD: mode = "rb"; break;
	case OWRITE: mode = "wb"; break;
	case ORDWR: mode = "w+b"; break;
	default: sysfatal("sysopen: unknown file mode");
	}
	return mode;
}

void
sysclose(File *f)
{
	fclose(f->aux);
	f->aux = nil;
}

int
sysopen(File *f, int omode)
{
	int fd;
	char *mode;
	FILE *bf;

	assert(f->path != nil);
	mode = modestr(omode);
	if((bf = fopen(f->path, mode)) == NULL){
		warn("sysopen \"%s\": %s\n", f->path, error());
		return -1;
	}
	f->aux = bf;
	return 0;
}

int
sysfdopen(File *f, int fd, int omode)
{
	FILE *bf;
	char *mode;

	mode = modestr(omode);
	if((bf = fdopen(fd, mode)) == NULL){
		warn("sysfdopen %d: %s\n", fd, error());
		return -1;
	}
	f->aux = bf;
	return 0;
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
}

vlong
sysftell(File *f)
{
	return ftell(f->aux);
}

void
sysremove(char *path)
{
	if(remove(path) < 0)
		warn("remove %s: %s\n", path, error());
}

char *
sysmktmp(void)
{
	int fd;
	char s[64];

	snprintf(s, sizeof s, ".strpg.%d.%06x", getpid(), rand());
	return estrdup(s);
}

int
syswrite(File *f, void *buf, int n)
{
	int m;

	if((m = fwrite(buf, 1, n, f->aux)) != n)
		return -1;
	return m;
}

int
sysread(File *f, void *buf, int n)
{
	int m;

	m = fread(buf, 1, n, f->aux);
	if(ferror(f->aux))
		return -1;
	else
		return m;
}

/* FIXME: deprecated
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
*/
