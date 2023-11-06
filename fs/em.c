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
/* FIXME: linux: mmap? plan9: see also readv(2) */
/* FIXME: don't do while loop in dygrow, compute size directly */
/* FIXME: do away with chains, useless (if it's less code) */

enum{
	/* must be powers of two */
	Poolsz = 1ULL<<30,
	Bshift = 24,
	Banksz = 1<<8,
	Bmask = Banksz - 1,
	Pshift = 16,
	Pagesz = 1<<16,
	Pmask = Pagesz - 1,

	EMrdonly = 1<<0,
};
typedef struct Page Page;
typedef Page**	Bank;
struct Page{
	int e;
	usize addr;
	uchar buf[Pagesz];
	Page *lleft;
	Page *lright;
};
struct EM{
	int id;
	int flags;
	int fd;
	int infd;
	char *path;
	Bank *banks;
};
static EM **etab;
static Page pused = {.lleft = &pused, .lright = &pused},
	pfree = {.lleft = &pfree, .lright = &pfree};
static ssize memfree, memreallyfree = Poolsz / Pagesz;
static uchar iobuf[IOUNIT];

#define PLINK(u,v)	do{ \
	Page *a = (u), *b = (v); \
	b->lleft->lright = a->lright; \
	a->lright->lleft = b->lleft; \
	b->lleft = a; \
	a->lright = b; \
	}while(0)
#define PUNLINK(u,v)	do{ \
	Page *a = (u), *b = (v); \
	a->lleft->lright = b->lright; \
	b->lright->lleft = a->lleft; \
	a->lleft = b; \
	b->lright = a; \
	}while(0)
#define POKE(p)	do{	\
	PUNLINK((p), (p)); \
	PLINK(pused.lleft, (p)); \
	}while(0)
#define BADDR(p)	((p) >> Bshift)
#define PADDR(p)	((p) >> Pshift & Bmask)
#define POFF(p)		((p) & Pmask)
#define PLEA(p, a)	((p)->buf + POFF((a)))

/* FIXME: make these unto macros as well? */
static int
flush(Page *p)
{
	EM *em;

	em = etab[p->e];
	if(em->flags & EMrdonly)
		return -1;
	if(em->fd < 0){
		if(em->path == nil)
			em->path = sysmktmp();	/* can be stupidly expensive */
		if((em->fd = create(em->path, ORDWR, 0640)) < 0)
			return -1;
		warn("fd %d\n", em->fd);
	}
	seek(em->fd, p->addr, 0);
	write(em->fd, p->buf, sizeof *p->buf);
	return 0;
}
static void
freepage(Page *p)
{
	if(p == nil)
		return;
	warn("pe %d\n", p->e);
	if(p->e >= 0)
		flush(p);
	PUNLINK(p, p);
	PLINK(pfree.lleft, p);
	memfree++;
}

static Page *
preclaim(void)
{
	Page *p;
	EM *em;

	p = pused.lright;
	em = etab[p->e];
	if((em->flags & EMrdonly) == 0)
		flush(p);
	em->banks[BADDR(p->addr)][PADDR(p->addr)] = nil;
	memset(p->buf, 0, Pagesz);
	return p;
}
static Page *
preuse(void)
{
	Page *p;

	p = pfree.lleft;
	memset(p->buf, 0, Pagesz);
	memfree--;
	return p;
}
static Page *
new(void)
{
	Page *p;

	p = emalloc(sizeof *p);
	p->lright = p->lleft = p;
	memfree++;
	memreallyfree--;
	return p;
}

#define PREAD(fd, page, off)	do{ \
	if(seek((fd), (off), 0) < 0) \
		warn("seek: %s\n", error()); \
	else if(read((fd), (page)->buf, sizeof((page)->buf)) < 0) \
		warn("read: %s\n", error()); \
	}while(0)
