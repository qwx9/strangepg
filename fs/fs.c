#include "strpg.h"
#include "fs.h"

void
closefs(File *f)
{
	free(f->s);
	f->s = nil;
	sysclose(f);
	/* not freeing f->path */
}

vlong
seekfs(File *f, vlong off)
{
	assert(f->aux != nil);
	f->foff = off;
	return sysseek(f, off);
}

int
writefs(File *f, void *buf, int n)
{
	assert(f->aux != nil && buf != nil);
	if(syswrite(f, buf, n) != n){
		if(debug & Debugtheworld){
			fprint(2, "writefs: short write: %r\n");
			abort();
		}
		sysfatal("writefs: short write: %r");
	}
	return 0;
}

int
readfs(File *f, void *buf, int n)
{
	assert(f->aux != nil && buf != nil);
	if((n = sysread(f, buf, n)) < 0)
		sysfatal("readfs: short read: %r");
	return n;
}

u8int
get8(File *f)
{
	uchar v;

	assert(f->aux != nil);
	if(sysread(f, &v, 1) <= 0)
		sysfatal("get8: short read: %r");
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
get64(File *f)
{
	u32int v;

	v = get32(f);
	return (u64int)get32(f) << 32 | v;
}

double
getdbl(File *f)
{
	union{ u64int v; double d; } u;

	u.v = get64(f);
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

int
opentmpfs(File *f, int mode)
{
	char *p;

	if((p = sysmktmp()) == nil)
		return -1;
	return openfs(f, p, mode);
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

void
freefs(File *f)
{
	if(f == nil)
		return;
	free(f->path);
	free(f);
}
