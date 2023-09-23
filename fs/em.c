#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

enum{
	Poolsz = 4*1024*1024*1024ULL,
	IOsz = 64*1024,
};
static u64int memfree = Poolsz;

/* FIXME: let the abstraction follow dfc-like specs and semantics:
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc.
 */
/* FIXME: maybe on linux stuff like mmap makes more sense; stdio */
/* NOTE: no prefetching before first use */
/* NOTE: always specify offset of given record or field or section:
 * this will evolve into dfc later
 */
/* FIXME: plan9: see also readv(2) */

EM *
emopen(char *path)
{
	EM *em;

	em = emalloc(sizeof *em);
	if((em->fd = emsysopen(path)) < 0)
		sysfatal("emopen: %r");
	em->path = path;
	em->norm = 1;
	em->realsz = em->totsz = emseek(em, 0, 2);
	return em;
}

EM *
emcreate(u64int sz)
{
	EM *em;

	em = emalloc(sizeof *em);
	em->totsz = sz;
	if((em->path = sysmktmp()) == nil){
		free(em);
		return nil;
	}
	if((em->fd = emsyscreate(em->path)) < 0)
		sysfatal("emcreate: %r");
	//warn("emcreate %s %zd\n", em->path, sz);
	return em;
}

// FIXME
EM *
emclone(char *path, vlong off, u64int sz)
{
	int n, m;;
	File *f;
	EM *em;

	f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		sysfatal("emclone: %r");
	seekfs(f, off);
	if((em = emcreate(sz)) == nil)
		sysfatal("emclone: %r");
	em->realsz = sz;
	em->buf = emalloc(IOsz);
	em->bufsz = IOsz;
	memfree -= IOsz;
	//warn("emclone %s size %zd\n", em->path, em->bufsz);
	for(m=sz; m>0; m-=n){
		n = m < em->bufsz ? m : em->bufsz;
		n = readfs(f, em->buf, n);
		if(emsyswrite(em, em->buf, n) < 0)
			sysfatal("emclone: %r");
	}
	warn("emclone %s %zd at %lld\n", em->path, sz, off);
	freefs(f);
	return em;
}

ssize
emflush(EM *em)
{
	uchar *p;

	if(em->nbuf <= 0)
		return 0;
	emseek(em, em->cacheoff, 0);
	p = em->buf + em->cacheoff - em->off;
	/*
	for(m=em->nbuf, p=em->buf; m>0; m-=n, p+=n)
		if((n = emsyswrite(em, p, m)) < 0)
			return -1;
	*/
	if(emsyswrite(em, p, em->nbuf) < 0)
		return -1;
	em->off += em->nbuf;
	em->nbuf = 0;
	em->age = time(nil);
	return 0;
}

// FIXME: idea: giant global read buffers to point into?
//	or list/array of allocated buffers?
ssize
emreap(ssize req)
{
	// FIXME: free space up
	USED(req);
	return 0;
}

int
emgrowfile(EM *em, vlong sz)
{
	uchar u[1] = {0};

	if(em->realsz >= sz)
		return 0;
	emseek(em, sz, 0);
	if(emsyswrite(em, u, sizeof u) < 0)
		return -1;
	em->realsz = sz;
	if(em->totsz < em->realsz)
		em->totsz = em->realsz;
	return 0;
}

ssize
emgrowbuf(EM *em, ssize n)
{
	n += em->off + IOsz;
	if(n > memfree && (n = emreap(n)) <= 0)
		return -1;
	memfree += em->bufsz;
	em->buf = erealloc(em->buf, n, em->bufsz);
	em->bufsz = n;
	n = em->bufsz;
	memfree -= em->bufsz;
	return n;
}

void
emshrink(EM *em, usize n)
{
	emflush(em);
	if(em->bufsz < n)
		return;
	if(em->nbuf > n)
		em->nbuf = n;
	memfree += em->bufsz - n;
	em->buf = erealloc(em->buf, n, em->bufsz);
	em->bufsz = n;
}

