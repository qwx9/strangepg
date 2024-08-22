#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "em.h"

/* FIXME:
 * - always clean up on exit: sig/notehandler
 * - test string and arbitrary read/write functions
 * - windowing, buffering, etc.
 * - prefetch, ie. readahead or other?
 * - testct13: test concurrent em with 10 procs (old: add qlock)
 * - tune Nwriters, channel size, age, etc.
 * - buf[Pagesz]: should it rather be allocated contiguous than just
 *	 pointing to a buffer? would be better for sequential access
 * - emfree; reuse freed etab slots
 * - read/write functions as inline or macros?
 * - add test: read from file (new=1)
 */

int multiplier = 30;

enum{
	/* must be powers of two */
	Poolsz = 1ULL<<32,
	Pshift = 16,
	Pagesz = 1ULL<<Pshift,
	Pmask = Pagesz - 1,
	Bshift = 12,
	Banksz = 1ULL<<Bshift,
	Bmask = Banksz - 1,
	Ashift = Pshift + Bshift,
	Amask = (1ULL<<Ashift) - 1,
	Pelder = 32,	/* FIXME: tune w/ prints */
	Chansz = 16,	/* FIXME: tune w/ prints */
	Nwriters = 10,
	Fdirty = 1<<0,
	Fsent = 1<<1,
	Eclosing = 1<<0,
};
typedef struct Page Page;
typedef struct Bank Bank;
struct Page{
	Bank *bank;
	char flags;
	uvlong age;
	usize vaddr;
	uchar buf[Pagesz];
	RWLock l;
	Page *lleft;
	Page *lright;
};
struct Bank{
	ssize paddr;
	EM *em;
	Page *pt[Banksz];
};
struct EM{
	int ref;
	char flags;
	int infd;
	RWLock l;
	Bank **banks;
	Page *cur;
};
static int swapfd = -1;
static Bank **banks;
static RWLock elock;
static EM **etab, *emstrbuf;
static Page pused = {.lleft = &pused, .lright = &pused};
static uvlong poolsz = Poolsz;
static ssize mpages, memfree, memreallyfree = Poolsz / Pagesz;
static Channel *wchan[Nwriters];
static uvlong emtc;
static vlong stroff;

#define PLINK(u, v)	do{ \
	Page *a = (u), *b = (v); \
	a->lright->lleft = b->lleft; \
	b->lleft->lright = a->lright; \
	a->lright = b; \
	b->lleft = a; \
}while(0)
#define PUNLINK(p)	do{ \
	(p)->lright->lleft = (p)->lleft; \
	(p)->lleft->lright = (p)->lright; \
	(p)->lleft = (p); \
	(p)->lright = (p); \
}while(0)
#define PRELINK(ll, p)	do{ \
	PUNLINK((p)); \
	PLINK((ll), (p)); \
}while(0)

/* FIXME: naming */
#define	VADDR(off)	((off) & ~Pmask)
#define	VOFF(off)	((off) & Pmask)
#define	PADDR(off, pa)	((pa) | VADDR((off)) & Amask)
#define	BANK(off)	((off) >> Ashift)
#define	PAGE(off)	((off) >> Pshift & Bmask)
#define PLEA(p, a)	((p)->buf + VOFF((a)))
#define PREAD(fd, p, off)	do{ \
	if(pread((fd), (p)->buf, sizeof((p)->buf), (off)) < 0) \
		warn("pread: %s\n", error()); \
}while(0)

/* FIXME
static void
cleanup(void *)
{
	int i;
	EM **em;

	if(swapfd < 0)
		return;
	wlock(&elock);
	close(swapfd);
	swapfd = -1;
	for(em=etab; em<etab+dylen(etab); em++){
		(*em)->ref = 0;
		emclose(*em);
		free(*em);
	}
	dyfree(etab);
	wunlock(&elock);
}
*/