#define BALLOC()	(emalloc(Banksz * sizeof(Bank)))
#define PALLOC(em, page, off)	do{ \
	if(memfree > 0) \
		(page) = preuse(); \
	else if(memreallyfree > 0) \
		(page) = new(); \
	else \
		(page) = preclaim(); \
	(page)->e = (em)->id; \
	(page)->addr = (off) & ~Pmask; \
	POKE((page)); \
	}while(0)
#define PAGE(em, page, off)	do{ \
	int __n, __readme = 0; \
	Bank __bank, *__bp; \
	Page **__pp; \
	__n = BADDR((off)); \
	dygrow((em)->banks, __n+1); \
	__bp = (em)->banks + __n; \
	if((__bank = *__bp) == nil){ \
		__bank = *__bp = BALLOC(); \
		if((em)->infd >= 0) \
			__readme = 1; \
	} \
	__pp = __bank + PADDR((off)); \
	if(((page) = *__pp) == nil){ \
		PALLOC((em), (page), (off)); \
		*__pp = (page); \
		if(__readme) \
			PREAD((em)->infd, (page), (off)); \
		else if((em)->fd >= 0) \
			PREAD((em)->fd, (page), (off)); \
	} \
	POKE((page)); \
	}while(0)

uchar *
emptr(EM *em, vlong off)
{
	Page *p;

	PAGE(em, p, off);
	return PLEA(p, off);
}

u64int
emr64(EM *em, vlong off)
{
	uchar *u;
	Page *p;

	off *= 8;
	PAGE(em, p, off);
	u = PLEA(p, off);
	DPRINT(Debugextmem, "read %llx %llx [%#p]", off/8, GBIT64(u), u);
	return GBIT64(u);
}

void
emw64(EM *em, vlong off, u64int v)
{
	uchar *u;
	Page *p;

	off *= 8;
	PAGE(em, p, off);
	u = PLEA(p, off);
	DPRINT(Debugextmem, "emw64 %llx %llx [%#p]", off, v, u);
	PBIT64(u, v);
}

int
em2fs(EM *em, File *f)
{
	vlong n;
	Bank *b;
	Page **p, **pe;
	File *ef;

	for(b=em->banks; b<em->banks+dylen(em->banks); b++){
		warn("em2fs: bank %zd\n", b-em->banks);
		if(*b != nil)
			for(p=*b, pe=p+Banksz; p<pe; p++)
				freepage(*p);
	}
	if(em->fd < 0){
		warn("em2fs: nothing to write\n");
		return 0;
	}
	ef = emalloc(sizeof *ef);
	if(fdopenfs(ef, em->fd, OREAD) < 0)
		goto err;
	for(;;){
		if((n = readfs(ef, iobuf, sizeof iobuf)) <= 0)
			break;
		if(writefs(f, iobuf, n) < 0)
			goto err;
	}
	closefs(ef);
	free(ef);
	if(n == 0)
		return 0;
err:
	free(ef);
	return -1;
}

void
emclose(EM *em)
{
	Bank *b;
	Page **p, **pe;

	for(b=em->banks; b<em->banks+dylen(em->banks); b++)
		if(*b != nil)
			for(p=*b, pe=p+Banksz; p<pe; p++)
				freepage(*p);
	if(em->infd >= 0)
		close(em->infd);
	if(em->fd >= 0)
		close(em->fd);
	if(em->path != nil){
		sysremove(em->path);
		free(em->path);
	}
	if(em->banks != nil)
		dyfree(em->banks);
	etab[em->id] = nil;
	free(em);
}

EM *
emopen(char *path, int flags)
{
	int i;
	EM *em;

	em = emalloc(sizeof *em);
	if(etab != nil){
		for(i=0; i<dylen(etab); i++)
			if(etab[i] == nil){
				etab[i] = em;
				em->id = i;
				break;
			}
		if(i == nelem(etab))
			dypush(etab, em);
	}else{
		dypush(etab, em);
		em->id = 0;
	}
	em->flags = flags;
	em->infd = em->fd = -1;
	if(path != nil){
		if((em->infd = open(path, OREAD)) < 0){
			free(em);
			return nil;
		}
	}
	return em;
}
