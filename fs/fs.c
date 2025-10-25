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

/* FIXME: nuke */
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

	if(buf == nil){
		if(n >= sizeof f->buf)
			panic("readfs: bug: buffer overflow %d > %d", n, sizeof f->buf);
		if(f->cur + n >= sizeof f->buf)
			f->cur = 0;
		buf = f->buf + f->cur;
	}else
		f->cur = 0;
	if((m = sysread(f, buf, n)) < 0){
		if(debug & Debugtheworld)
			panic("readfs: short read %d not %d: %s", m, n, error());
		sysfatal("readfs: short read %d not %d: %s", m, n, error());
	}
	f->end = f->cur + m;
	f->next = -1;
	return m;
}

/* FIXME: unbuffered, use .buf */
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

void
splitfs(File *f, char sep)
{
	f->sep = sep;
}

static inline void
skiptonext(File *f)
{
	f->foff += f->next - f->cur + 1;
	f->cur = f->next + 1;
	f->next = -1;
}

static inline void
skiptoeol(File *f)
{
	f->foff += f->next - f->cur;
	f->cur = f->next;
}

static inline void
skipfrag(File *f)
{
	f->foff += f->end - f->cur;
	f->cur = f->end;
}

static inline void
skipfield(File *f)
{
	int n;

	n = f->toksep - f->cur + 1;
	f->foff += n;
	f->cur += n;
}

int
seekfs(File *f, vlong off)
{
	assert(f->aux != nil);
	DPRINT(Debugfs, "seek to %lld", off);
	if(sysseek(f, off) < 0)
		return -1;
	f->flags = 0;
	f->next = -1;
	f->tok = nil;
	f->toksz = 0;
	f->cur = f->end = 0;
	f->foff = off;
	return 0;
}

static inline int
slurp(File *f, int sz)
{
	int n;
	char *s;

	DPRINTN(Debugfs, "[slurp: ");
	s = f->buf + f->cur;
	if((n = f->end - f->cur) > 0 && f->cur > 0){
		DPRINTN(Debugfs, "moving back %d bytes; ", n);
		memmove(f->buf, s, n);
	}
	f->cur = 0;
	f->end = n;
	s = f->buf + n;
	if((n = sysread(f, s, sz)) < 0){
		DPRINTN(Debugfs, "read error: %s]", error());
		return -1;
	}else if(n == 0){
		f->flags |= FFSeof;
		DPRINTN(Debugfs, "at eof; ");
	}
	DPRINTN(Debugfs, "read %d bytes]", n);
	f->end += n;
	return 0;
}

static inline int
geteol(File *f, char *s, int n)
{
	char *t;

	DPRINTN(Debugfs, "[geteol: ");
	if(n <= 0){
		DPRINTN(Debugfs, "at eol]");
		f->next = f->end;
		return 0;
	}
	if((t = memchr(s, '\n', n)) == nil){
		if(f->flags & FFSeof){
			DPRINTN(Debugfs, "at eof]");
			f->next = f->end;
			return 0;
		}
		DPRINTN(Debugfs, "truncated]");
		f->next = -1;
		return -1;
	}
	f->next = t - s + f->cur;
	DPRINTN(Debugfs, "in %d bytes]", f->next - f->cur);
	return 0;
}

static inline char *
terminate(File *f, char *s, char *e)
{
	*e = 0;
	f->toksz = e - s;
	f->toksep = f->cur + f->toksz;
	f->tok = s;
	DPRINTN(Debugfs, "len=%lld]", f->toksz);
	return s;
}

static inline char *
getsep(File *f)
{
	int n;
	char *s, *t;

	DPRINTN(Debugfs, "[getsep: ");
	s = f->buf + f->cur;
	if(f->next != -1){
		if((n = f->next - f->cur) == 0)
			return terminate(f, s, s);
	}else
		n = f->end - f->cur;
	if((t = memchr(s, f->sep, n)) != nil)
		return terminate(f, s, t);
	if(f->next != -1)
		return terminate(f, s, f->buf + f->next);
	DPRINTN(Debugfs, "no sep]");
	return nil;
}

char *
nextfield(File *f)
{
	int n;
	vlong m;
	char *s;

	if(f->end - f->cur <= 0){
		warn("BUG: nextfield without readline\n");
		return nil;
	}
	if(f->nf++ > 0)
		skipfield(f);
	DPRINTS(Debugfs, "%#p:%d,%d nextfield %d:%d:%d: ",
		f, f->nr, f->nf, f->cur, f->next, f->end);
	if(f->next != -1 && f->cur >= f->next){
		DPRINTN(Debugfs, "eol\n");
		return nil;
	}
	if((s = getsep(f)) != nil){
		DPRINTN(Debugfs, "\n");
		return s;
	}
	for(m=0;;){
		n = f->end - f->cur;
		if(n > Readsz){
			DPRINTN(Debugfs, "; skipping %d bytes ", Readsz);
			m += Readsz;
			f->foff += Readsz;
			f->cur += Readsz;
		}
		if(slurp(f, Readsz) < 0)
			return nil;
		s = f->buf + f->cur;
		n = f->end - f->cur;
		geteol(f, s, n);
		if((s = getsep(f)) != nil)
			break;
	}
	f->toksz += m;
	DPRINTN(Debugfs, " actual len=%lld\n", f->toksz);
	return s;
}

