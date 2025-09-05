#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "em.h"
#include "../lib/khashl.h"

/* based on: Yang et al, FIFO Queues are all you need for cache eviction, 2023
 * with growable pool, single hash table for all 3 queues and freq packed into
 * values */

int multiplier = 16;

KHASHL_MAP_INIT(KH_LOCAL, Pool, p, uvlong, uvlong, kh_hash_uint64, kh_eq_generic)

typedef	uchar* Page;

enum{
	Pagesz = 1<<16,
	EMsz = (1LL<<32) >> 16,
	PFagesh = 58,
	PFagesz = 1<<2,
	PFg = 1ULL<<60,
	PFm = 1ULL<<61,
	PFqmask = 3ULL<<60,
	PFmask = 15ULL<<58,
	PFdirty = 1ULL<<62,
	PFcached = 1ULL<<63,
	Nope = ~0ULL,
};
static Pool *pool;
static Page *pages;
static uvlong *S, *M, *G;
static int msz, ssz, stail, shead, mtail, mhead, gtail, ghead;
static uvlong top, freep, nfree, nalloc = 16;
static int nextem, swapfd = -1;
static EM emstr = -1;
static vlong emstroff;

/* FIXME: writer procs on eviction from m or s and dispatch */

static inline void
feedpages(void)
{
	int n, np, psz;
	uchar *d;
	Page *p, *e;

	if((np = nalloc) > nfree)
		np = nfree;
	nalloc *= 2;
	d = emalloc(np * (psz = Pagesz * sizeof *d));
	n = dylen(pages);
	DPRINT(Debugem, "feedpages: freep %lld top %lld n %d np %d", freep, top, n, np);
	dygrow(pages, n + np - 1);
	for(p=pages+n, e=p+np; p<e; p++, d+=psz)
		*p = d;
	dyresize(M, n + np);
	dyresize(S, 0.1 * (n + np));
	top += np;
	nfree -= np;
}

static inline void
qinsertg(uvlong va)
{
	uvlong i, h;

	if(ghead == gtail){ /* FIXME: if size > 0 and fragmentation... */
		h = gtail;
		do{
			if((i = G[h++]) != Nope){
				G[ghead++] = i;
				if(ghead >= ssz)
					ghead = 0;
			}
			if(h >= ssz)
				h = 0;
		}while(h != gtail);
	}
	G[ghead++] = va;
	if(ghead >= ssz)
		ghead = 0;
}

static inline void
qinsertm(uvlong i)
{
	M[mhead++] = i;
	if(mhead >= msz)
		mhead = 0;
}

static inline void
qinserts(uvlong va, uvlong i)
{
	S[shead++] = i;
	if(shead >= ssz)
		shead = 0;
	qinsertg(va);
}

static inline void
qevictm(void)
{
	uvlong i;

	while(mtail != mhead){
		i = M[mtail++];
		if(mtail >= msz)
			mtail = 0;
		if((i & PFagesz-1ULL<<PFagesh) != 0)
			qinsertm(i - (1ULL << PFagesh));
		else
			break;
	}
}

static inline void
qevicts(void)
{
	uvlong i;

	while(stail != shead){
		i = S[stail++];
		if(stail >= ssz)
			stail = 0;
		if(i & PFagesz-1ULL<<PFagesh > 1ULL << PFagesh){
			qinsertm(i);
			if(mhead == mtail)
				qevictm();
		}else{
			qinsertg(i);	/* FIXME: not va! maybe just have another i->va ht? */
			break;
		}
	}
}

static inline void
checkcache(void)
{
	while(shead == stail && mhead == mtail){	/* while cache is full */
		if(shead == stail)
			qevicts();
		else
			qevictm();
	}
}

/* so long as the value is in the hash table, it's in one of the 3 queues */
static inline uvlong
popg(void)
{
	uvlong va, i, t, h;
	khint_t k;

	checkcache();
	for(t=gtail, h=ghead; t!=h; t++){
		if(t >= ssz)
			t = 0;
		if((va = G[t]) != Nope){
			k = p_get(pool, va);
			assert(k != kh_end(pool));
			i = kh_val(pool, k);
			if((i & PFmask) != PFg){	/* picked up earlier from qread (hit) */
				G[t] = Nope;
				continue;
			}
			p_del(pool, k);
			G[t] = Nope;
			gtail = t + 1 >= ssz ? 0 : t + 1;
			return i;
		}
	}
	abort();
}

static inline uvlong
getpage(void)
{
	if(freep < top)
		return freep++;
	else if(nfree == 0)
		return popg();
	else{
		feedpages();
		return freep++;
	}
}

static inline ssize
qread(EM em, vlong addr, int wr)
{
	int abs;
	uvlong i, va;
	khint_t k;

	va = addr & ~(PFagesz-1) | em;
	k = p_get(pool, va);
	if(k != kh_end(pool)){
		i = kh_val(pool, k);
		if((i & PFqmask) != PFg){
			if((i & PFagesz-1ULL << PFagesh) != PFagesz-1ULL << PFagesh)
				kh_val(pool, k) = i + (1ULL<<PFagesh);
			return i & PFm ? M[i & ~PFmask] : S[i & ~PFmask];
		}
		i &= ~PFmask;	/* removed from G in popg */
		if(wr)
			i |= PFdirty;
		checkcache();
		kh_val(pool, k) = i | PFm;
		qinsertm(i);
	}else{
		checkcache();
		i = getpage();
		qinserts(va, i);
		k = p_put(pool, va, &abs);
		assert(abs);
		kh_val(pool, k) = i;
	}
	return i;
}

