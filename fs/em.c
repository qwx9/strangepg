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

// FIXME: maybe chains are unnecessary here, it's two extra link ops
static void
llink(Chunk *lleft, Chunk *lright)
{
	lright->lleft->lright = lleft->lright;
	lleft->lright->lleft = lright->lleft;
	lright->lleft = lleft;
	lleft->lright = lright;
}
static void
lunlink(Chunk *lleft, Chunk *lright)
{
	lleft->lleft->lright = lright->lright;
	lright->lright->lleft = lleft->lleft;
	lleft->lleft = lright;
	lright->lright = lleft;
}
static void
poke(Chunk *c)
{
	dprint(Debugextmem, "poke %#p ll %#p lr %#p", c, c->lleft, c->lright);
	lunlink(c, c);
	llink(norris.lleft, c);
}
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
	dprint(Debugextmem, "flushchunk %#p[%llx:%llx] fd %d stream %d off %zd", c, c->off, c->len, em->fd, em->stream, c->off);
	if(!em->stream && seek(em->fd, c->off, 0) < 0)
		return -1;
	dprint(Debugextmem, "flushchunk %llx %d", c->len, em->fd);
	if(write(em->fd, c->buf, c->len) != c->len)
		return -1;
	return 0;
}
void
emflush(EM *em)
{
	Chunk *c, **cp;

	dprint(Debugextmem, "emflush %#p %zd chunks", em, dylen(em->cp));
	for(cp=em->cp; cp<em->cp+dylen(em->cp); cp++)
		if((c = *cp) != nil)
			flushchunk(em, c);
}

static void
freechunk(EM *em, Chunk **cp)
{
	Chunk *c;

	if(cp == nil || (c = *cp) == nil)
		return;
	dprint(Debugextmem, "freechunk %#p[%llx:%llx]", c, c->off, c->len);
	if(em->fd >= 0 && flushchunk(em, c) < 0)
		warn("freechunk: %s\n", error());
	lunlink(c, c);
	llink(handouts.lleft, c);
	memfree++;
	c->len = 0;
	c->off = -1;
}
static void
freechain(EM *em, Chunk **l, Chunk **le)
{
	int c;
	Chunk **p;

	if(l == nil)
		return;
	assert(le >= em->cp && le <= em->cp + dylen(em->cp));
	dprint(Debugextmem, "freechain %#p … %#p %lld", l, le, dylen(em->cp));
	for(c=0, p=l; p<le; p++, c++)
		freechunk(em, p);
	while(c-- > 0)
		dydelete(em->cp, 0);
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
	c->off = -1;
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
		/* poked by caller */
	}else if(memreallyfree > 0)
		c = allocchunk();
	else
		c = evictchunk();
	return c;
}

ssize
emshrink(EM *em, ssize sz)
{
	ssize ci;

	ci = CADDR(sz);
	dprint(Debugextmem, "emshrink %s[%llx] → %llx", em->path, dylen(em->cp), ci);
	if(dylen(em->cp) <= ci){
		dprint(Debugextmem, "emshrink: nothing to resize");
		return 0;
	}
	freechain(em, em->cp + ci + 1, em->cp + dylen(em->cp));
	dyhdr(em->cp)->len = ci + 1;
	/* not touching .off or .len */
	return ci;
}

int
embraceextendextinguish(EM *em, EM *from)
{
	vlong off;
	ssize ni, ci;
	Chunk *c, **cp;

	dprint(Debugextmem, "embraceextendextinguish %s[%llx/%llx] ← %s[%llx/%llx]",
		em->path, dylen(em->cp), dyhdr(em->cp)->sz, from->path,
		dylen(from->cp), dyhdr(from->cp)->sz);
	assert(em->stream);	/* only makes sense for a pipe, all is scrambled */
	ci = dylen(em->cp);
	ni = dylen(from->cp);
	dygrow(em->cp, ni+ci+1);
	memcpy(em->cp + ci, from->cp, ni * sizeof *cp);
	dyhdr(em->cp)->len += ni;	/* ouch */
	dprint(Debugextmem, "embraceextendextinguish %s[%llx/%llx] ← %s[%llx/%llx]",
		em->path, dylen(em->cp), dyhdr(em->cp)->sz, from->path,
		dylen(from->cp), dyhdr(from->cp)->sz);
	off = ci * Chunksz;
	for(cp=em->cp+ci; cp<em->cp+dylen(em->cp); cp++, off+=Chunksz){
		warn("[");
		if((c = *cp) == nil){
			warn("]");
			continue;
		}
		c->off = off | c->off & Chunksz-1;
		warn("%llx]", c->off);
		poke(c);
	}
	memset(&from->cp, 0, ni * sizeof *em->cp);	/* oof */
	return 0;
}

int
emsteal(EM *em, EM *victim)
{
	ssize ci;
	intptr v;
	Chunk **cp, **ce;

	dprint(Debugextmem, "emsteal %s[%llx/%llx] ← %s[%llx/%llx]",
		em->path, dylen(em->cp), dyhdr(em->cp)->sz, victim->path,
		dylen(victim->cp), dyhdr(victim->cp)->sz);
	/* ouch */
	if(dylen(victim->cp) == 0)
		return 0;
	freechain(em, em->cp, em->cp + dylen(em->cp));
	dyfree(em->cp);
	v = sizeof(*victim->cp) * dyhdr(victim->cp)->sz;
	memcpy((uchar *)&em->cp,
		(uchar *)&victim->cp, v);	/* oh man */
	memset((uchar *)&victim->cp, 0, v);	/* oof */
	ci = dylen(em->cp);
	for(cp=em->cp, ce=cp+ci; cp<ce; cp++)
		if(*cp != nil)
			poke(*cp);
	victim->cp = nil;
	return 0;
}

