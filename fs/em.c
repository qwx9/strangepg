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

/* FIXME: let the abstraction follow dfc-like specs and semantics:
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc. */
/* FIXME: maybe on linux stuff like mmap makes more sense; stdio */
/* FIXME: plan9: see also readv(2) */
/* FIXME: try a functional language approach? pass function ptr?
 * we iterate the same way but do different things, like 10 times
 * AND depending on which list (lright or right) */

static Chunk norris = {.lleft = &norris, .lright = &norris, .left = &norris, .right = &norris},
	handouts = {.lleft = &handouts, .lright = &handouts, .left = &handouts, .right = &handouts};

static ssize memfree, memreallyfree = Poolsz / Chunksz;
static uchar iobuf[IOUNIT];

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
	dprint(Debugextmem, "poke %#p ll %#p lr %#p", c, c->lleft, c->lright);
}
static void
pokeeach(Chunk *l, Chunk *le){
	Chunk *c, *r;

	if(l == nil)
		return;
	for(c=l->right; c!=le; c=r){
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
	cunlink(c, c);
	lunlink(c, c);
	free(c->buf);
	free(c);
	memreallyfree++;
}

static int
flushchunk(EM *em, Chunk *c)
{
	ssize n;

	dprint(Debugextmem, "flushchunk %#p[%llx]", c, c->off);
	if(seek(em->fd, c->off, 0) < 0)
		return -1;
	n = Chunksz;
	if(em->farthest > c->off)
		n = MIN(Chunksz, em->farthest - c->off);
	if(write(em->fd, c->buf, n) != n)
		return -1;
	return 0;
}

void
emflush(EM *em)
{
	Chunk *c;

	for(c=em->c.right; c!=&em->c; c=c->right)
		flushchunk(em, c);
}

static void
freechunk(EM *em, Chunk *c)
{
	dprint(Debugextmem, "freechunk %#p", c);
	if(c == nil || c == &em->c)
		return;
	if(em->fd >= 0 && flushchunk(em, c) < 0)
		warn("freechunk: %s\n", error());
	cunlink(c, c);
	lunlink(c, c);
	llink(handouts.lleft, c);
	c->off = -1;
}

static void
freechain(EM *em, Chunk *l, Chunk *le)
{
	Chunk *c, *r;

	dprint(Debugextmem, "freechain %#p le %#p: ", l, le);
	if(l == nil || l->right == l->left)
		return;
	for(c=l->right; c!=le; c=r){
		r = c->right;
		freechunk(em, c);
		memfree++;
	}
}

static Chunk *
chunkseek(Chunk *l, ssize off)
{
	Chunk *c, *p;

	assert((off & (Chunksz-1)) == 0);
	for(p=c=l->right; c!=l; p=c, c=c->right){
		if(c->off > off)
			break;
	}
	return p->off < off ? l : p;
}

void
printchain(Chunk *r)
{
	Chunk *c, *l;

	dprint(Debugextmem, "chain %#p: ", r);
	for(c=r, l=nil; l!=r; c=c->right, l=c)
		dprint(Debugextmem, " %#p[%llx]", c, c->off);
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

static Chunk *
allocchunk(void)
{
	Chunk *c;

	c = emalloc(sizeof *c);
	c->buf = emalloc(Chunksz);
	c->right = c->left = c->lright = c->lleft = c;
	memreallyfree--;
	dprint(Debugextmem, "allocchunk %#p size %#llx", c, Chunksz);
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
readchunk(EM *em, Chunk *c, vlong off)
{
	ssize n;

	if(em->fd < 0 || em->stream)
		return 0;
	if(seek(em->fd, off, 0) < 0)
		return -1;
	if((n = readn(em->fd, c->buf, Chunksz)) < 0)
		sysfatal("readchunk %s: %s", em->path, error());
	return n;
}

static Chunk *
getchunk(EM *em, vlong off, ssize *sz)
{
	ssize n;
	vlong coff;
	Chunk *c;

	dprint(Debugextmem, "getchunk: off %llx (%llx)",
		off, off - off%Chunksz);
	assert(off >= 0 && off != EMbupkis);
	coff = off -  off % Chunksz;
	if((c = chunkseek(&em->c, coff)) == &em->c || c->off > coff){
		c = newchunk();
		clink(em->c.left, c);
		if((n = readchunk(em, c, off)) < 0)
			sysfatal("getchunk: %s", error());
		if(sz != nil)
			*sz = n;
	}else
		poke(c);
	c->off = coff;
	return c;
}

/* align to chunk boundary and free everything past it */
ssize
emshrink(EM *em, ssize sz)
{
	Chunk *c;

	sz += Chunksz - sz % (Chunksz+1);
	dprint(Debugextmem, "emshrink %s[%llx] → %llx",
		em->path, em->c.left->off + Chunksz, sz);
	if((c = chunkseek(&em->c, sz)) == &em->c){
		dprint(Debugextmem, "emshrink: nothing to resize");
		return 0;
	}
	freechain(em, c->left, &em->c);
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
	return 0;
}

int
emembraceextendextinguish(EM *em, EM *victim)
{
	Chunk *c;

	printchain(&victim->c);
	printchain(&em->c);
	assert(victim->c.lright == &victim->c && victim->c.lleft == &victim->c);
	freechain(em, &em->c, &em->c);
	c = victim->c.right;
	cunlink(&victim->c, &victim->c);
	clink(&em->c, c);
	pokeeach(&em->c, &em->c);
	printchain(&victim->c);
	printchain(&em->c);
	return 0;
}

ssize
emwrite(EM *em, vlong off, uchar *buf, ssize n)
{
	ssize m;
	uchar *p, *s;
	Chunk *c;

	if(em->fd < 0){
		werrstr("emwrite: no file");
		return -1;
	}
	for(s=buf; n>0; n-=m, off+=m, s+=m){
		c = getchunk(em, off, nil);
		m = off - c->off;
		dprint(Debugextmem, "emwrite: write [%llx..%llx] → %#p Δ %llx",
			off, off+n, c, m);
		p = c->buf + m;
		m = n < Chunksz - m ? n : Chunksz - m;
		memcpy(p, s, m);
	}
	m = s - buf;
	em->off = off + m;
	if(em->off > em->farthest)
		em->farthest = em->off;
	return m;
}

uchar *
emread(EM *em, vlong off, ssize *want)
{
	ssize n, m;
	uchar *p;
	Chunk *c;

	n = *want;
	c = getchunk(em, off, &m);
	if(m == 0){
		*want = 0;
		return nil;
	}
	p = c->buf + off - c->off;
	*want = MIN(n, c->off + Chunksz - off);
	return p;
}

int
empput64(EM *em, vlong off, u64int v)
{
	uchar u[8];

	assert(off >= 0 && off != EMbupkis);
	off *= 8;
	dprint(Debugextmem, "empput64 %s[%llx] ← %llux", em->path, off, v);
	PBIT64(u, v);
	if(emwrite(em, off, u, sizeof u) < 0)
		sysfatal("empput64 %s %llx: %s", em->path, off, error());
	return 0;
}

u64int
empget64(EM *em, vlong off)
{
	ssize m;
	uchar *p;
	u64int v;

	assert(off >= 0 && off != EMbupkis);
	off *= 8;
	dprint(Debugextmem, "empget64 %s[%llx]", em->path, off);
	m = 8;
	p = emread(em, off, &m);
	if(p == nil || m == 0)
		return EMbupkis;
	if(m < 0 || m > 0 && m != 8)
		sysfatal("empget64 %s short read [%llx] %llx: %s", em->path, off, m, error());
	em->off = off + 8;
	v = GBIT64(p);
	dprint(Debugextmem, "empget64: [%llx] got %llux", off, v);
	return v;
}

/* these are dangerous, use with care; not maintaining a pointer
 * to current chunk either */
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
	em->c.left = em->c.right = &em->c;
	em->c.lleft = em->c.lright = &em->c;
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

EM*
emopen(char *path)
{
	EM *em;

	dprint(Debugextmem, "emopen %s", path);
	em = emnew(1);
	em->path = estrdup(path);
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
	freechain(em, &em->c, &em->c);
	if(em->fd >= 0)
		close(em->fd);
	if(em->path != nil && em->tmp){
		dprint(Debugextmem, "emclose %s: remove", em->path);
		sysremove(em->path);
	}
	free(em->path);
	free(em);
}
