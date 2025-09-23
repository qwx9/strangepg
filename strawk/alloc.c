#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef VERSION
#include "strpg.h"
#include "threads.h"
#endif
#include "awk.h"

enum{
	POOLSZ = 8192,
};
typedef struct Pool Pool;
struct Pool{
	uschar **pool;
	uschar *end;
	uschar *tail;
	size_t sz;
	size_t slot;
	size_t nslot;
	size_t last;	/* before release */
	#ifdef VERSION
	RWLock lock;
	#endif
};
static Pool frozen, temp;

static inline void clear(Pool *p)
{
	wlock(&p->lock);
	p->sz = POOLSZ;
	if(p->slot == 1)
		memset(p->pool[0], 0, p->tail - p->pool[0]);
	p->last = p->slot;
	p->slot = 0;
	p->tail = p->pool[p->slot++];
	p->end = p->tail + p->sz;
	wunlock(&p->lock);
}

static inline void init(Pool *p)
{
	if((p->pool = calloc(1, sizeof *p)) == NULL
	|| (p->pool[0] = calloc(1, POOLSZ)) == NULL)
		FATAL("calloc: out of memory");
	p->nslot = 1;
	initrwlock(&p->lock);
	clear(p);
}

static inline uschar *grow(Pool *p, size_t n)
{
	uschar *s;

	if(p->slot < p->nslot){
		s = p->pool[p->slot++];
		if(p->slot <= p->last)
			memset(s, 0, p->sz);
	}else{
		for(;; p->sz*=2){
			p->nslot++;
			if((p->pool = realloc(p->pool, p->nslot * sizeof *p->pool)) == NULL)
				FATAL("realloc: out of memory");
			if((s = calloc(1, p->sz)) == NULL)
				FATAL("calloc: out of memory");
			p->pool[p->slot++] = s;
			if(p->sz >= n)
				break;
		}
	}
	p->tail = s;
	p->end = s + p->sz;
	p->sz *= 2;
	return s;
}

static inline uschar *alloc(size_t n, int istemp)
{
	uschar *s;
	Pool *p;

	p = istemp ? &temp : &frozen;
	wlock(&p->lock);
	s = p->tail;
	if(s + n >= p->end)
		s = grow(p, n);
	assert(p->end - p->tail >= n);
	p->tail += n + 7 & ~7;
	wunlock(&p->lock);
	return s;
}

/* FIXME: no checks, no cleanup! avoid calling this with non-temp */
static inline void *resize(const void *s, size_t old, size_t n, int istemp)
{
	void *p;

	p = alloc(n, istemp);
	if(n > 0 && s != NULL){
		if(old > n)
			old = n;
		memmove(p, s, old);
	}
	return p;
}

void initpool(void)
{
	init(&frozen);
	init(&temp);
}

void cleanpool(void)
{
	clear(&temp);
}

char *tempstrdup(const char *s)
{
	size_t n;

	n = strlen(s);
	return resize(s, n+1, n+1, 1);
}

void *temprealloc(const void *p, size_t old, size_t n)
{
	return resize(p, old, n, 1);
}

void *tempalloc(size_t n)
{
	return alloc(n, 1);
}

char *pstrdup(const char *s)
{
	size_t n;

	n = strlen(s);
	return resize(s, n+1, n+1, 0);
}

void *prealloc(const void *p, size_t old, size_t n)
{
	return resize(p, old, n, 0);
}

void *palloc(size_t n)
{
	return alloc(n, 0);
}

char *defstrdup(const char *s)
{
	size_t n;

	n = strlen(s);
	return resize(s, n+1, n+1, compile_time == RUNNING || runnerup != NULL);
}

void *defrealloc(const void *p, size_t old, size_t n)
{
	return resize(p, old, n, compile_time == RUNNING || runnerup != NULL);
}

void *defalloc(size_t n)
{
	return alloc(n, compile_time == RUNNING || runnerup != NULL);
}

void dgrow(void **buf, int *size, void **pos, int want, int blocksz, const char *fn)
{
	size_t sz, n;
	intptr_t off;
	uschar *p;

	sz = *size;
	if(sz >= want)
		return;
	n = blocksz > 0 ? want % blocksz : 0;
	/* round up to next multiple of blocksz */
	if(n > 0)
		want += blocksz - n;
	p = *buf;
	off = pos != NULL ? *(uschar**)pos - p : 0;
	p = drealloc(p, off > 0 ? off : sz, want, fn);
	DPRINTF("%p %zu -> %zu\n", (void *)p, sz, n);
	*buf = p;
	if (pos)
		*pos = p + off;
}

void *drealloc(void *s, size_t old, size_t new, const char *fn)
{
	void *p;

	DPRINTF("drealloc %p %zu to %zu in %s ", s, old, new, fn);
	if((p = realloc(s, new)) == NULL)
		FATAL("realloc: out of memory");
	if(new > old)
		memset((uschar *)p + old, 0, new - old);
	DPRINTF("-> %p\n", p);
	return p;
}

void *dmalloc(size_t n, const char *fn)
{
	void *p;

	DPRINTF("dmalloc %zu in %s ", n, fn);
	if((p = calloc(1, n)) == NULL)
		FATAL("calloc: out of memory");
	DPRINTF("-> %p\n", p);
	return p;
}

char *dstrdup(const char *s, const char *fn)
{
	void *p;

	DPRINTF("dstrdup %s in %s ", s, fn);
	if((p = strdup(s)) == NULL)
		FATAL("strdup: out of memory");
	DPRINTF("-> %p\n", p);
	return p;
}

void dfree(void *p, const char *fn)
{
	DPRINTF("dfree %p in %s\n", p, fn);
	free(p);
}