static ssize
readchunk(EM *em, Chunk *c)
{
	ssize n, m, off;
	uchar *p;

	dprint(Debugextmem, "readchunk: %s[off] %llx", em->path, COFF(c->off));
	if(em->fd < 0 || em->stream)
		return 0;
	off = COFF(c->off);
	if(off < c->off){
		dprint(Debugextmem, "readchunk: read before");
		if(seek(em->fd, off, 0) < 0)
			return -1;
		for(n=c->off-off, p=c->buf+off, m=0; n>0; p+=m, n-=m)
			if((m = read(em->fd, p, n)) <= 0)
				break;
		if(m < 0)
			sysfatal("read: %s", error());
		if(m == 0)
			return -1;
		c->len += c->off - off;
		c->off = off;
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
		c->len = Chunksz;
	}
	return c->len;
}

static Chunk *
getchunk(EM *em, vlong off, ssize want)
{
	ssize ci;
	Chunk *c, **cp;

	dprint(Debugextmem, "getchunk: %s[off] %llx", em->path, off);
	assert(off >= 0 && off != EMbupkis);
	ci = CADDR(off);
	if(em->cp == nil || ci >= dylen(em->cp))
		dygrow(em->cp, ci);
	cp = em->cp + ci;
	if((c = *cp) == nil){
		c = newchunk();
		c->off = COFF(off);
		dyinsert(em->cp, ci, c);
	}
	dprint(Debugextmem, "getchunk: read? %llx:%llx want %llx:%llx",
		c->off, c->len, off, want);
	if((want > 0 || em->mode & EMclown) && c->off + c->len < off + want)
		if(readchunk(em, c) < 0)
			return nil;
	poke(c);
	return c;
}

ssize
emwrite(EM *em, vlong off, uchar *buf, ssize n)
{
	ssize m, Δ, ci;
	uchar *p, *s;
	Chunk *c;

	dprint(Debugextmem, "emwrite %s[%llx] %llx to %#p", em->path, off, n, buf);
	if(em->fd < 0){
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
		if(c->off > off)
			c->off = off;
		if(c->off + c->len < off + m)
			c->len = m - c->off + off;
		dprint(Debugextmem, "emwrite %s[%llx] wrote %llx in %#p", em->path, off, m, c);
	}
	em->off = off;
	return s - buf;
}

u64int
empget64(EM *em, vlong off)
{
	ssize ci;
	uchar *p;
	u64int v;
	Chunk *c;

	assert(off >= 0 && off != EMbupkis);
	off *= 8;
	ci = CADDR(off);
	if(dylen(em->cp) <= ci || (c = em->cp[ci]) == nil
	|| off < c->off || off + 8 > c->off + c->len)
		if((c = getchunk(em, off, 8)) == nil)
			return EMbupkis;
	p = c->buf + off - c->off;
	v = GBIT64(p);
	dprint(Debugextmem, "empget64 %s[%llx] → %llx", em->path, off, v);
	if(c->off > off)
		c->off = off;
	if(c->off + c->len < off + 8)
		c->len = 8 - c->off + off;
	em->off = off + 8;
	return v;
}

int
empput64(EM *em, vlong off, u64int v)
{
	ssize ci;
	uchar *p;
	Chunk *c;

	assert(off >= 0 && off != EMbupkis);
	off *= 8;
	ci = CADDR(off);
	if(dylen(em->cp) <= ci || (c = em->cp[ci]) == nil
	|| off < c->off || off + 8 > c->off + c->len)
		if((c = getchunk(em, off, 0)) == nil)
			return -1;
	p = c->buf + off - c->off;
	PBIT64(p, v);
	dprint(Debugextmem, "empput64 %s[%llx] ← %llx", em->path, off, v);
	if(c->off > off)
		c->off = off;
	if(c->off + c->len < off + 8)
		c->len = 8 - c->off + off;
	em->off = off + 8;
	return 0;
}
int
emput64(EM *em, u64int v)
{
	return empput64(em, em->off / 8, v);
}
u64int
emget64(EM *em)
{
	return empget64(em, em->off / 8);
}

/* default: rw temp file from scratch: write without prior read */
EM*
emnew(int notmp)
{
	EM *em;

	em = emalloc(sizeof *em);
	if(!notmp){
		em->tmp = 1;
		em->path = sysmktmp();
		if((em->fd = create(em->path, ORDWR, 0644)) < 0)
			sysfatal("emfdopen: %s", error());
	}else
		em->fd = -1;
	em->l.lleft = em->l.lright = &em->l;
	return em; 
}

EM*
emfdopen(int fd, int notmp)
{
	EM *em;

	dprint(Debugextmem, "emfdopen %d", fd);
	em = emnew(notmp);
	em->fd = fd;
	em->stream = 1;
	return em;
}

/* buffer backed by preexisting file: read before write */
EM*
emopen(char *path)
{
	EM *em;

	dprint(Debugextmem, "emopen %s", path);
	em = emnew(1);
	em->path = estrdup(path);
	em->mode |= EMclown;
	if((em->fd = open(path, ORDWR)) < 0)
		return nil;
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
	if(em->path != nil && em->tmp){
		dprint(Debugextmem, "emclose %s: remove", em->path);
		sysremove(em->path);
	}
	free(em->path);
	free(em);
}
