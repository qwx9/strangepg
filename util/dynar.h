/* ideas from klib and stb */
typedef struct Dyhdr Dyhdr;
struct Dyhdr{
	ssize sz;
	ssize len;
	char align[];
};

/* FIXME: most of this shit doesn't really need to be a macro at all,
 * at least use inline */
/* FIXME: could we do just ssize[2] instead of a struct? 
 * so -1 would be len, -2 would be sz?
 * or align to 8 or 16 bytes? + 7 & ~7 */

/* FIXME: non portable */
#define alignof(x)	offsetof(struct{ char __u; __typeof__(x) __v; }, __v)

/* private, do not use directly */
#define	dyalign(a)	((uchar*)(a) - alignof(a))
#define	dyhsz(a)	(sizeof(Dyhdr) + alignof(a))

#define dyhdr(a)	(((Dyhdr*)(dyalign(a))) - 1)
#define dysz(a)	((a) == nil ? 0 : dyhdr(a)->sz)

/* FIXME: UGH */
#define dysetsz(a,h,n)	do{ \
	(a) = (__typeof__(a))((uchar*)(h)+dyhsz(a)); \
	(h)->sz = (n); \
}while(0)
#define dynew(a,h,n)	do{ \
	(h) = emalloc( \
		(n) * sizeof(*(a)) + sizeof(Dyhdr) + alignof(a)); \
	dysetsz((a), (h), (n)); \
}while(0)
#define dyextend(a,h)	do{ \
	(h) = erealloc((h), \
		2 * (h)->sz * sizeof(*(a)) + dyhsz(a), \
		(h)->sz * sizeof(*(a)) + dyhsz(a)); \
	dysetsz((a), (h), 2*(h)->sz); \
}while(0)

/* public */
#define dylen(a)	((a) == nil ? 0 : dyhdr(a)->len)
#define dyavail(a)	((a) == nil ? 0 : dyhdr(a)->sz - dyhdr(a)->len)

#define dyfree(a)	do{ \
	if((a) != nil){ \
		free(dyhdr(a)); \
		(a) = nil; \
	} \
}while(0)
#define dypush(a,v)	do{ \
	Dyhdr *__h; \
	if((a) == nil) \
		dynew((a), __h, 4); \
	else{ \
		__h = dyhdr(a); \
		if(__h->sz <= __h->len) \
			dyextend((a), __h); \
	} \
	(a)[__h->len++] = (v); \
}while(0)
#define dypop(a)	do{assert((a) != nil && dyhdr(a)->len > 0); --dyhdr(a)->len;}while(0)
#define dyclear(a)	do{if((a) != nil){ memset((a), 0, dyhdr(a)->len * sizeof(*(a))); dyhdr(a)->len = 0;}}while(0)
#define dyreset(a)	do{if((a) != nil){ dyhdr(a)->len = 0;}}while(0)
#define dyprealloc(a,n)	do{Dyhdr*__h; dynew((a),__h,(n)); dyhdr(a)->len = (n); }while(0)

#define dygrow(a,i)	do{ \
	if((a) == nil){ \
		dyprealloc(a, (i)); \
		break; \
	} \
	Dyhdr*___h = dyhdr(a); \
	while(___h->sz <= (i)) \
		dyextend((a), ___h);	\
}while(0)
#define dyresize(a,i) do{ \
	Dyhdr*_h; \
	if((a) == nil || (_h = dyhdr(a))->sz < (i)){ \
		dygrow(a, (i)); \
		_h = dyhdr(a); \
		_h->len = (i); \
	}else if(_h->len < (i)) \
		_h->len = (i); \
	}while(0)
#define dyshrink(a,n) do{ \
	Dyhdr*_h; \
	if((a) != nil){ \
		_h = dyhdr(a); \
		if((n) <= _h->sz / 2){ \
			_h = erealloc(_h, \
				(n) * sizeof(*(a)) + dyhsz(a), \
				_h->sz * sizeof(*(a)) + dyhsz(a)); \
			dysetsz(a,_h,n); \
		} \
		if(_h->len > (n)) \
			_h->len = (n); \
	} \
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
