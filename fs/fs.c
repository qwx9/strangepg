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

int
seekfs(File *f, vlong off)
{
	assert(f->aux != nil);
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
		if(debug & Debugtheworld)
			panic("writefs: short write %d not %d: %s", m, n, error());
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
		if(debug & Debugtheworld)
			panic("readfs: short read %d not %d: %s", m, n, error());
		sysfatal("readfs: short read %d not %d: %s", m, n, error());
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
		sysfatal("get8: short read %d: %s", m, error());
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

static inline char *
terminate(File *f, char *cur, char *end)
{
	f->toksz += end - cur;
	*end++ = 0;
	f->tok = end;
	return cur;
}

char *
nextfield(File *f)
{
	int n;
	char *s, *t, *p;

	f->foff += f->toksz;
	if(f->nf++ > 0)
		f->foff++;	/* sep */
	s = f->tok;
	n = f->end - s;
	f->toksz = 0;
	if(n <= 0)
		return nil;
	if((t = memchr(s, f->sep, n)) != nil)
		return terminate(f, s, t);
	else if(!f->trunc)	/* EOL */
		return terminate(f, s, s + n);
	p = f->end - f->len - n;
	memmove(p, s, n);
	if((s = readfrag(f)) == nil)
		goto err;
	else if((t = memchr(s, f->sep, f->len)) != nil)
		return terminate(f, p, t);
	else if(!f->trunc)	/* EOL */
		return terminate(f, p, s + f->len);
	/* field is longer than the read size, giving up for this one */
	f->toksz += f->end - p;
	while(f->trunc){
		if((s = readfrag(f)) == nil)
			goto err;
		else if((t = memchr(s, f->sep, f->len)) != nil){
			terminate(f, s, t);
			return nil;
		}else
			f->toksz += f->len;
	}
	f->tok = s + f->len;	/* EOL */
	return nil;
err:
	f->tok = f->end;
	f->toksz = 0;
	return nil;
}

void
splitfs(File *f, char sep)
{
	f->sep = sep;
}

char *
readline(File *f)
{
	/* previous line unterminated, longer than size of buffer */
	while(f->trunc)
		readfrag(f);
	f->foff = sysftell(f);
	f->nr++;
	f->nf = 0;
	f->toksz = 0;
	f->tok = readfrag(f);
	return f->tok;
}

File *
openfs(char *path, int mode)
{
	File *f;

	f = emalloc(sizeof *f);
	if(sysopen(f, path, mode) < 0){
		free(f);
		return nil;
	}
	f->sep = '\t';
	f->path = estrdup(path);
	return f;
}

File *
fdopenfs(int fd, int mode)
{
	File *f;

	f = emalloc(sizeof *f);
	if(sysfdopen(f, fd, mode) < 0){
		free(f);
		return nil;
	}
	f->sep = '\t';
	return f;
}
