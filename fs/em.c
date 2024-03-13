#include "strpg.h"
#include "fs.h"
#include "em.h"

/* FIXME:
 * - stalling due to tail being heavily contended and undirtied pages nabbed
 *	 too quickly; lruproc just keeps going ahead but those pages are not relinked
 *	 instead, work on the tail more aggressively in lruproc and let further dirty
 *	 pages bubble up naturally
 * - cleanup(): interrupt threads, force free em and remove temp files
 * - always clean up on exit: sig/notehandler
 * - use banking again; benchmark against hashing
 * - param tuning
 * - bigger/smaller pages? banks?
 * - non-64 byte accesses
 * - macrofy PAGE again
 * - windowing, buffering, etc.
 * - make lruproc yield cpu sometime
 * - prefetch, ie. readahead or other?
 * - testct11: Nf==1, test with 10
 * - testct13: test concurrent em with 10 procs (old: add qlock)
 */

int multiplier = 29;

enum{
	/* must be powers of two */
	Poolsz = 1ULL<<32,
	Chunksz = 4096,
	Pshift = 16,
	Pagesz = 1<<Pshift,
	Pmask = Pagesz - 1,
	Idsz = 1<<10,
	Ishift = 64 - 10,
	Imask = Idsz-1,
	Addrmask = (1ULL<<Ishift)-1,
	Pelder = 128,	/* FIXME: tune */
	Nwriters = 4,	/* FIXME: tune (also channel size) */
	Fdirty = 1<<0,
	Fsent = 1<<1,
	Eclosing = 1<<0,
};
static uvlong poolsz = Poolsz;
typedef struct Page Page;
struct Page{
	int em;
	char flags;
	vlong age;
	usize addr;
	uchar buf[Pagesz];
	RWLock l;
	Page *lleft;
	Page *lright;
};
struct EM{
	int id;
	ssize ref;
	char flags;
	int fd[1+Nwriters];
	int infd;
	char *path;
	RWLock l;
	Page *cur;
};
KHASH_MAP_INIT_INT64(phash, Page*)
static EM **etab;
static Page pused = {.lleft = &pused, .lright = &pused};
static ssize mpages, memfree, memreallyfree = Poolsz / Pagesz;
static Channel *wchan[Nwriters];
static uvlong emtc;
static khash_t(phash) *pmap;
static int nproc;

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

#define BADDR(p)	((p) & (Addrmask & ~Pmask))
#define POFF(p)		((p) & Pmask)
#define PADDR(p)	((p) & ~Pmask)
#define PLEA(p, a)	((p)->buf + POFF((a)))
#define PREAD(fd, page, off)	do{ \
	if(pread((fd), (page)->buf, sizeof((page)->buf), BADDR((off))) < 0) \
		warn("pread: %s\n", error()); \
}while(0)

