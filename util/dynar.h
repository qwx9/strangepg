/* ideas from klib and stb */
typedef struct Dyhdr Dyhdr;
struct Dyhdr{
	usize sz;
	usize len;
} __attribute__((packed));	/* alignment */

/* private, do not use directly */
#define dyhdr(a)	((Dyhdr*)(a) - 1)
#define dysetsz(a,h,n)	((a) = (void*)((h)+1), (h)->sz = (n), (a))
#define dynew(a,h,n)	((h) = emalloc((n) * sizeof(*(a)) + sizeof(Dyhdr)), dysetsz((a), (h), (n)))
#define dyextend(a,h,n)	((h) = erealloc((h), \
	2*(n) + sizeof(Dyhdr), (n) + sizeof(Dyhdr)), \
	dysetsz((a), (h), (n) / sizeof(*(a)) * 2))
#define dychecksz(a,h,i)	((a) == nil ? dynew((a),(h),4) : \
	(h)->sz <= (i) ? dyextend((a), (h), sizeof(*(a)) * (h)->sz) : \
	(a))

/* public */
#define dyfree(a)	do{ \
	if((a) != nil){ \
		free(dyhdr(a)); \
		(a) = nil; \
	}}while(0)
#define dylen(a)	((a) == nil ? 0 : dyhdr(a)->len)
#define dyavail(a)	((a) == nil ? 0 : dyhdr(a)->sz - dyhdr(a)->len)
#define dypush(a,v)	do{Dyhdr*__h = dyhdr(a); (a) = dychecksz((a),__h,__h->len); (a)[dyhdr(a)->len++] = (v);}while(0)
#define dypop(a)	do{assert((a) != nil && dyhdr(a)->len > 0); --dyhdr(a)->len;}while(0)
#define dyclear(a)	do{if((a) != nil){ memset((a), 0, dyhdr(a)->len * sizeof(*(a))); dyhdr(a)->len = 0;}}while(0)
#define dyreset(a)	do{if((a) != nil){ dyhdr(a)->len = 0;}}while(0)
#define dyprealloc(a,n)	do{Dyhdr*__h; (a) = dynew((a),__h,(n)); dyhdr(a)->len = (n); }while(0)

/* FIXME: starting to suck big time */
#define dygrow(a,i)	do{ \
	if((a) == nil){ \
		dyprealloc(a, (i)); \
		break; \
	} \
	Dyhdr*___h = dyhdr(a); \
	if(___h->sz <= (i)){ \
		(a) = dychecksz((a),___h,(i)); \
		while(___h->sz <= (i))	\
			(a) = dyextend((a), ___h, sizeof(*(a)) * ___h->sz);	\
	}}while(0)
#define dyresize(a,i) do{ \
	Dyhdr*_h = dyhdr(a); \
	if((a) == nil || _h->sz < (i)){ \
		dygrow(a, (i)); \
		_h = dyhdr(a); \
		_h->len = (i); \
	} \
	_h->len = (i); \
	}while(0)
#define dyinsert(a,i,v)	do{ \
	dyresize(a, (i)+1); \
	(a)[(i)] = (v); \
	}while(0)
#define dydelete(a,i)	do{ \
	ssize __Δ = dyhdr(a)->len - (i) - 1; \
	assert(__Δ >= 0); \
	if(__Δ > 0) \
		memmove((a)+(i), (a)+(i)+1, __Δ*sizeof(*(a))); \
	dypop((a)); \
	}while(0)
