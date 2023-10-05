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

enum{
	IOsz = 64*1024,
};
static Chunk norris = {.lleft = &norris, .lright = &norris, .left = &norris, .right = &norris},
	handouts = {.lleft = &handouts, .lright = &handouts, .left = &handouts, .right = &handouts};
static u64int memfree, memreallyfree = Poolsz;

static int
mktmp(EM *em)
{
	int fd;
	char *path;

	path = sysmktmp();
	if((fd = create(path, ORDWR, 0644)) < 0)
		sysfatal("mktmp: %s", error());
	if(em != nil)
		em->path = path;
	else
		free(path);
	return fd;
}

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
	dprint(Debugextmem, "poke %#p ll %#p lr %#p", c, c->lleft, c->lright);
	lunlink(c, c);
	llink(norris.lleft, c);
}
static void
pokeeach(Chunk *l, Chunk *le){
	Chunk *c, *r;

	if(l == nil)
		return;
	dprint(Debugextmem, "pokeeach: l %#p le %#p", l, le);
	for(c=l->right; c!=le; c=r){
		dprint(Debugextmem, "→ pokeeach %#p cl %#p cr %#p",
			c, c->left, c->right);
		r = c->right;
		poke(c);
	}
}

static void
nukechunk(Chunk *c)
{
	dprint(Debugextmem, "nukechunk %#p", c);
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

	dprint(Debugextmem, "flushchunk %#p[%llx-%llx]: → %d",
		c, c->start, c->end, c->fd);
	if(seek(c->fd, c->start, 0) < 0)	// FIXME: emseek?
		return -1;
	n = c->end - c->start;
	if(write(c->fd, c->buf, n) != n)
		return -1;
	return 0;
}

static void
freechunk(Chunk *c)
{
	dprint(Debugextmem, "freechunk %#p", c);
	if(c == nil || c->right == c->left)
		return;
	if(c->fd >= 0 && flushchunk(c) < 0)
		warn("flush: %s\n", error());
	c->fd = -1;
	cunlink(c, c);
	lunlink(c, c);
	llink(handouts.lleft, c);
	c->start = c->end = -1;
}

static void
freechain(Chunk *l, Chunk *le)
{
	Chunk *c, *r;

	
	dprint(Debugextmem, "freechain %#p le %#p: ", l, le);
	if(l == nil || l->right == l->left)
		return;
	for(c=l->right; c!=le; c=r){
		r = c->right;
		freechunk(c);
		memfree += c->bufsz;
	}
}

