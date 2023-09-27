/* ideas from klib and stb */
typedef struct Dyhdr Dyhdr;
struct Dyhdr{
	void *buf;
	usize len;
	usize sz;
};

/* private, do not use directly */
#define dyhdr(a)	((Dyhdr*)(a) - 1)
#define dysetsz(a,h,n)	((a) = (void*)((h)+1), h->sz = (n), (a))
#define dynew(a,h,n)	((h) = emalloc((n) * sizeof(*(a)) + sizeof(Dyhdr)), dysetsz((a), (h), (n)))
#define dyextend(a,h,n)	((h) = erealloc((h), \
	2*n + sizeof(Dyhdr), n + sizeof(Dyhdr)), \
	dysetsz((a), (h), n / sizeof(*(a)) * 2))
#define dychecksz(a,h)	((a) == nil ? dynew((a),(h),4) : \
	(h)->sz == (h)->len ? dyextend((a), (h), sizeof(*(a)) * h->sz) : \
	(a))

/* public */
#define dyfree(a)	(free(dyhdr(a)))
#define dylen(a)	((a) == nil ? 0 : dyhdr(a)->len)
#define dyavail(a)	((a) == nil ? 0 : dyhdr(a)->sz - dyhdr(a)->len)
#define dypush(a,v)	do{Dyhdr*__h = dyhdr(a); (a) = dychecksz((a),__h); (a)[dyhdr(a)->len++] = v;}while(0)

#define dypop(a)	do{assert((a) != nil && dyhdr(a)->len > 0); --dyhdr(a)->len;}while(0)

#define dyclear(a)	do{if((a) != nil){ memset((a), 0, dyhdr(a)->len * sizeof(*(a))); dyhdr(a)->len = 0;}while(0)
#define dyprealloc(a,n)	do{Dyhdr*__h; (a) = dynew((a),__h,(n)); dyhdr(a)->len = (n); }while(0)
