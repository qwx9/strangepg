#include "strpg.h"
#include "fs.h"
#include "em.h"

/* - currently just greedily uses up as much memory as is given,
 * but could apply some balancing heuristic to free stuff that
 * hasn't been used in a while
 * - worker(s) with their own memory limit instead of a global one
 * - defining a hierarchy of priorities (multiple lists)
 * - sync: temporarily remove chunks from the global lists, etc. */
/* FIXME: let the abstraction follow dfc-like specs and semantics...?
 *	eg. fetch record n of type x from EM image y → calculate offset,
 *	cache, etc. ← would be external to em, this is generic enough */
/* FIXME: don't do while loop in dygrow, compute size directly */

int multiplier = 29;

KHASH_MAP_INIT_INT64(pagetab, void*)

enum{
	/* must be powers of two */
	Poolsz = 1ULL<<32,
	Pshift = 16,
	Pagesz = 1<<Pshift,
	Pmask = Pagesz - 1,
	Bshift = 12,
	Banksz = 1<<Bshift,
	Bmask = Banksz - 1,
};
static uvlong poolsz = Poolsz;
typedef struct Page Page;
struct Page{
	int e;
	char dirty;
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
	Page ***banks;	/* banks → pages → page */
};
static EM **etab;
static Page pused = {.lleft = &pused, .lright = &pused},
	pfree = {.lleft = &pfree, .lright = &pfree};
static ssize memfree, memreallyfree = Poolsz / Pagesz;
#define Sucks	((Page*)0x1)	/* yes it does */

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
#define PRELINK(ll, p)	do{ \
	(p)->lleft->lright = (p)->lright; \
	(p)->lright->lleft = (p)->lleft; \
	(ll)->lright->lleft = (p); \
	(p)->lleft = (ll); \
	(p)->lright = (ll)->lright; \
	(ll)->lright = (p); \
	}while(0)
#define POKE(p)	do{	\
	PUNLINK((p), (p)); \
	PLINK(pused.lleft, (p)); \
	}while(0)
#define BADDR(p)	((p) >> Pshift + Bshift)
#define PADDR(p)	((p) >> Pshift & Bmask)
#define POFF(p)		((p) & Pmask)
#define PLEA(p, a)	((p)->buf + POFF((a)))

static void
poolcheck(void)
{
	Page ***b, **pp, **pe, *p;
	EM **ep, *em;

	for(ep=etab; ep<etab+dylen(etab); ep++){
		if((em = *ep) == nil)
			continue;
		for(b=em->banks; b<em->banks+dylen(em->banks); b++){
			if(*b == nil)
				continue;
			for(pp=*b, pe=pp+Banksz; pp<pe; pp++){
				if((p = *pp) <= Sucks)
					continue;
				assert(p->e == em->id);
				assert(p->e == ep - etab);
				assert(BADDR(p->addr) < dylen(em->banks));
				assert(em->banks[BADDR(p->addr)] != nil);
				assert(em->banks[BADDR(p->addr)][PADDR(p->addr)] == p);
			}
		}
	}
}

/* FIXME: make these unto macros as well? */
static int
flush(EM *em, Page *p)
{
	if(em->fd < 0){
		if(em->path == nil)
			em->path = sysmktmp();	/* can be stupidly expensive */
		if((em->fd = create(em->path, ORDWR, 0640)) < 0)
			return -1;
	}
	if(seek(em->fd, p->addr, 0) < 0)
		warn("flush: seek %d: %s\n", em->fd, error());
	if(write(em->fd, p->buf, sizeof p->buf) != sizeof p->buf)
		warn("flush: write %d: %s\n", em->fd, error());
	return 0;
}
static void
freepage(EM *em, Page *p)
{
	if(p->dirty)
		flush(em, p);
	em->banks[BADDR(p->addr)][PADDR(p->addr)] = nil;
	PRELINK(pfree.lleft, p);
	memfree++;
}