static inline int
startline(File *f)
{
	int n;
	char *s;

	DPRINTN(Debugfs, "[startline: ");
	s = f->buf + f->cur;
	if((n = f->end - f->cur) > 0){
		if(geteol(f, s, n) >= 0)
			return 0;
		if(n > Readsz){
			DPRINTN(Debugfs, "enough buffered]");
			return 0;
		}
	}
	DPRINTN(Debugfs, "not enough buffered; ");
	if(slurp(f, Readsz) < 0)
		return -1;
	if((n = f->end - f->cur) == 0){
		DPRINTN(Debugfs, "eof]");
		return -1;
	}
	s = f->buf + f->cur;
	geteol(f, s, n);
	DPRINTN(Debugfs, "]");
	return 0;
}

int
skipline(File *f)
{
	int n;
	char *s;

	DPRINTS(Debugfs, "%#p:%d skipline %d:%d:%d: ",
		f, f->nr+1, f->cur, f->next, f->end);
	while(f->next == -1){
		if(f->end - f->cur > Readsz){
			DPRINTN(Debugfs, "; skipping %d bytes ", Readsz);
			f->foff += Readsz;
			f->cur += Readsz;
		}
		if(slurp(f, Readsz) < 0)
			return -1;
		n = f->end - f->cur;
		s = f->buf + f->cur;
		if(geteol(f, s, n) < 0)
			skipfrag(f);
	}
	skiptoeol(f);
	DPRINTN(Debugfs, "now at %lld\n", f->foff);
	return 0;
}

static inline int
skiprest(File *f)
{
	int n;
	char *s;

	DPRINTN(Debugfs, "[skiprest");
	while(f->next == -1){
		if((n = f->end - f->cur) > Readsz){
			DPRINTN(Debugfs, "; skipping %d bytes ", Readsz);
			f->foff += Readsz;
			f->cur += Readsz;
		}
		if(slurp(f, Readsz) < 0)
			return -1;
		n = f->end - f->cur;
		s = f->buf + f->cur;
		if(geteol(f, s, n) < 0)
			skipfrag(f);
	}
	skiptonext(f);
	DPRINTN(Debugfs, "]");
	return 0;
}

char *
readline(File *f)
{
	DPRINTS(Debugfs, "%#p:%d readline %d:%d:%d: ",
		f, f->nr+1, f->cur, f->next, f->end);
	if(f->cur < f->end && skiprest(f) < 0)
		return nil;
	if(startline(f) < 0)
		return nil;
	f->nr++;	/* makes sense until we seek backwards */
	f->nf = 0;
	f->tok = nil;
	f->toksz = f->next != -1 ? f->next - f->cur : 0;
	DPRINTN(Debugfs, "\n");
	return f->buf + f->cur;	/* unterminated */
}

int
readlineat(File *f, int m, vlong off)
{
	int n, blk;
	vlong o;

	assert(m <= Readsz);
	DPRINT(Debugfs, "%#p readlineat %d:%d:%d: %d bytes at %lld (Δ %lld)",
		f, f->cur, f->next, f->end, m, off, off - f->foff);
	o = off - f->foff;
	n = f->end - f->cur;
	if(o > 0 && o <= n){
		f->cur += o;
		f->foff += o;
		n -= o;
		if(n < m){
			if(n > 0)
				off += n;
			if(sysseek(f, off) < 0)
				return -1;
		}
		blk = o <= Readsz/2 ? Readsz/2 : m;
	}else{
		if(sysseek(f, off) < 0)
			return -1;
		f->cur = f->end = 0;
		f->foff = off;
		f->flags = 0;
		if(o < 0)
			o = -o;
		blk = o <= Readsz/16 ? Readsz/16 : m;
	}
	n = f->end - f->cur;
	while(n < m){
		if(f->flags & FFSeof){
			m = n;
			break;
		}
		if(slurp(f, blk) < 0)
			return -1;
		n = f->end - f->cur;
	}
	f->next = f->cur + m;
	f->nf = 0;
	f->tok = nil;
	f->toksz = 0;
	return 0;
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
	DPRINT(Debugfs, "openfs %s: %#p", path, f);
	f->path = estrdup(path);
	f->sep = '\t';
	f->next = -1;
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
	DPRINT(Debugfs, "fdopenfs %d: %#p", fd, f);
	f->sep = '\t';
	f->next = -1;
	return f;
}