/* FIXME: UGHHHHHH */
static inline Page *
GETPAGE(EM *em, ssize off)
{
	int new, old;
	ssize i, va;
	Bank *bank;
	Page *p, *l;

	va = VADDR(off);
	if(em->cur == nil || (p = em->cur)->vaddr != va){
		old = new = 0;
		i = BANK(off);
		if(i >= dylen(em->banks) || (bank = em->banks[i]) == nil){
			bank = emalloc(sizeof *bank);
			bank->paddr = dylen(banks) << Ashift;
			bank->em = em;
			wlock(&elock);
			dypush(banks, bank);
			wunlock(&elock);
			wlock(&em->l);
			dyinsert(em->banks, i, bank);
			wunlock(&em->l);
			new = 1;
		}else if((p = bank->pt[PAGE(off)]) == nil)
			new = 1;
		else if(p->vaddr != va || p->bank != bank)
			old = 1;
		else
			goto relink;
		rlock(&pused.l);
		p = pused.lleft;
		rlock(&p->l);
		runlock(&pused.l);
		vlong m = 0;
		while((p->flags & Fdirty) != 0 || p == &pused){
			if(p->vaddr == va && p->bank == bank){
				old = new = 0;
				break;
			}
			l = p->lleft;
			rlock(&l->l);
			runlock(&p->l);
			p = l;
			m++;
		}
		runlock(&p->l);
		p->vaddr = va;
		p->bank = bank;
		if(p->age == 0)	/* virgin */
			memfree--;
		p->age = 0;
		bank->pt[PAGE(va)] = p;
		if(m > 64)
			warn("stalled, %lld iterations age %#llx emtc %#llx\n",
				m, p->age, emtc);
relink:
		assert(p != &pused);
		em->cur = p;
		if(p->age == 0 || emtc - p->age >= Pelder){
			PUNLINK(p);
			PLINK(&pused, p);
			p->age = emtc;
		}
		emtc++;
		if(old)
			PREAD(swapfd, p, PADDR(va, bank->paddr));
		else if(new && em->infd >= 0)
			PREAD(em->infd, p, VADDR(off));
	}
	return p;
}

static void
cacheproc(void *arg)
{
	EM *em;
	Bank *bank;
	Page *p;
	Channel *c;

	c = arg;
	for(;;){
		if((p = recvp(c)) == nil)
			break;
		bank = p->bank;
		em = bank->em;
		em->ref--;
		if((p->flags & Fdirty) == 0){
			p->flags &= ~Fsent;
			continue;
		}
		if((em->flags & Eclosing) == 0){
			wlock(&p->l);
			DPRINT(Debugextmem, "flush %#p:%#p:%#p\n", em, p, p->buf);
			if(pwrite(swapfd, p->buf, sizeof p->buf, PADDR(p->vaddr, bank->paddr)) < 0)
				warn("flush: %s\n", error());
			wunlock(&p->l);
		}else if(em->ref <= 0)
			emclose(em);
		p->flags &= ~(Fdirty | Fsent);
	}
}

static void
feedpages(void)
{
	ssize n;
	Page *l, *r, *pl, *p, *pe;

	n = memreallyfree;
	if(n > 128)
		n = 128;
	pl = emalloc(n * sizeof *pl);
	memreallyfree -= n;
	for(p=pl, pe=p+n; p<pe; p++){
		p->lright = p + 1;
		p->lleft = p - 1;
	}
	p--;
	r = &pused;
	p->lright = r;
	l = r->lleft;
	pl->lleft = l;
	r->lleft = p;
	l->lright = pl;
	mpages += n;
	memfree += n;
}

/* FIXME: check if the slowdowns on linux we've seen are because of the
 * sleep amount; this function over all just sucks, we need better */
static void
lruproc(void *)
{
	vlong n;
	uint i;
	int j;
	EM *em;
	Page *p, *l;

	for(;;){
		p = pused.lleft;
		/* this doesn't do any locking so scanning is just optimistic
		 * but in practice it's faster to do this and go wherever */
		for(n=0, i=0, j=0;; n++){
			l = p->lleft;
			if(p->bank == nil)
				goto skip;
			em = p->bank->em;
			if((em->flags & Eclosing) != 0){
				p->flags &= ~(Fdirty|Fsent);
				goto skip;
			}
			if((p->flags & (Fdirty|Fsent)) == Fdirty){
				for(j=0; j<nelem(wchan); j++)
					if(nbsendp(wchan[i++ % nelem(wchan)], p) != 0)
						break;
				if(j == nelem(wchan))
					break;
				p->flags |= Fsent;
				em->ref++;
			}
		skip:
			if(l == &pused || n >= mpages)
				break;
			p = l;
		}
		if(j < nelem(wchan))
			lsleep(100);
	}
}