static Page *
preclaim(void)
{
	Page *p;
	EM *em;

	p = pused.lright;
	if((em = etab[p->e]) != nil){
		if(p->dirty)
			flush(em, p);
		em->banks[BADDR(p->addr)][PADDR(p->addr)] = nil;
	}
	memset(p->buf, 0, Pagesz);
	return p;
}
static Page *
preuse(void)
{
	Page *p;

	p = pfree.lleft;
	DPRINT(Debugextmem, "preuse %#p memfree %zx", p, memfree);
	memset(p->buf, 0, Pagesz);
	memfree--;
	return p;
}
static Page *
new(void)
{
	int n;
	Page *p, *pl, *pe;

	n = memreallyfree < Banksz ? memreallyfree : Banksz;
	pl = emalloc(n * sizeof *pl);
	memreallyfree -= n;
	pl->lright = pl->lleft = pl;
	for(p=pl+1, pe=pl+n; p<pe; p++){
		p->lright = p->lleft = p;
		PLINK(pfree.lleft, p);
		memfree++;
	}
	return pl;
}

#define PREAD(fd, page, off)	do{ \
	if(seek((fd), 0, 2) <= (off)) \
		break; \
	if(seek((fd), (off) & ~Pmask, 0) < 0) \
		warn("seek: %s\n", error()); \
	else if(read((fd), (page)->buf, sizeof((page)->buf)) < 0) \
		warn("read: %s\n", error()); \
	}while(0)
#define BALLOC()	(emalloc(Banksz * sizeof(Page**)))
/* POKEd at the end */
#define PALLOC(em, page, off)	do{ \
	if(memfree > 0) \
		(page) = preuse(); \
	else if(memreallyfree > 0) \
		(page) = new(); \
	else \
		(page) = preclaim(); \
	(page)->e = (em)->id; \
	(page)->addr = (off) & ~Pmask; \
	}while(0)
#define PAGE(em, page, off)	do{ \
	int __n; \
	Page **__bank, **__pp; \
	__n = BADDR((off)); \
	dygrow((em)->banks, __n); \
	if(((__bank) = (em)->banks[__n]) == nil){ \
		(em)->banks[__n] = __bank = BALLOC(); \
		/* this SUCKS */ \
		if((em)->infd >= 0) \
			for(__pp=__bank; __pp<__bank+Banksz; __pp++) \
				*__pp = Sucks; \
	} \
	__pp = __bank + PADDR((off)); \
	if(((page) = *__pp) <= Sucks){ \
		PALLOC((em), (page), (off)); \
		if(*__pp == Sucks) \
			PREAD((em)->infd, (page), (off)); \
		else if((em)->fd >= 0) \
			PREAD((em)->fd, (page), (off)); \
		*__pp = (page); \
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
	DPRINT(Debugextmem, "r64 %#p:%s %llx %llx [%#p]", em, em->path, off/8, GBIT64(u), u);
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
	DPRINT(Debugextmem, "w64 %#p:%#p [%zx] %llx %llx [%#p]", em, em->banks[BADDR(off)], off/8, off, v, u);
	PBIT64(u, v);
	p->dirty = 1;
}

/* FIXME: slow as sloth taking shit */
int
em2fs(EM *em, File *f, ssize nbytes)
{
	u64int u;
	vlong off;

	if(em->banks == nil)
		return 0;
	for(off=0; off<nbytes/sizeof u; off++){
		//n = nbytes - off > Pagesz ? Pagesz : nbytes - off;
		u = emr64(em, off);
		put64(f, u);
	}
	return 0;
}

void
emclose(EM *em)
{
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
	EM *em;

	em = emalloc(sizeof *em);
	dypush(etab, em);
	em->id = dylen(etab) - 1;
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

void
initem(void)
{
	if(multiplier < 16 || multiplier > 63)
		sysfatal("invalid memory size");
	poolsz = 1ULL << multiplier;
	memreallyfree = poolsz / Pagesz;
}
