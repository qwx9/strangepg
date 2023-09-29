#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

/* - currently just greedily uses up as much memory as is given,
 * but could apply some balancing heuristic to free stuff that
 * hasn't been used in a while
 * - workers with their own memory limit instead of a global one
 * - defining a hierarchy of priorities (multiple lists) would be
 * - sync: temporarily remove chunks from the global lists, etc. */

/* FIXME: let the abstraction follow dfc-like specs and semantics:
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc. */
/* FIXME: maybe on linux stuff like mmap makes more sense; stdio */
/* FIXME: plan9: see also readv(2) */
/* FIXME: try a functional language approach? pass function ptr?
 * we iterate the same way but do different things, like 10 times
 * AND depending on which list (lright or right) */

/* FIXME: tune */
enum{
	Poolsz = 1ULL<<32,
	IOsz = 64*1024,
	Maxchunk = 1*1024*1024,
	Maxgreed = 1ULL<<26,
};
static Chunk norris = {.lleft = &norris, .lright = &norris},
	handouts = {.lleft = &handouts, .lright = &handouts};
static u64int memfree, memreallyfree = Poolsz;

/* oh well */
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
clink(Chunk *left, Chunk *right)
{
	right->left->right = left->right;
	left->right->left = right->left;
	right->left = left;
	left->right = right;
}
static void
cunlink(Chunk *left, Chunk *right)
{
	left->left->right = right->right;
	right->right->left = left->left;
	left->left = right;
	right->right = left;
}

static void
poke(Chunk *c)
{
	lunlink(c, c);
	llink(norris.lleft, c);
}
static void
pokeeach(Chunk *c, Chunk *l)
{
	Chunk *r;

	for(; c!=l; c=r){
		r = c->right;
		poke(c);
	}
}

static void
nukechunk(Chunk *c)
{
	if(c == nil)
		return;
	cunlink(c->left, c);
	lunlink(c->lleft, c);
	free(c->buf);
	memreallyfree += c->bufsz;
	free(c);
}

static int
flushchunk(Chunk *c)
{
	ssize n;

	if(seek(c->fd, c->start, 0) < 0)
		return -1;
	n = c->end - c->start;
	if(write(c->fd, c->buf, n) != n)
		return -1;
	return 0;
}

static void
freechunk(Chunk *c)
{
	if(c == nil)
		return;
	c->start = c->end = -1;
	if(c->fd >= 0 && flushchunk(c) < 0)
		warn("flush: %s\n", error());
	c->fd = -1;
	cunlink(c->left, c);
	lunlink(c->lleft, c);
	llink(handouts.lleft, c);
}

static void
freechain(Chunk *l, Chunk *le)
{
	Chunk *c, *r;

	if(l == nil)
		return;
	for(c=l->right; c!=le; c=r){
		r = c->right;
		freechunk(c);
		memfree += c->bufsz;
	}
}

static Chunk *
chunkglob(Chunk *l, ssize n)
{
	Chunk *c, *r;

	for(c=l->lright; c!=l; c=r){
		r = c->lright;
		n -= c->bufsz;
		if(n <= 0)
			break;
	}
	return c;
}

static Chunk *
chunkseek(Chunk *l, ssize n)
{
	Chunk *c, *r;

	for(c=l->right; c!=l; c=r){
		r = c->right;
		if(n < c->end)
			break;
	}
	return c;
}

static Chunk *
newchunk(ssize n)
{
	Chunk *c;

	assert(n > 0);
	if(n > Maxchunk)
		n = Maxchunk;
	if(n > memreallyfree)
		n = memreallyfree;
	if(n == 0)
		return nil;
	if(n < IOsz)
		n = IOsz;	/* avoid allocating tiny blocks */
	c = emalloc(sizeof *c);
	c->buf = emalloc(n);
	c->bufsz = n;
	c->fd = -1;
	memreallyfree -= n;
	return c;
}

int
emshrink(EM *em, ssize sz)
{
	Chunk *c;

	if((c = em->c.right) == &em->c){
		werrstr("empty buffer");
		return -1;
	}
	c = chunkseek(c, sz);
	assert(c->end <= sz);
	freechain(c->right, em->c.left);
	return 0;
}

static ssize
fsize(int fd)
{
	ssize sz, off;

	if(fd < 0)
		return -1;
	off = seek(fd, 0, 1);
	sz = seek(fd, 0, 2);
	seek(fd, off, 0);
	return sz;
}

int
emappend(EM *em, EM *from)
{
	Chunk *c;

	c = from->c.right;
	cunlink(c->left, from->c.right->left);
	clink(em->c.left, c);
	pokeeach(c, &em->c);
	from->off = 0;
	from->cp = nil;
	return 0;
}