/* FIXME: macro */
/* FIXME: tune when shit gets locked/unlocked? */
/* FIXME: reduce number of (w)locking */
/* FIXME: remove asserts */
/* FIXME: concurrent PAGE user access */
static Page *
PAGE(EM *em, ssize off)
{
	int rr, nr, ret;
	ssize a;
	Page *l, *r, *p;
	khiter_t k;
	if((off & ~Addrmask) != 0)
		sysfatal("em: shit! access out of bounds %#llx, max %#llx", off, Addrmask);
	a = off & ~Pmask | em->id;
	if(em->cur == nil || (p = em->cur)->addr != a){
		p = nil;
		rr = nr = 0;
		k = kh_get(phash, pmap, a);
		if(k != kh_end(pmap)){
			p = kh_val(pmap, k);
			if(p == nil)	/* on disk */
				rr = 1;
			else if(p->addr != a){	/* reallocated */
				rr = 1;
				p = nil;
			}
			emtc++;
		}else
			nr = 1;
		if(p == nil){
			(em)->cur = nil;
			rlock(&pused.l);
			(p) = pused.lleft;
			rlock(&(p)->l);
			runlock(&pused.l);
			vlong m = 0;
			while(((p)->flags & Fdirty) != 0 || p == &pused){
				assert((p)->addr != a);
				l = (p)->lleft;
				runlock(&(p)->l);
				rlock(&l->l);
				(p) = l;
				m++;
			}
			runlock(&(p)->l);
			wlock(&(p)->l);
			if((p)->addr != 0){
				k = kh_get(phash, pmap, (p)->addr);
				kh_val(pmap, k) = nil;
			}
			(p)->addr = a;
			(p)->em = (em)->id;
			if((p)->age == 0)
				memfree--;
			(p)->age = 0;
			wunlock(&(p)->l);
			if(m > 32)
				warn("stalled, %lld iterations age %#llx emtc %#llx\n", m, (p)->age, emtc);
		}
		k = kh_put(phash, pmap, a, &ret);
		kh_val(pmap, k) = (p);
		(em)->cur = (p);
		assert((p) != &pused);
		if((p)->age == 0 || emtc - (p)->age >= Pelder){
			l = (p)->lright;
			r = (p)->lleft;
			if(l == &pused){
				l = (p)->lleft;
				r = (p)->lright;
			}
			wlock(&r->l);
			wlock(&l->l);
			PUNLINK((p));
			wunlock(&l->l);          
			wunlock(&r->l);
			l = &pused;
			wlock(&l->l);
			r = l->lright;
			wlock(&r->l);
			PLINK(l, (p));
			wunlock(&l->l);
			wunlock(&r->l);
			(p)->age = emtc;
		}
		if(rr){
			if((em)->fd[0] >= 0)
				PREAD((em)->fd[0], (p), (off));
		}else if(nr && (em)->infd >= 0)
			PREAD((em)->infd, (p), (off));
	}
	return p;
}

static int
flush(EM *em, Page *p, int i)
{
	DPRINT(Debugextmem, "flush %#p:%#p:%#p\n", em, p, p->buf);
	if(pwrite(em->fd[i], p->buf, sizeof p->buf, BADDR(p->addr)) != sizeof p->buf)
		warn("flush: write %d: %s\n", em->fd[i], error());
	return 0;
}

static void
cacheproc(void *ip)
{
	int id;
	EM *em;
	Page *p;
	Channel *c;

	threadsetname("cacheproc");
	id = (intptr)ip;
	c = wchan[id];
	id++;
	for(;;){
		if((p = recvp(c)) == nil)
			break;
		em = etab[p->em];
		em->ref--;
		rlock(&p->l);
		if((em->flags & Eclosing) != 0)
			emclose(em);
		else{
			if(em->fd[0] < 0){
				wlock(&em->l);
				wunlock(&em->l);
			}
			rlock(&em->l);
			if(flush(em, p, id) < 0)
				warn("flush: %s\n", error());
			runlock(&em->l);
		}
		p->flags &= ~(Fdirty | Fsent);
		runlock(&p->l);
	}
}

static void
feedpages(void)
{
	ssize n;
	Page *l, *r, *pl, *p, *pe;

	n = memreallyfree < Chunksz ? memreallyfree : Chunksz;
	pl = emalloc(n * sizeof *pl);
	memreallyfree -= n;
	for(p=pl, pe=p+n; p<pe; p++){
		p->lright = p + 1;
		p->lleft = p - 1;
	}
	p--;
	r = &pused;
	p->lright = r;
	wlock(&r->l);
	l = r->lleft;
	if(l != r)
		wlock(&l->l);
	pl->lleft = l;
	r->lleft = p;
	l->lright = pl;
	if(l != r)
		wunlock(&l->l);
	wunlock(&r->l);
	mpages += n;
	memfree += n;
}