void
printchain(Chunk *r)
{
	Chunk *c, *l;

	dprint(Debugextmem, "chain %#p: ", r);
	for(c=r, l=nil; l!=r; c=c->right, l=c)
		dprint(Debugextmem, " %#p[%llx-%llx]", c, c->start, c->end);
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

/* does not check start to return nearest neighbor */
static Chunk *
chunkseek(Chunk *l, ssize n)
{
	Chunk *c, *r;

	dprint(Debugextmem, "chunkseek %#p %llx", l, n);
	for(c=l->right; c!=l; c=r){
		r = c->right;
		if(n < c->end){
			dprint(Debugextmem, "chunkseek found %#p %llx..%llx",
				c, c->start, c->end);
			break;
		}
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
	dprint(Debugextmem, "newchunk %#p size %llx", c, c->bufsz);
	c->fd = -1;
	c->right = c->left = c->lright = c->lleft = c;
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
	dprint(Debugcoarse, "emshrink %s from %llx to %llx c %#p[%llx..%llx]",
		em->path, c->end - c->start, sz, c, c->start, c->end);
	assert(c->end - c->start <= c->bufsz);
	freechain(c->right, em->c.left);
	freechunk(c);
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
	pokeeach(&from->c, &from->c);
	cunlink(&from->c, &from->c);
	clink(em->c.left, c);
	from->off = 0;
	from->cp = &em->c;
	return 0;
}

int
emflip(EM *em, EM *victim)
{
	Chunk *c;

	printchain(&victim->c);
	printchain(&em->c);
	assert(victim->c.lright == &victim->c && victim->c.lleft == &victim->c);
	freechain(&em->c, &em->c);
	c = victim->c.right;
	cunlink(&victim->c, &victim->c);
	clink(&em->c, c);
	pokeeach(&em->c, &em->c);
	em->off = victim->off = 0;
	printchain(&victim->c);
	printchain(&em->c);
	victim->cp = &victim->c;
	return 0;
}

void
emflush(EM *em)
{
	Chunk *c;

	if(em->fd < 0)
		em->fd = mktmp(em);
	for(c=em->c.right; c!=&em->c; c=c->right)
		flushchunk(c);
}

vlong
emseek(EM *em, vlong off, int mode)
{
	Chunk *c;

	if(em->fd >= 0){
		if((off = seek(em->fd, off, mode)) < 0){
			warn("emseek: %s[%llx/%d]: %s", em->path, off, mode, error());
			return -1;
		}
	}
	/* just look up from start, chances are we're seeking towards there */
	if((c = chunkseek(&em->c, off)) != &em->c)
		poke(c);
	em->cp = c;
	em->off = off;
	return off;
}

static ssize
supply(ssize n)
{
	Chunk *c;

	dprint(Debugextmem, "supply %llx", n);
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

	dprint(Debugextmem, "shave");
	for(c=norris.lright; c!=&norris; c=c->lright)
		if((n = c->end - c->start) < c->bufsz){
			dprint(Debugextmem, "shave chunk %#p %llx → %llx",
				c, c->bufsz, c->end - c->start);
			c->buf = erealloc(c->buf, n, c->bufsz);
			memreallyfree += c->bufsz - n;
			c->bufsz = n;
		}
	dprint(Debugextmem, "done shaving");
}

static ssize
feedmem(ssize n)
{
	ssize m;
	Chunk *c;

	dprint(Debugextmem, "feedmem %llx [1] free %llx reallyfree %llx",
		n, memfree, memreallyfree);
	if(n > Maxgreed)
		n = Maxgreed;
	if(n <= memfree)
		return n;
	m = n - memfree;
	m -= supply(m);
	dprint(Debugextmem, "feedmem %llx [2] free %llx reallyfree %llx",
		n, memfree, memreallyfree);
	if(m <= memfree)
		return n;
	shave();
	m -= supply(m);
	dprint(Debugextmem, "feedmem %llx [3] free %llx reallyfree %llx",
		n, memfree, memreallyfree);
	if(m <= memfree)
		return n;
	if((c = chunkglob(&norris, m)) != &norris){
		freechain(c, c);
		freechunk(c);
	}
	dprint(Debugextmem, "feedmem %llx [4] free %llx reallyfree %llx",
		n, memfree, memreallyfree);
	if(m <= memfree)
		return n;
	assert(norris.lright == &norris);
	assert(memfree <= 0);
	assert(memreallyfree <= 0);
	return n - m;	/* dommage, mange ta main */
}

static int
chunkread(EM *em, Chunk *c)
{
	int r;

	if(em->fd < 0)
		em->fd = mktmp(em);
	dprint(Debugextmem, "chunkread %s[%llx..+%llx] (%llx) into %#p",
		em->path, c->start, c->end, c->bufsz, c);
	emseek(em, c->start, 0);
	if((r = read(em->fd, c->buf, c->bufsz)) < 0)
		sysfatal("chunkread: readn: %s", error());
	return r;
}

static Chunk *
chunkalloc(ssize *n, ssize off)
{
	ssize m, k;
	Chunk *cl, *c;

	m = *n;
	dprint(Debugextmem, "chunkalloc %llx", m);
	if((m = feedmem(m)) == 0){
		werrstr("chunkalloc: reached memory limit");
		return nil;
	}
	for(cl=nil, k=0; m>0; m-=c->bufsz, k+=c->bufsz){
		if((c = handouts.lright) == &handouts)
			break;
		poke(c);
		memfree -= c->bufsz;
		if(cl == nil)
			cl = c;
		else
			clink(cl, c);
		c->start = off;
		c->end = c->start + m;
		dprint(Debugextmem, "chunkalloc: chaining %#p[%llx]", c, c->bufsz);
	}
	*n = k;
	return cl;
}

static uchar *
fetch(EM *em, vlong off, ssize *want)
{
	ssize n;
	uchar *p;
	Chunk *c, *l;

	n = *want;
	c = em->cp;
	l = c->left;
	dprint(Debugextmem, "fetch %s[%llx..+%llx]",
		em->path, off, n);
	if(off < c->start || off + n >= c->end)
		c = chunkseek(&em->c, off);
	if(c == &em->c){
		if((c = chunkalloc(&n, off)) == nil)
			return nil;
		if((n = chunkread(em, c)) < 0){
			freechunk(c);
			return nil;
		}
		clink(em->c.left, c);
		p = c->buf;
	}else if(off < c->start){
		n = MIN(c->start - off, n);
		if((c = chunkalloc(&n, off)) == nil)
			return nil;
		if((n = chunkread(em, c)) < 0){
			freechunk(c);
			return nil;
		}
		c->fd = em->tmpfd < 0 ? em->fd : em->tmpfd;		/* hack */
		clink(l, c);
		p = c->buf;
	}else{
		em->cp = c;// = trymerge(em, c, off, n);		// FIXME
		n = MIN(c->end - off, n);
		p = c->buf + off - c->start;
		poke(c);
	}
	em->off = off + n;
	em->cp = em->off >= c->end ? c->right : c;
	*want = n;
	return p;
}

/* just keep hitting the piñata until as much candy as requested falls */
int
emread(EM *em, vlong off, uchar *u, ssize n)
{
	ssize m, k;
	uchar *p, *d, *e;

	dprint(Debugextmem, "emread %s[%llx..+%llx] → %#p",
		em->path, off, n, u);
	emseek(em, off, 0);
	for(d=u, e=d+n, k=0; d<e; d+=m, k+=m, n-=m){
		m = n;
		if((p = fetch(em, off, &m)) == nil)
			return -1;
		off += m;
		em->off = off;
		if(m == 0){
			em->off += n;
			return 0;
		}
		if(u != nil)			/* hack */
			memcpy(d, p, m);
	}
	return k;
}

int
emwrite(EM *em, vlong off, uchar *buf, ssize n)
{
	ssize m;
	uchar *p;
	Chunk *c;

	dprint(Debugextmem, "emwrite %s[%llx..+%llx]", em->path, off, n);
	assert(off >= 0);
	emseek(em, off, 0);
	for(m=n; n>0; n-=m, buf+=m, off+=m){
		m = n;
		if((p = fetch(em, off, &m)) == nil){
			sysfatal("fetch: nope");
			/* FIXME
			if((c = chunkalloc(&m, off)) == nil)
				return -1;
			c->fd = em->tmpfd < 0 ? em->fd : em->tmpfd;
			clink(em->c.left, c);
			*/
		}
		c = em->cp;
		if(m == 0){
			assert(c != &em->c && c->bufsz > 0);
			m = n < c->bufsz ? n : c->bufsz;
		}
		memcpy(p, buf, m);
	}
	return 0;
}

u64int
empget64(EM *em, vlong i)
{
	int m;
	uchar u[8];

	i *= 8;
	dprint(Debugextmem, "empget64 %s[%llx]", em->path, i);
		if((m = emread(em, i, u, sizeof u)) < 0)
		sysfatal("empget64 %s short read %llx: %s\n", em->path, i, error());
	dprint(Debugextmem, "empget64 %s: off now %llx res %d", em->path, em->off, m);
	if(m == 0)
		return EMeof;
	return GBIT64(u);
}

u64int
emget64(EM *em)
{
	int m;
	uchar u[8];

	dprint(Debugextmem, "emget64 %s[%llx]", em->path, em->off);
	if((m = emread(em, em->off, u, sizeof u)) < 0)
		sysfatal("empget64 %s short read %llx: %s", em->path, em->off, error());
	dprint(Debugextmem, "emget64 %s: off now %llx res %d", em->path, em->off, m);
	if(m == 0)
		return EMeof;
	return GBIT64(u);
}

int
empput64(EM *em, vlong i, u64int v)
{
	uchar u[8];

	i *= 8;
	dprint(Debugextmem, "empput64 %s[%llx]", em->path, i);
	emseek(em, i, 0);
	PBIT64(u, v);
	if(emwrite(em, i, u, sizeof u) < 0)
		sysfatal("empput64 %s %llx: %s", em->path, i, error());
	return 0;
}

int
emput64(EM *em, u64int v)
{
	uchar u[8];

	dprint(Debugextmem, "emput64 %s[%llx]", em->path, em->off);
	PBIT64(u, v);
	if(emwrite(em, em->off, u, sizeof u) < 0)
		sysfatal("empput64 %s %llx: %s", em->path, em->off, error());
	return 0;
}

EM*
emnew(void)
{
	EM *em;

	em = emalloc(sizeof *em);
	em->tmpfd = em->fd = -1;
	em->c.left = em->c.right = &em->c;
	em->c.lleft = em->c.lright = &em->c;
	em->cp = &em->c;
	return em; 
}

EM*
emfdopen(int fd)
{
	EM *em;

	dprint(Debugextmem, "emfdopen %d", fd);
	em = emnew();
	em->tmpfd = em->fd = fd;
	return em;
}

EM*
emopen(char *path)
{
	EM *em;

	dprint(Debugextmem, "emopen %s", path);
	em = emnew();
	if((em->fd = open(path, ORDWR)) < 0)
		return nil;
	em->tmpfd = em->fd;
	em->path = estrdup(path);
	return em;
}

EM*
emclone(char *path)
{
	ssize n, m, off;
	EM *em;

	dprint(Debugextmem, "emclone %s", path);
	if((em = emopen(path)) == nil)
		return nil;
	em->tmpfd = mktmp(nil);	/* hack */
	n = fsize(em->fd);
	dprint(Debugextmem, "emclone %s: fsize %llx", path, n);
	for(off=0; n>0; n-=m, off+=m){
		m = n;
		emread(em, off, nil, m);
	}
	dprint(Debugextmem, "emclone %s: done reading", path);
	printchain(&em->c);
	emflush(em);
	close(em->fd);
	free(em->path);
	em->fd = em->tmpfd;
	em->path = nil;
	em->tmpfd = -1;
	dprint(Debugextmem, "emclone %s: switched to new file", path);
	return em;
}

void
emclose(EM *em, int dontassume)
{
	if(em == nil)
		return;
	freechain(&em->c, &em->c);
	if(em->tmpfd >= 0 && em->tmpfd 	!= em->fd)
		close(em->tmpfd);
	if(em->fd >= 0)
		close(em->fd);
	if(em->path != nil && !dontassume){
		dprint(Debugextmem, "emclose %s: remove", em->path);
		sysremove(em->path);
	}
	free(em->path);
	free(em);
}