uchar *
emfetch(EM *em, vlong off, ssize n)
{
	uchar *p;
	ssize m;

	if(off >= em->cacheoff && off + n <= em->cacheoff + em->nbuf)
		return em->buf + off - em->off;
	else if(off >= em->off && off + n <= em->off + em->bufsz){
		if(off < em->cacheoff){
			emseek(em, off, 0);
			p = em->buf + off - em->off;
			m = em->cacheoff - off;
			if((n = emsysread(em, p, m)) < 0)
				return nil;
			em->cacheoff = off;
			em->nbuf += m;
		}
		if(off + n > em->cacheoff + em->nbuf){
			emseek(em, em->cacheoff + em->nbuf, 0);
			p = em->buf + off - em->off;
			m = off + n - em->cacheoff - em->nbuf;
			if((n = emsysread(em, p, m)) < 0)
				return nil;
			em->nbuf += n;
		}
		return em->buf + off - em->off;
	}
	emflush(em);
	m = off + n - em->off - em->bufsz;
	emgrowbuf(em, em->bufsz + m);
	emgrowfile(em, em->bufsz);
	if(off >= em->off){
		if(off < em->cacheoff){
			emseek(em, off, 0);
			p = em->buf + off - em->off;
			m = em->cacheoff - off;
			if((n = emsysread(em, p, m)) < 0)
				return nil;
			em->cacheoff = off;
			em->nbuf += m;
		}
		if(off + n > em->cacheoff + em->nbuf){
			emseek(em, em->cacheoff + em->nbuf, 0);
			p = em->buf + off - em->off;
			m = off + n - em->cacheoff - em->nbuf;
			if((n = emsysread(em, p, m)) < 0)
				return nil;
			em->nbuf += n;
		}
		em->realsz = em->bufsz;
		return em->buf + off - em->off;
	}
	emseek(em, 0, 0);
	em->off = 0;
	if((n = emsysread(em, em->buf, em->bufsz)) < 0)
		return nil;
	em->cacheoff = em->off;
	em->nbuf = n;
	em->realsz = em->bufsz;
	return em->buf;
}

ssize
empreload(EM *em)
{
	if(em->totsz <= 0)
		return 0;
	if(emfetch(em, 0, em->totsz) == nil){
		warn("empreload: %r\n");
		return -1;
	}
	return em->totsz;
}

ssize
embarf(EM *em, vlong off, uchar *q, ssize n)
{
	uchar *p;

	if(off >= em->off && off + n <= em->off + em->bufsz){
		p = em->buf + off - em->totsz;
		memmove(p, q, n);
		return n;
	}
	// FIXME: ignoring any buffering if it hasn't been done already
	emseek(em, off, 0);
	if((n = emsyswrite(em, q, n)) < 0)
		return -1;
	if(off + n > em->realsz)
		em->realsz = off + n;
	return n;
}

/* convenience functions */
u64int
emget64(EM *em, vlong off)
{
	uchar *p;
	u64int v;

	if((p = emfetch(em, off, 8)) == nil)
		sysfatal("emget64: %r");
	v = GBIT64(p);
	return v;
}

ssize
emput64(EM *em, vlong off, u64int v)
{
	union { uchar u[sizeof(u64int)]; u64int v; } u;

	u.v = v;
	if(embarf(em, off, u.u, sizeof u.u) < 0)
		sysfatal("emput64: %r");
	return 0;
}

void
emnuke(EM *em)
{
	em->nbuf = 0;
	if(em->bufsz > 0){
		free(em->buf);
		memfree += em->bufsz;
		em->buf = nil;
		em->bufsz = 0;
	}
}

vlong
emseek(EM *em, vlong off, int mode)
{
	warn("emseek %s at %lld\n", em->path, off);
	return emsysseek(em, off, mode);
}

void
emclose(EM *em)
{
	if(em == nil)
		return;
	close(em->fd);
	if(em->path != nil && !em->norm)
		sysremove(em->path);
	free(em->path);
	free(em->buf);
	free(em);
}