static void
lruproc(void *)
{
	vlong n, on;
	uint i;
	EM *em;
	Page *p, *l;

	/* FIXME: there's waiting for stuff */
	threadsetname("lruproc");
	for(i=on=0;;){
		if(memfree < Chunksz/2 && memreallyfree > 0)
			feedpages();
		if(memfree < Chunksz/4)
			on = 1;
		p = pused.lleft;
		for(n=0;; n++){
			l = p->lleft;
			if((p->flags & (Fdirty|Fsent)) == Fdirty && on){
				em = etab[p->em];
				if(em->fd[0] < 0){
					if(em->path == nil)
						em->path = sysmktmp();	/* can be stupidly expensive */
					if((em->fd[0] = create(em->path, ORDWR, 0640)) < 0){
						warn("create: %s\n", error());
						continue;
					}
					/* FIXME: cleaner way? */
					for(i=1; i<nelem(em->fd); i++)
						if((em->fd[i] = dup(em->fd[0], -1)) < 0)
							sysfatal("what");
				}
				em->ref++;
				while(nbsendp(wchan[i++ % nelem(wchan)], p) == 0)
					;
				p->flags |= Fsent;
			}
			if(l == &pused || n >= mpages)
				break;
			p = l;
		}
	}
}

void
emw64(EM *em, vlong off, u64int v)
{
	uchar *u;
	Page *p;

	off *= 8;
	p = PAGE(em, off);
	//PAGE(em, off, p);
	u = PLEA(p, off);
	PBIT64(u, v);
	p->flags |= Fdirty;
	DPRINT(Debugextmem, "w64 %#p[%#zx]:%#p[%#zx] (%#zx) → %llx", em, PADDR(off), p, POFF(off), off/8, v);
}

u64int
emr64(EM *em, vlong off)
{
	uchar *u;
	Page *p;

	off *= 8;
	p = PAGE(em, off);
	//PAGE(em, off, p);
	u = PLEA(p, off);
	DPRINT(Debugextmem, "r64 %#p[%#zx]:%#p[%#zx] (%#zx) ← %llx", em, PADDR(off), p, POFF(off), off/8, GBIT64(u));
	return GBIT64(u);
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
	int i;

	if(em == nil)
		return;
	em->flags |= Eclosing;
	if(em->ref > 0)
		return;
	if(em->infd >= 0)
		close(em->infd);
	if(em->fd[0] >= 0){
		for(i=0; i<nelem(em->fd); i++)
			close(em->fd[i]);
	}
	if(em->path != nil){
		sysremove(em->path);
		free(em->path);
	}
	etab[em->id] = nil;
	free(em);
}

EM *
emopen(char *path, int flags)
{
	int i;
	EM *em;

	em = emalloc(sizeof *em);
	dypush(etab, em);
	em->id = dylen(etab) - 1;
	em->flags = flags;
	for(i=0; i<nelem(em->fd); i++)
		em->fd[i] = -1;
	em->infd = -1;
	if(path != nil){
		if((em->infd = open(path, OREAD)) < 0){
			free(em);
			return nil;
		}
	}
	if(nproc == 0){
		pmap = kh_init(phash);
		for(i=0; i<nelem(wchan); i++)
			if((wchan[i] = chancreate(sizeof(Page*), 16)) == nil)	/* FIXME: tune */
				sysfatal("chancreate: %s", error());
		feedpages();
		if(proccreate(lruproc, nil, mainstacksize) < 0)
			sysfatal("proccreate: %s", error());
		for(i=0; i<nelem(wchan); i++){
			if(proccreate(cacheproc, (void*)i, mainstacksize) < 0)
				sysfatal("proccreate: %s", error());
			nproc++;
		}
	}
	return em;
}

static void
cleanup(void)
{
	EM **em;

	/* FIXME: interrupt all threads */
	for(em=etab; em<etab+dylen(etab); em++)
		emclose(*em);
}

void
initem(void)
{
	if(multiplier < 28 || multiplier > 63)
		sysfatal("invalid memory size");
	poolsz = 1ULL << multiplier;
	memreallyfree = poolsz / Pagesz;
	atexit(cleanup);
}
