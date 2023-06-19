#include "strpg.h"
#include "fs.h"

static Filefmt *fftab[FFnil];

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

vlong
tellfs(File *f)
{
	return sysftell(f);
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
	assert(g->infile == nil);
	g->infile = emalloc(sizeof *g->infile);
	if(openfs(g->infile, path, mode) < 0)
		return nil;
	return g->infile;
}

int
loadfs(char *path, int type)
{
	Filefmt *ff;
	Graph *g;

	if(type < 0 || type >= nelem(fftab)){
		werrstr("invalid fs type");
		return -1;
	}
	ff = fftab[type];
	assert(ff != nil);
	if(ff->load == nil){
		werrstr("unimplemented fs type");
		return -1;
	}
	if((g = ff->load(path)) == nil)
		return -1;
	g->type = type;
	if(ff->chlev != nil
	&& ff->chlev(g, 1) < 0)
		return -1;
	return newlayout(g, -1);
}

int
chlevel(Graph *g, int n)
{
	Filefmt *ff;

	assert(g->type >= 0 && g->type < nelem(fftab));
	stoplayout(g);
	ff = fftab[g->type];
	if(g == nil || ff->chlev == nil || n < 0 || n >= g->levels.len)
		return -1;
	if(g->infile == nil || g->nlevels <= 0 || g->levels.len <= 0){
		werrstr("no loaded levels");
		return -1;
	}
	if(ff->chlev(g, n) < 0)
		return -1;
	return 0;
}

void
initfs(void)
{
	fftab[FFgfa] = reggfa();
	fftab[FFindex] = regindex();
}
