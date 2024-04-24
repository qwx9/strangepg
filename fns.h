void	warn(char*, ...);
void	dprint(int, char*, ...);
void	vawarn(char*, va_list);
void	vadebug(char*, char*, va_list);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
#define panic(x) {warn((x)); abort();}
void	errmsg(char*, ...);
int	errstr(char*, uint);
vlong	μsec(void);
void	sysquit(void);
void	quit(void);
void	sysinit(void);

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

void	newthread(void (*)(void*), void (*)(void*), void*, void*, char*, uint);
void*	threadstore(void*);

#define DPRINT(x,...)	do{ \
	if((debug&(x)) != 0) \
		dprint((x), __VA_ARGS__); \
}while(0)

/* FIXME: get something better */
#define	CLK0(c)	if((debug & Debugperf) != 0){ \
	if((c).t0 == 0) \
		(c).t0 = (c).t = μsec(); \
	else \
		(c).t = μsec(); \
}
#define	CLK1(c)	if((debug & Debugperf) != 0){ \
	vlong _t = μsec(); \
	(c).Δt += _t - (c).t; \
	(c).nsamp++; \
	_t -= (c).t0; \
	if(_t >= PerfΔt){ \
		_t = (double)(c).Δt / ((c).nsamp * _t) * 1000000; \
		warn("[perf] %s: normavg %lld μs n %d\n", \
			(c).lab, _t, (c).nsamp); \
		(c).nsamp = 0; \
		(c).Δt = 0; \
		(c).t0 += PerfΔt; \
	} \
}
