/* ideas from klib and stb */
typedef struct Dyhdr Dyhdr;
struct Dyhdr{
	void *buf;
	usize len;
	usize sz;
};

#define dyhdr(a)	((Dyhdr*)(a) - 1)
#define dyfree(a)	(free(dyhdr(a)))
#define dylen(a)	((a) == nil ? 0 : dyhdr(a)->len)
#define dysetsz(a,h,n)	((a) = (void*)((h)+1), h->sz = (n), (a))
#define dynew(a,h)	((h) = emalloc(4 * sizeof(*(a)) + sizeof(Dyhdr)), dysetsz((a), (h), 4))
#define dyextend(a,h,n)	((h) = erealloc((h), \
	2*n + sizeof(Dyhdr), n + sizeof(Dyhdr)), \
	dysetsz((a), (h), n / sizeof(*(a)) * 2))
#define dychecksz(a,h)	((a) == nil ? dynew((a),(h)) : \
	(h)->sz == (h)->len ? dyextend((a), (h), sizeof(*(a)) * h->sz) : \
	(a))
#define dypush(a,v)	{Dyhdr*__h = dyhdr(a); (a) = dychecksz(a,__h); (a)[dyhdr(a)->len++] = v;}
#define dypop(a)	(assert((a) != nil && dyhdr(a)->len > 0), (a)[--dyhdr(a)->len])
