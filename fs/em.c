#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* FIXME: let the abstraction follow dfc-like specs and semantics:
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc.
 */

int
emopen(EM *em, char *path)
{
	if((em->fd = emsysopen(path)) < 0)
		sysfatal("emopen: %r");
	em->path = path;
	return 0;
}

EM *
emcreate(usize sz)
{
	char *path;
	EM *em;

	/* FIXME
	if(sz < Maxmem){
	*/
	if(1){
		em = emalloc(sizeof *em);
		em->buf = emalloc(sz);
		em->bufsz = sz;
		em->nbuf = sz;
		em->totsz = sz;
		return em;
	}
	if((path = sysmktmp()) == nil)
		return nil;
	em = emalloc(sizeof *em);
	if(emopen(em, path) < 0)
		sysfatal("emcreate: %r");
	em->buf = emalloc(Maxmem);
	em->bufsz = Maxmem;
	emwrite(em, sz-1, (uchar*)path + strlen(path) - 1, 1);
	return em;
}

ssize
emprefetch(EM *em)
{
	ssize r;

	em->nbuf = 0;
	emseek(em, 0, 0);
	if((r = emsysread(em, em->buf, em->bufsz)) > 0)
		em->nbuf = em->bufsz;
	return r;
}

uchar *
emreadn(EM *em, vlong off, ssize n)
{
	uchar *p;

	/* FIXME
	ssize r;

	assert(p >= em->buf);
	assert(p + n <= em->buf + em->bufsz);
	emseek(em, off, 0);
	if((r = emsysread(em, p, n)) <= 0)
		return nil;
	return p + r;
	*/
	USED(n);
	p = em->buf + off;
	return p + off;
}

ssize
emwrite(EM *em, vlong off, uchar *p, ssize n)
{
	/* FIXME
	ssize r;

	emseek(em, off, 0);
	assert(p >= em->buf);
	assert(p + n - em->buf <= em->totsz);
	if(p + n > em->buf + em->nbuf){
		emreadn(em, p, em->readsz);
		p -= 
	}
	if((r = emsyswrite(em, p, n)) != n)
		return -1;
	return r;
	*/
	memcpy(em->buf+off, p, n);
	return n;
}

u64int
emget64(EM *em, vlong off)
{
	uchar *p;
	union { uchar u[8]; u64int v; } u;

	/* FIXME
	if(em->bufsz <= Maxmem){
	*/
	if(1){
		p = em->buf + off;
		return GBIT64(p);
	}
	emseek(em, off, 0);
	if(emsysread(em, u.u, sizeof(u64int)) <= 0)
		sysfatal("emget64: %r");
	return u.v;
}

ssize
emput64(EM *em, vlong off, u64int v)
{
	union { uchar u[8]; u64int v; } u;

	/* FIXME
	if(em->bufsz <= Maxmem){
	*/
	if(1){
		*(u64int*)(em->buf + off) = v;
		return -1;
	}
	u.v = v;
	emseek(em, off, 0);
	return emsyswrite(em, u.u, sizeof u.v);
}

int
emseek(EM *em, vlong off, int mode)
{
	return emsysseek(em, off, mode);
}

void
emclose(EM *em)
{
	close(em->fd);
	free(em->buf);
	free(em->path);
	free(em);
}
