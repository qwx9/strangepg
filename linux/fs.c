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

/* avoid hidden files, especially on error */
int
sysmktmp(void)
{
	int fd;
	char tmp[] = "_strpg.XXXXXX";

	if((fd = mkstemp(tmp)) < 0)
		return -1;
	if(unlink(tmp) < 0)
		warn("sysmktmp: %s\n", error());
	return fd;
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

/* this sucks in plan9 and in sucks in posix and linux: we can't use
 * getline(2) because it reads the entire line and reallocs the read
 * buffer, which defeats our purpose;  we have to use fgets(2)
 * instead, which doesn't handle NULs in input and doesn't give read
 * length information */
char *
readfrag(File *f, int *len)
{
	int n, m;
	char *s;

	assert(f != nil && f->path != nil && f->aux != nil);
	if(len != nil)
		*len = 0;
	if(f->trunc)
		f->trunc = 0;
	s = (char *)f->buf;
	if(fgets(s, sizeof f->buf, f->aux) == NULL)
		return nil;
	n = strlen(s);
	/* handle NULs in input */
	while(n < sizeof f->buf - 1 && (n < 1 || s[n-1] != '\n')){
		s[n] = '\x15';	/* nak */
		m = strlen(s + n);
		assert(m > 0);
		n += m;
	}
	if(s[n-1] == '\n')
		s[--n] = 0;
	else
		f->trunc = 1;
	if(len != nil)
		*len = n;
	return s;
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