void
emw64(EM *em, vlong off, u64int v)
{
	uchar *u;
	Page *p;

	off *= 8;
	p = GETPAGE(em, off);
	u = PLEA(p, off);
	PBIT64(u, v);
	p->flags |= Fdirty;
	DPRINT(Debugextmem, "w64 %#llx:%#p:%#p[%#llx][%#llx]:%#p[%#llx] (%#llx) → %llx", PADDR(off, p->bank->paddr), em, p->bank, BANK(off), PAGE(off), p, VOFF(off), off/8, v);
}

u64int
emr64(EM *em, vlong off)
{
	uchar *u;
	u64int v;
	Page *p;

	off *= 8;
	p = GETPAGE(em, off);
	u = PLEA(p, off);
	v = GBIT64(u);
	DPRINT(Debugextmem, "r64 %#llx:%#p:%#p[%#llx][%#llx]:%#p[%#llx] (%#llx) ← %llx", PADDR(off, p->bank->paddr), em, p->bank, BANK(off), PAGE(off), p, VOFF(off), off/8, v);
	return v;
}

void
emwrite(EM *em, vlong off, uchar *buf, int sz)
{
	int n;
	uchar *u;
	vlong bound;
	Page *p;

	assert(buf != nil);
	while(sz > 0){
		p = GETPAGE(em, off);
		u = PLEA(p, off);
		bound = off + (1 << Pshift) & ~Pmask;
		n = sz;
		if(bound - off < sz)
			n = bound - off;
		memcpy(u, buf, n);
		sz -= n;
		off = bound;
	}
}

/* assumption: fits in buffer, not going to bother further; instead
 * "allocate" within page boundaries */
uchar *
emread(EM *em, vlong off, int sz, vlong *n)
{
	vlong bound;
	uchar *u;
	Page *p;

	p = GETPAGE(em, off);
	u = PLEA(p, off);
	bound = off + (1 << Pshift) & ~Pmask;
	if(bound - off < sz)
		sz = bound - off;
	*n = sz;
	return u;
}

/* 64k max characters ought to be quite enough... */
char *
emgetstring(EMstring off)
{
	char *s;
	Page *p;

	/* uninitialized or past the tail are both errors */
	if(emstrbuf == nil){
		emstrbuf = emopen(nil);
		return nil;
	}else if(off >= stroff)
		return nil;
	p = GETPAGE(emstrbuf, off);
	s = (char *)PLEA(p, off);
	return s;
}

EMstring
emputstring(char *s, int len)
{
	uchar *u;
	vlong off, bound;
	Page *p;

	len++;
	if(len > Pagesz){
		werrstr("string too long");
		return -1;
	}
	if(emstrbuf == nil)
		emstrbuf = emopen(nil);
	off = stroff;
	bound = off + (1 << Pshift) & ~Pmask;
	if(bound - off < len)
		off = bound;
	p = GETPAGE(emstrbuf, off);
	u = PLEA(p, off);
	memcpy(u, (uchar *)s, len);
	stroff = off + len;
	return off;
}

/* FIXME: overwrites clean pages, wasting time */
int
em2fs(EM *em, File *f, ssize nbytes)
{
	u64int u;
	vlong off;

	if(em == nil || (em->flags & Eclosing) != 0)
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
	if(em == nil)
		return;
	em->flags |= Eclosing;
	if(em->ref > 0)
		return;
	if(em->infd >= 0)
		close(em->infd);
}

EM *
emopen(char *path)
{
	int i;
	EM *em;

	em = emalloc(sizeof *em);
	dypush(etab, em);
	em->infd = -1;
	if(path != nil && (em->infd = open(path, OREAD)) < 0){
		free(em);
		return nil;
	}
	if(swapfd < 0){
		if((swapfd = sysmktmp()) < 0)
			sysfatal("emopen: %s", error());
		for(i=0; i<nelem(wchan); i++)
			if((wchan[i] = chancreate(sizeof(Page*), Chansz)) == nil)
				sysfatal("chancreate: %s", error());
		feedpages();
		/* FIXME: proper cleanup: cleanfn + reclaim used: kill when empty */
		newthread(lruproc, nil, nil, nil, "lru", mainstacksize);
		for(i=0; i<nelem(wchan); i++)
			newthread(cacheproc, nil, wchan[i], nil, "cache", mainstacksize);
	}
	return em;
}

void
initem(void)
{
	if(multiplier < 28 || multiplier > 63)
		sysfatal("invalid memory size");
	poolsz = 1ULL << multiplier;
	memreallyfree = poolsz / Pagesz;
}
