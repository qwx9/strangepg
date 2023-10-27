#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* - currently just greedily uses up as much memory as is given,
 * but could apply some balancing heuristic to free stuff that
 * hasn't been used in a while
 * - worker(s) with their own memory limit instead of a global one
 * - defining a hierarchy of priorities (multiple lists)
 * - sync: temporarily remove chunks from the global lists, etc. */
/* FIXME: let the abstraction follow dfc-like specs and semantics...?
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc. ← would be external to em, this is generic enough */
/* FIXME: maybe on linux stuff like mmap makes more sense */
/* FIXME: plan9: see also readv(2) */

static Chunk norris = {.lleft = &norris, .lright = &norris},
	handouts = {.lleft = &handouts, .lright = &handouts};

static ssize memfree, memreallyfree = Poolsz / Chunksz;
static uchar iobuf[IOUNIT];

/* significantly impact on performance due to frequency of calls */
#define llink(u,v)	do{ \
	Chunk *a = (u), *b = (v); \
	b->lleft->lright = a->lright; \
	a->lright->lleft = b->lleft; \
	b->lleft = a; \
	a->lright = b; \
	}while(0)
#define lunlink(u,v)	do{ \
	Chunk *a = (u), *b = (v); \
	a->lleft->lright = b->lright; \
	b->lright->lleft = a->lleft; \
	a->lleft = b; \
	b->lright = a; \
	}while(0)
#define poke(c)	do{	\
	lunlink((c), (c)); \
	llink(norris.lleft, (c)); \
	}while(0)

static Chunk *
evictchunk(void)
{
	Chunk *c;

	c = norris.lright;
	assert(c != &norris);
	dprint(Debugextmem, "evict %#p", c);
	return c;
}

static int
flushchunk(EM *em, Chunk *c)
{
	if(em->flags & EMshutit)
		return 0;
	dprint(Debugextmem, "flushchunk %#p[%llx:%llx] fd %d", c, c->off, c->len, em->fd);
	if(em->fd < 0){
		if(em->flags & EMondisk){
			if((em->fd = open(em->path, ORDWR)) < 0)
				sysfatal("flushchunk: %s", error());
		}else{
			if(em->path == nil)
				em->path = sysmktmp();
			if((em->fd = create(em->path, ORDWR, 0644)) < 0)
				sysfatal("emfdopen: %s", error());
			em->flags |= EMondisk;
		}
		dprint(Debugextmem, "flushchunk fd=%d", em->fd);
	}
	if((em->flags & EMpipe) == 0)
		if(seek(em->fd, c->off, 0) < 0)
			return -1;
	if(write(em->fd, c->buf, c->len) != c->len)
		return -1;
	return 0;
}

static void
freechunk(EM *em, Chunk *c)
{
	if(c == nil)
		return;
	dprint(Debugextmem, "freechunk %#p[%llx:%llx]", c, c->off, c->len);
	if(em->fd >= 0 && flushchunk(em, c) < 0)
		warn("freechunk: %s\n", error());
	lunlink(c, c);
	llink(&handouts, c);
	if(c->len > 0)
		memset(c->buf, 0, c->len);
	memfree++;
	c->len = -1;
	c->off = -1;
}
static void
freechain(EM *em, Chunk **l, Chunk **le)
{
	int c;
	Chunk **p;

	if(l == nil)
		return;
	dprint(Debugextmem, "freechain %#p … %#p %lld", l, le, dylen(em->cp));
	assert(le >= em->cp && le <= em->cp + dylen(em->cp));
	for(c=0, p=l; p<le; p++, c++){
		freechunk(em, *p);
		*p = nil;
	}
}

/* FIXME: doesn't *quite* fit the api */
void
emflushtofs(EM *em, File *f)
{
	Chunk *c, **cp;

	dprint(Debugextmem, "emflushtofs %#p cp %#p %zd chunks", em, em->cp, dylen(em->cp));
	em->flags |= EMshutit;	// FIXME: dangerous
	for(cp=em->cp; cp<em->cp+dylen(em->cp); cp++)
		if((c = *cp) != nil){
			dprint(Debugextmem, "emflushtofs %#p", c);
			assert(c->len >= 0 && c->len <= Chunksz);
			assert(c->off >= 0);
			writefs(f, c->buf, c->len);
			assert(CADDR(c->off) == cp-em->cp);
			freechunk(em, c);
			*cp = nil;
		}
}

