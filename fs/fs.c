#include "strpg.h"
#include "fs.h"

static Filefmt *ff[FFnil];

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
	if(syswrite(f, buf, n) != n)
		sysfatal("writefs: short write: %r");
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
put64(File *f, u64int v)
{
	uchar u[8];

	PBIT64(u, v);
	return writefs(f, u, sizeof u);
}

int
openfs(File *f, char *path, int mode)
{
	assert(f->aux == nil && f->path == nil);
	f->path = estrdup(path);
	return sysopen(f, mode);
}

File *
graphopenfs(char *path, int mode, Graph *g)
{
	assert(g->file == nil);
	g->file = emalloc(sizeof *g->file);
	if(openfs(g->file, path, mode) < 0)
		return nil;
	return g->file;
}

Graph*
loadfs(int type, char *path)
{
	Filefmt *f;

	if(type < 0 || type >= nelem(ff)){
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
