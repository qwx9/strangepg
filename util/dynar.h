enum{
	Initsz = 4,
};

/* private */
#define	_dylen(a)		(*((ssize *)(a)-1))
#define	_dysz(a)		(*((ssize *)(a)-2))
#define	_dyalign(a)		(sizeof(*a) + 2 * sizeof(ssize) + 7 & ~7)
#define	_dyptr(a)		((void *)((uchar *)(a) - _dyalign(a)))
#define	_dystart(a,p)	((void *)((uchar *)(p) + _dyalign(a)))
#define	_dyalloc(a,n)	do{ \
							void *_p = emalloc(_dyalign(a) + (n) * sizeof(*a)); \
							(a) = _dystart((a), _p); \
							_dysz(a) = (n); \
						}while(0)
#define	_dyrealloc(a,n)	do{ \
							if((a) == nil) \
								_dyalloc((a), (n)); \
							else{ \
								void *_p = erealloc(_dyptr(a), \
									_dyalign(a) + (n) * sizeof(*a), \
									_dyalign(a) + _dysz(a) * sizeof(*a)); \
								(a) = _dystart((a), _p); \
								_dysz(a) = (n); \
							} \
						}while(0)
#define	_dygrow(a,n)	do{ \
							ssize _n = (a) == nil ? 0 : _dysz(a); \
							if(_n < (n)){ \
								if(_n == 0) \
									_n = Initsz; \
								while(_n <= (n)) \
									_n *= 2; \
								_dyrealloc((a), _n); \
							} \
						}while(0)

/* public */
#define dylen(a)		((a) == nil ? 0 : _dylen(a))
#define	dyfree(a)		do{ \
							if((a) != nil){ \
								free(_dyptr(a)); \
								(a) = nil; \
							} \
						}while(0)
#define	dychecksz(a,n)	do{ \
							if((a) == nil || _dysz(a) <= (n)) \
								_dyrealloc((a), (n) + 1); \
						}while(0)
#define	dyresize(a,n)	do{ \
							if((a) == nil || _dysz(a) != (n)) \
								_dyrealloc((a), (n) + 1); \
							_dylen(a) = (n); \
						}while(0)
#define	dygrow(a,i)		do{ \
							_dygrow((a), (i) + 1); \
							if(dylen(a) < (i) + 1) \
								_dylen(a) = (i) + 1; \
						}while(0)
/* bleh */
#define	dypushat(a,i,v)	do{ \
							dygrow((a), (i)); \
							(a)[i] = v; \
						}while(0)
#define dypush(a,v)		do{ \
							ssize _m = dylen(a); \
							_dygrow((a), _m + 1); \
							(a)[_m] = (v); \
							_dylen(a)++; \
						}while(0)