int
emflip(EM *em, EM *victim)
{
	freechain(&em->c, &em->c);
	memcpy(&em->c, &victim->c, sizeof em->c);
	pokeeach(em->c.right, &em->c);
	victim->c.right = victim->c.left = &victim->c;
	em->off = victim->off = 0;
	victim->cp = nil;
	return 0;
}

void
emflush(EM *em)
{
	Chunk *c;

	for(c=em->c.right; c!=&em->c; c=c->right)
		flushchunk(c);
}

vlong
emseek(EM *em, vlong off, int mode)
{
	Chunk *c;

	if((off = seek(em->fd, off, mode)) < 0)
		return -1;
	/* just look up from start, chances are we're seeking towards there */
	if((c = chunkseek(em->c.right, off)) != em->c.right)
		poke(c);
	em->cp = c;
	em->off = off;
	return off;
}

static ssize
supply(ssize n)
{
	Chunk *c;

	while(n > 0){
		if((c = newchunk(n)) == nil)
			break;
		llink(handouts.lleft, c);
		memfree += c->bufsz;
		n -= c->bufsz;
	}
	return n;
}

static void
shave(void)
{
	ssize n;
	Chunk *c;

	for(c=norris.lright; c!=&norris; c=c->lright)
		if((n = c->end - c->start) < c->bufsz){
			c->buf = erealloc(c->buf, n, c->bufsz);
			memreallyfree += c->bufsz - n;
			c->bufsz = n;
		}
}

static ssize
feedmem(ssize n)
{
	ssize m;
	Chunk *c;

	if(n > Maxgreed)
		n = Maxgreed;
	if(n < memfree)
		return n;
	m = n - memfree;
	m -= supply(m);
	if(m < memfree)
		return n;
	shave();
	m -= supply(m);
	if(m < memfree)
		return n;
	if((c = chunkglob(&norris, m)) != &norris){
		freechain(c, c);
		freechunk(c);
	}
	if(m < memfree)
		return n;
	assert(norris.lright == &norris);
	assert(memfree <= 0);
	assert(memreallyfree <= 0);
	return n - m;	/* dommage, mange ta main */
}

static Chunk*
allocchunk(ssize *n)
{
	ssize m;
	Chunk *c, *cl;

	m = feedmem(*n);
	*n = m;
	for(cl=nil, c=handouts.lright; c!=&handouts; c=c->lright){
		poke(c);
		memfree -= c->bufsz;
		m -= c->bufsz;
		if(cl == nil)
			cl = c;
		else
			clink(cl->left, c);
		if(m <= 0)
			break;
	}
	return c;
}

static ssize
getspan(EM *em, vlong off, ssize n)
{
	ssize m, Δ;
	Chunk *c, *a;

	if((c = chunkseek(em->c.right, off)) == &em->c){
		c = allocchunk(&n);
		c->start = off;
		c->end = off + n;
		clink(&em->c, c);
		em->cp = c;
		return n;
	}
	if((m = n) > Maxchunk)
		n = Maxchunk;
	em->cp = c;
	poke(c);
	for(; c!=&em->c && n>0; c=c->right){
		if(off >= c->start){
			Δ = c->end - off;
			off += Δ;
			n -= Δ;
			poke(c);
			continue;
		}
		Δ = n < c->start - off ? n : c->start - off;
		a = allocchunk(&Δ);
		a->start = off;
		a->end = off + Δ;
		clink(c, a);
		off += Δ;
		n -= Δ;
	}
	assert(n == 0);
	return m;
}

static Chunk *
fetch(EM *em, vlong off, ssize *n)
{
	ssize m, k;
	uchar *p, *s;
	Chunk *c;

	/* FIXME: prefetch logic here */
	m = *n;
	if(m < Maxchunk)
		m = Maxchunk;
	if((k = getspan(em, off, m)) < 0)
		sysfatal("fetch %s[%lld..+%lld]: [%lld] %s", em->path, off, off+m, k, error());
	m = k;
	if(seek(em->fd, off, 0) < 0)
		sysfatal("fetch %s[%lld..+%lld]: [%lld] %s", em->path, off+m, k, m, error());
	em->off = off;
	c = em->cp;
	s = c->buf + off - c->start;
	for(p=s; c!=&em->c; c=c->right, m-=k, p+=k){
		k = c->bufsz < m ? c->bufsz : m;
		if(read(em->fd, p, k) != k)
			sysfatal("fetch %s[%lld..+%lld]: [%lld] %s", em->path, off+m, k, m, error());
		em->off += k;
	}
	*n = m;
	return em->cp;
}

