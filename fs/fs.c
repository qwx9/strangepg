#include "strpg.h"
#include "fs.h"

/* not freeing f->path for later reuse */
void
closefs(File *f)
{
	sysclose(f);
}

void
freefs(File *f)
{
	if(f == nil)
		return;
	if(f->aux != nil)
		closefs(f);
	free(f->path);
	free(f);
}

vlong
seekfs(File *f, vlong off)
{
	assert(f->aux != nil);
	// FIXME: test this
	f->trunc = 0;
	f->foff = off;
	return sysseek(f, off);
}

int
setsizefs(File *f, vlong sz)
{
	return syswstatlen(f, sz);
}

int
writefs(File *f, void *buf, int n)
{
	int m;

	assert(f->aux != nil && buf != nil);
	if((m = syswrite(f, buf, n)) != n){
		if(debug & Debugtheworld){
			warn("writefs: short write %d not %d: %s\n", m, n, error());
			abort();
		}
		sysfatal("writefs: short write %d not %d: %s", m, n, error());
	}
	return 0;
}

void
flushfs(File *f)
{
	sysflush(f);
}

int
readfs(File *f, void *buf, int n)
{
	int m;

	assert(f->aux != nil && buf != nil);
	if((m = sysread(f, buf, n)) < 0){
		if(debug & Debugtheworld){
			warn("readfs: short read %d not %d: %s\n", m, n, error());
			abort();
		}
		sysfatal("readfs: short read %d not %d: %s\n", m, n, error());
	}
	return m;
}

/* ugh */
int
get8(File *f, u8int *v)
{
	int m;
	uchar u[sizeof *v];

	if((m = sysread(f, u, sizeof u)) <= 0)
		return m;
	*v = GBIT8(u);
	return 0;
}

int
get16(File *f, u16int *v)
{
	int m;
	uchar u[sizeof *v];

	if((m = sysread(f, u, sizeof u)) <= 0)
		return m;
	*v = GBIT16(u);
	return 0;
}

int
get32(File *f, u32int *v)
{
	int m;
	uchar u[sizeof *v];

	if((m = sysread(f, u, sizeof u)) <= 0)
		return m;
	*v = GBIT32(u);
	return 0;
}

int
get64(File *f, u64int *v)
{
	int m;
	uchar u[sizeof *v];

	if((m = sysread(f, u, sizeof u)) <= 0)
		return m;
	*v = GBIT64(u);
	return 0;
}

u8int
eget8(File *f)
{
	int m;
	uchar u[1];

	assert(f->aux != nil);
	if((m = sysread(f, u, sizeof u)) <= 0)
		sysfatal("get8: short read %d: %s\n", m, error());
	return GBIT8(u);
}

u16int
eget16(File *f)
{
	int m;
	uchar u[2];

	assert(f->aux != nil);
	if((m = sysread(f, u, sizeof u)) <= 0)
		sysfatal("get16: short read %d: %s", m, error());
	return GBIT16(u);
}

u32int
eget32(File *f)
{
	int m;
	uchar u[4];

	assert(f->aux != nil);
	if((m = sysread(f, u, sizeof u)) <= 0)
		sysfatal("get32: short read %d: %s", m, error());
	return GBIT32(u);
}

u64int
eget64(File *f)
{
	int m;
	uchar u[8];

	assert(f->aux != nil);
	if((m = sysread(f, u, sizeof u)) <= 0)
		sysfatal("get64: short read %d: %s", m, error());
	return GBIT64(u);
}

double
egetdbl(File *f)
{
	union{ u64int v; double d; } u;

	u.v = eget64(f);
	return u.d;
}

int
put32(File *f, u32int v)
{
	uchar u[4];

	PBIT32(u, v);
	return writefs(f, u, sizeof u);
}

int
put64(File *f, u64int v)
{
	uchar u[8];

	PBIT64(u, v);
	return writefs(f, u, sizeof u);
}

vlong
tellfs(File *f)
{
	return sysftell(f);
}

/* FIXME: sucks */
char *
nextfield(File *f, char *s, int *len, char sep)
{
	int n;
	char *t;

	if(len != nil)
		*len = 0;
	if(s == nil || *s == 0)
		return nil;
	if((t = strchr(s, sep)) != nil){
		if(len != nil)
			*len = t - s;
		*t++ = 0;
		return t;
	}
	if(len != nil)
		*len = strlen(s);
	while(f != nil && f->trunc){
		if((s = readfrag(f, &n)) == nil)
			return nil;
		if((t = strchr(s, sep)) != nil){
			if(len != nil)
				*len += t - s;
			*t++ = 0;
			break;
		}else
			*len += n;
	}
	return t;
}

char *
readline(File *f, int *len)
{
	/* previous line unterminated, longer than size of buffer */
	while(f->trunc)
		readfrag(f, nil);
	f->foff = sysftell(f);
	f->nr++;
	return readfrag(f, len);
}

int
openfs(File *f, char *path, int mode)
{
	assert(f->aux == nil && f->path == nil);
	f->path = estrdup(path);
	return sysopen(f, mode);
}

int
fdopenfs(File *f, int fd, int mode)
{
	return sysfdopen(f, fd, mode);
}