static inline uchar *
lookup(EM em, uvlong addr, int wr)
{
	uvlong i;

	i = qread(em, addr, wr);
	return pages[i] + (addr & Pagesz-1);
}

static inline int
getoverlap(vlong off, int sz)
{
	vlong end;

	end = off + Pagesz & ~(Pagesz-1);
	if(off + sz > end)
		return off + sz - end;
	else
		return 0;
}

static inline vlong
jumpoverlap(vlong off, int sz)
{
	vlong end;

	end = off + Pagesz & ~(Pagesz-1);
	if(off + sz >= end)
		off = end;
	return off;
}

/* FIXME: alignment/cross-boundary access:
 * we want all bytes in u to be in the page, right now it's assumed */
u64int
emr64(EM em, vlong i)
{
	return *(u64int *)lookup(em, sizeof(u64int) * i, 0);
}
void
emw64(EM em, vlong i, u64int v)
{
	*(u64int *)lookup(em, sizeof(u64int) * i, 1) = v;
}
u32int
emr32(EM em, vlong i)
{
	return *(u32int *)lookup(em, sizeof(u32int) * i, 0);
}
void
emw32(EM em, vlong i, u32int v)
{
	*(u32int *)lookup(em, sizeof(u32int) * i, 1) = v;
}
u16int
emr16(EM em, vlong i)
{
	return *(u16int *)lookup(em, sizeof(u16int) * i, 0);
}
void
emw16(EM em, vlong i, u16int v)
{
	*(u16int *)lookup(em, sizeof(u16int) * i, 1) = v;
}
u8int
emr8(EM em, vlong i)
{
	return *lookup(em, i, 0);
}
void
emw8(EM em, vlong i, u8int v)
{
	*lookup(em, i, 1) = v;
}

void
emreadi(EM em, vlong i, void *p, int sz)
{
	int o;
	uchar *u;

	i *= sz;
	o = getoverlap(i, sz);
	u = lookup(em, i, 0);
	memcpy(p, u, sz - o);
	if(o > 0){
		u = lookup(em, i+o, 0);
		memcpy(p, u, o);
	}
}

void
emwritei(EM em, vlong i, void *p, int sz)
{
	int o;
	uchar *u;

	i *= sz;
	o = getoverlap(i, sz);
	u = lookup(em, i, 1);
	memcpy(u, p, sz - o);
	if(o > 0){
		u = lookup(em, i+o, 1);
		memcpy(u, p, o);
	}
}

char *
emgetstr(EMstring off)
{
	uchar *u;

	if(emstr == -1){
		werrstr("no strings to hand");
		return nil;
	}else if(off < 0 || off >= emstroff){
		werrstr("invalid offset");
		return nil;
	}
	u = lookup(emstr, off, 0);
	return (char *)u;
}

EMstring
emputstr(char *s, int len)
{
	uchar *u;
	vlong off;

	if(emstr == -1)
		emstr = emopen(nil);
	if(++len >= Pagesz)
		sysfatal("emputstr: string too long: %d > %d", len, Pagesz);
	off = jumpoverlap(emstroff, len);
	u = lookup(emstr, off, 1);
	memcpy(u, s, len);
	emstroff = off + len;
	return off;
}

void
emclose(EM em)
{
	USED(em);
}

static void
allsystemsgo(void)
{
	M = emalloc((msz = nfree) * sizeof *M);
	S = emalloc((ssz = msz / 10) * sizeof *M);
	G = emalloc(ssz * sizeof *G);	/* FIXME: ? */
	if((swapfd = sysmktmp()) < 0)
		sysfatal("initem: %s", error());
	/*
	for(i=0; i<nelem(wchan); i++)
		if((wchan[i] = chancreate(sizeof(void*), Wchansz)) == nil)
			sysfatal("chancreate: %s", error());
	for(i=0; i<nelem(wchan); i++)
		newthread(writer, nil, wchan[i], nil, "emwriter", mainstacksize);
	*/
}

/* FIXME: no reuse */
static EM
openaccount(void)
{
	EM em;

	if((em = nextem++) >= EMsz)
		sysfatal("emopen: too many buffers in reuse-less implementation");
	return em;
}

EM
emopen(char *path/*, int sequential*/)
{
	EM em;

	USED(path);
	em = openaccount();
	if(swapfd < 0)
		allsystemsgo();
	return em;
}

void
initem(void)
{
	if(multiplier < 6 || multiplier > 22)
		sysfatal("invalid memory ceiling: %dMB, multiplier must be [6-22]",
			1 << multiplier >> 4);
	nfree = 1 << multiplier;
	if((pool = p_init()) == nil)
		sysfatal("initem: %s", error());
}