/* just keep hitting the piñata until as much candy as asked for falls */
static uchar *
readchunk(EM *em, vlong off, ssize *want)
{
	ssize n;
	uchar *p;
	Chunk *c;

	n = *want;
	assert(off >= 0);
	if((c = em->cp) != nil && off >= c->start && off+n < c->end
	|| (c = chunkseek(em->c.right, off)) != &em->c && off >= c->start
	|| (c = fetch(em, off, &n)) != nil && n > 0){
		c->fd = em->tmpfd;
		if(c->end - off < n)
			n = c->end - off;
		*want = n;
		p = c->buf + off - c->start;
		em->cp = c->right;
		em->off = c->start;
		return p;
	}else
		return nil;
}

/* the indirection, anakin, look at it all */
int
emread(EM *em, vlong off, uchar *u, ssize n)
{
	ssize m;
	uchar *p, *d, *e;

	for(d=u, e=d+n; d<e; d+=m, off+=m, n-=m){
		m = n;
		if((p = readchunk(em, off, &m)) == nil)
			return -1;
		if(u != nil)			/* hack */
			memcpy(d, p, m);
	}
	return 0;
}

u64int
empget64(EM *em, vlong i)
{
	uchar u[8];

	if(emread(em, i*8, u, sizeof u) < 0)
		sysfatal("empget64 %s %lld: %s", em->path, i, error());
	return GBIT64(u);
}

u64int
emget64(EM *em)
{
	uchar u[8];

	emread(em, em->off, u, sizeof u);
	return GBIT64(u);
}

int
emwrite(EM *em, vlong off, uchar *buf, ssize n)
{
	ssize m;
	uchar *p;
	Chunk *c;

	assert(off >= 0);
	/* push through any allocation failures */
	while(n > 0){
		m = n;
		if((c = em->cp) != nil && off >= c->start && off+m < c->end
		|| (c = chunkseek(em->c.right, off)) != &em->c && off >= c->start
		|| (m = getspan(em, off, m)) >= 0
		  && (c = chunkseek(em->c.right, off)) != &em->c
		  && off >= c->start){
			c->fd = em->tmpfd;
			if(c->end - off < m)
				m = c->end - off;
			p = c->buf + off - c->start;
			memcpy(p, buf, m);
			em->cp = c->right;
			off += m;
			em->off = off;
			n -= m;
		}else
			sysfatal("emwrite %s[%lld..+%lld]: [%lld] %s",
				em->path, off, off+m, m, error());
	}
	return 0;
}

int
empput64(EM *em, vlong i, u64int v)
{
	uchar u[8];

	PBIT64(u, v);
	if(emwrite(em, i*8, u, sizeof u) < 0)
		sysfatal("empput64 %s %lld: %s", em->path, i*8, error());
	return 0;
}

int
emput64(EM *em, u64int v)
{
	uchar u[8];

	PBIT64(u, v);
	if(emwrite(em, em->off, u, sizeof u) < 0)
		sysfatal("empput64 %s %lld: %s", em->path, em->off, error());
	return 0;
}

EM*
emnew(void)
{
	EM *em;

	em = emalloc(sizeof *em);
	em->tmpfd = em->fd = -1;
	em->c.left = em->c.right = &em->c;
	return em; 
}

EM*
emfdopen(int fd)
{
	EM *em;

	em = emnew();
	em->tmpfd = em->fd = fd;
	return em;
}

EM*
emopen(char *path)
{
	EM *em;

	em = emnew();
	if((em->fd = create(path, ORDWR, 0644)) < 0)
		return nil;
	em->tmpfd = em->fd;
	em->path = estrdup(path);
	return em;
}

EM*
emclone(char *path)
{
	ssize n, m, off;
	char *tmp;
	EM *em;

	if((em = emopen(path)) == nil)
		return nil;
	tmp = sysmktmp();
	if((em->tmpfd = open(tmp, ORDWR)) < 0)	/* hack */
		sysfatal("emclone: %s", error());
	n = fsize(em->fd);
	for(off=0; n>0; n-=m, off+=m){
		m = n;
		emread(em, off, nil, m);
	}
	emflush(em);
	close(em->fd);
	free(em->path);
	em->fd = em->tmpfd;
	em->path = tmp;
	em->tmpfd = -1;
	return em;
}

void
emclose(EM *em)
{
	if(em == nil)
		return;
	freechain(&em->c, &em->c);
	if(em->fd >= 0)
		close(em->fd);
	assert(em->tmpfd < 0);
	if(em->path != nil)
		sysremove(em->path);
	free(em->path);
	free(em);
}
