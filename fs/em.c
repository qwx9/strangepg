#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* FIXME: will change this up later, see below */
enum{
	Poolsz = 4*1024*1024*1024ULL,
	Maxsz = Poolsz / 10,
	IOsz = 64*1024,
	Maxchunk = 1024*1024,
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
/* FIXME: aggressively gobble up memory up until Poolsz, but only
 * as much as we can/want to read at a time; if we really want to
 * read in a huge file, let's do it in parallel (later)
 * better yet, read in mb-sized chunks in an LRU: intervals loaded
 * in file; see page(1) */

EM *
emopen(char *path)
{
	EM *em;

	em = emalloc(sizeof *em);
	if((em->fd = emsysopen(path)) < 0)
		sysfatal("emopen: %r");
	em->path = estrdup(path);
	em->artwork = 1;
	em->realsz = em->totsz = emseek(em, 0, 2);
	emseek(em, 0, 0);
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
	return em;
}

/* FIXME
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
	for(m=sz; m>0; m-=n){
		n = m < em->bufsz ? m : em->bufsz;
		n = readfs(f, em->buf, n);
		if(emsyswrite(em, em->buf, n) < 0)
			sysfatal("emclone: %r");
	}
	freefs(f);
	return em;
}
*/

ssize
emflush(EM *em)
{
	uchar *p;

	if(em->nbuf <= 0 || em->artwork)
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
	return 0;
}

static ssize
emreap(ssize req)
{
	// FIXME: free space up
	USED(req);
	return 0;
}

static int
emgrowfile(EM *em)
{
	uchar u[1] = {0};
	ssize m;

	if(em->artwork)
		return 0;
	m = em->off + em->bufsz;
	if(em->realsz >= em->totsz || em->realsz >= m)
		return 0;
	emseek(em, m-1, 0);
	if(emsyswrite(em, u, sizeof u) < 0)
		return -1;
	em->realsz = m;
	return 0;
}

static ssize
emgrowbuf(EM *em)
{
	ssize m;

	if(em->bufsz >= em->totsz)
		return 0;
	if(em->totsz < Maxsz)
		m = em->totsz - em->bufsz;
	else
		m = Maxsz;
	if(m > memfree){
		emreap(m);
		if(m > memfree)
			m = memfree;
	}
	if(m <= 0)
		return 0;
	em->buf = erealloc(em->buf, em->bufsz + m, em->bufsz);
	memfree -= m;
	em->bufsz += m;
	return m;
}

void
emshrink(EM *em, usize n)
{
	if(em->buf == nil)
		return;
	emflush(em);
	if(em->bufsz < n)
		return;
	if(em->nbuf > n)
		em->nbuf = n;
	memfree += em->bufsz - n;
	em->buf = erealloc(em->buf, n, em->bufsz);
	em->bufsz = n;
}

static uchar *
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
			em->nbuf += n;
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
	emflush(em);	// FIXME: make sure to handle this one too later
	emgrowbuf(em);
	emgrowfile(em);
	em->off = off;
	// FIXME: fuck it, we'll rewrite this anyway
	m = em->bufsz;
	if(em->off + em->bufsz > em->totsz){
		em->off -= em->totsz - em->bufsz;
		m -= em->totsz - em->bufsz;
	}
	emseek(em, off, 0);
	p = em->buf + off - em->off;
	if((n = emsysread(em, p, m)) < 0)
		return nil;
	em->cacheoff = off;
	em->nbuf = n;
	return p;
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

static ssize
embarf(EM *em, vlong off, uchar *q, ssize n)
{
	uchar *p;

	/* whatever we write must be in a cached region
	 * to remain in sync */
	if((p = emfetch(em, off, n)) == nil)
		return -1;
	memmove(p, q, n);
	return n;
}

u64int
empget64(EM *em, vlong off)
{
	uchar *p;
	u64int v;

	assert(off >= 0);
	if((p = emfetch(em, off, 8)) == nil)
		sysfatal("emget64: %r");
	em->lastoff = off + 8;
	v = GBIT64(p);
	return v;
}

u64int
emget64(EM *em)
{
	return empget64(em, em->lastoff);
}

ssize
empput64(EM *em, vlong off, u64int v)
{
	union { uchar u[sizeof(u64int)]; u64int v; } u;

	assert(off >= 0);
	u.v = v;
	if(embarf(em, off, u.u, sizeof u.u) < 0)
		sysfatal("emput64: %r");
	em->lastoff = off + 8;
	return 0;
}

ssize
emput64(EM *em, u64int v)
{
	return empput64(em, em->lastoff, v);
}

void
emnuke(EM *em)
{
	emflush(em);
	emshrink(em, 0);
}

vlong
emseek(EM *em, vlong off, int mode)
{
	return emsysseek(em, off, mode);
}

void
emclose(EM *em)
{
	if(em == nil)
		return;
	emnuke(em);
	emsysclose(em);
	if(em->path != nil && !em->artwork)
		sysremove(em->path);
	free(em->path);
	free(em);
}