void
printchain(Chunk **cp)
{
	Chunk *c, **ep;

	if((debug & Debugextmem) == 0)
		return;
	dprint(Debugextmem, "chain %#p: ", cp);
	for(ep=cp+dylen(cp),c=*cp; cp<ep; c=*++cp)
		if(c == nil)
			warn("[]");
		else
			warn("[%#p^%llx:%llx]", c, c->off, c->len);
	warn("\n");
}

static Chunk *
allocchunk(void)
{
	Chunk *c;

	c = emalloc(sizeof *c);
	c->buf = emalloc(Chunksz);
	c->lright = c->lleft = c;
	c->off = c->len = -1;
	memreallyfree--;
	dprint(Debugextmem, "allocchunk %#p", c);
	return c;
}

static Chunk *
newchunk(void)
{
	Chunk *c;

	dprint(Debugextmem, "newchunk: free %llx reallyfree %llx",
		memfree, memreallyfree);
	if(memfree > 0){
		c = handouts.lright;
		assert(c != &handouts);
		memfree--;
	}else if(memreallyfree > 0)
		c = allocchunk();
	else
		c = evictchunk();
	poke(c);
	return c;
}

static ssize
readchunk(EM *em, Chunk *c)
{
	ssize n, m;
	uchar *p;

	dprint(Debugextmem, "readchunk: %s[off] %llx", em->path, c->off);
	assert(c->off >= 0 && c->len >= 0 && CMOD(c->off) == 0);
	if(em->fd < 0){
		if((em->flags & (EMpipe|EMshutit)) != 0)
			return 0;
		if(em->path == nil)
			em->path = sysmktmp();
		if((em->fd = open(em->path, ORDWR)) < 0)
			return -1;
		em->flags |= EMondisk;
	}
	if(c->len < Chunksz){
		dprint(Debugextmem, "readchunk: read after");
		if(seek(em->fd, c->off + c->len, 0) < 0)
			return -1;
		for(n=Chunksz-c->len, p=c->buf+c->len, m=0; n>0; p+=m, n-=m)
			if((m = read(em->fd, p, n)) <= 0)
				break;
		if(m < 0)
			sysfatal("read: %s", error());
		if(m == 0)
			return -1;
		c->len = p - c->buf;
	}
	return c->len;
}

static Chunk *
getchunk(EM *em, vlong off, ssize want)
{
	ssize ci;
	Chunk *c, **cp;

	dprint(Debugextmem, "getchunk: %s[%llx]", em->path, off);
	assert(off >= 0 && off != EMbupkis);
	ci = CADDR(off);
	if(em->cp == nil || ci >= dylen(em->cp))
		dygrow(em->cp, ci+1);
	cp = em->cp + ci;
	if((c = *cp) == nil){
		c = newchunk();
		c->off = COFF(off);
		c->len = 0;
		dyinsert(em->cp, ci, c);
	}else
		poke(c);
	assert(c->len >= 0);
	assert(c->off >= 0);
	dprint(Debugextmem, "getchunk: read? %llx:%llx want %llx:%llx",
		c->off, c->len, off, want);
	if((want > 0 || em->flags & EMclown) && c->off + c->len < off + want)
		if(readchunk(em, c) < 0)
			return nil;
	return c;
}

ssize
emwrite(EM *em, vlong off, uchar *buf, ssize n)
{
	ssize m, Δ, ci;
	uchar *p, *s;
	Chunk *c;

	dprint(Debugextmem, "emwrite %s[%llx] %llx to %#p", em->path, off, n, buf);
	if(em->fd < 0 && (em->flags & EMshutit) != 0){
		werrstr("emwrite: no file");
		return -1;
	}
	for(s=buf, ci=CADDR(off); n>0; n-=m, off+=m, s+=m, ci++){
		if(ci >= dylen(em->cp) || (c = em->cp[ci]) == nil)
			if((c = getchunk(em, off, 0)) == nil)
				sysfatal("getchunk: %s", error());
		Δ = off - c->off;
		p = c->buf + Δ;
		m = Chunksz - Δ;
		if(n < m)
			m = n;
		memcpy(p, s, m);
		if(c->off + c->len < off + m)
			c->len = off + m - c->off;
		assert(c->len <= Chunksz);
		dprint(Debugextmem, "emwrite %s[%llx] wrote %llx in %#p", em->path, off, m, c);
	}
	return s - buf;
}

void
emw64(EM *em, ssize off, usize v)
{
	ssize ci;
	uchar *p;
	Chunk *c;

	assert(em != nil);
	off *= 8;
	ci = CADDR(off);
	if(dylen(em->cp) <= ci || (c = em->cp[ci]) == nil
	|| off + 8 > c->off + c->len)
		if((c = getchunk(em, off, 0)) == nil)
			return;
	// FIXME: replace asserts with tests (with asserts)
	assert(off >= c->off);
	assert((off-c->off & 7) == 0);
	p = c->buf + off - c->off;
	if(c->off + c->len < off + 8)
		c->len = off + 8 - c->off;
	assert(c->len <= Chunksz);
	PBIT64(p, (em->flags & EMclone) == 0 ? v | EMexist : v);
}
usize
emr64(EM *em, ssize off)
{
	usize v;
	ssize ci;
	uchar *p;
	Chunk *c;

	assert(em != nil);
	off *= 8;
	ci = CADDR(off);
	if(dylen(em->cp) <= ci || (c = em->cp[ci]) == nil
	|| off + 8 > c->off + c->len)
		if((c = getchunk(em, off, 0)) == nil)
			return EMbupkis;
	assert(off >= c->off);
	assert((off-c->off & 7) == 0);
	if(off + 8 > c->off + c->len)
		return EMbupkis;
	p = c->buf + off - c->off;
	v = GBIT64(p);
	if((em->flags & EMclone) != 0)
		return v;
	return v == 0 ? EMbupkis : v & ~EMexist;
}

/* default: rw temp file from scratch: write without prior read */
EM*
emnew(int flags)
{
	EM *em;

	em = emalloc(sizeof *em);
	em->flags = flags;
	em->fd = -1;
	em->l.lleft = em->l.lright = &em->l;
	return em; 
}

EM*
emfdopen(int fd, int flags)
{
	EM *em;

	dprint(Debugextmem, "emfdopen %d", fd);
	em = emnew(flags | EMpipe);
	em->fd = fd;
	return em;
}

/* buffer backed by preexisting file: read before write */
EM*
emopen(char *path)
{
	EM *em;

	dprint(Debugextmem, "emopen %s", path);
	em = emnew(EMshutit|EMclown);
	em->path = estrdup(path);
	em->flags |= EMondisk;
	if((em->fd = open(path, ORDWR)) < 0){
		emclose(em);
		return nil;
	}
	return em;
}

EM*
emclone(char *path)
{
	int fd;
	ssize n, off;
	EM *em;

	dprint(Debugextmem, "emclone %s", path);
	if((em = emnew(0)) == nil)
		return nil;
	if((fd = open(path, OREAD)) < 0)
		sysfatal("emclone: %s", error());
	for(off=0;; off+=n){
		if((n = read(fd, iobuf, sizeof iobuf)) <= 0)
			break;
		emwrite(em, off, iobuf, n);
	}
	close(fd);
	if(n < 0)
		sysfatal("emclone %s: %s", path, error());
	dprint(Debugextmem, "emclone %s: read %llx bytes", path, off);
	em->flags |= EMclone;
	return em;
}

void
emclose(EM *em)
{
	if(em == nil)
		return;
	freechain(em, em->cp, em->cp + dylen(em->cp));
	if(em->cp != nil)
		dyfree(em->cp);
	if(em->fd >= 0)
		close(em->fd);
	if(em->path != nil && (em->flags & (EMshutit|EMondisk)) == EMondisk){
		dprint(Debugextmem, "emclose %s: remove", em->path);
		sysremove(em->path);
	}
	free(em->path);
	free(em);
}
