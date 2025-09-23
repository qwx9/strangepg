void	warn(char*, ...);
void	logmsg(char*);
void	logerr(char*);
char*	va(char*, ...);
void	dprintnl(int, char*, ...);
void	dprint(int, char*, ...);
void	vawarn(char*, va_list);
void	vadebug(char*, char*, va_list);
void	initlog(void);
char*	error(void);
char*	smprint(char*, ...);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
void	panic(char*, ...);
void	errmsg(char*, ...);
int	errstr(char*, uint);
vlong	μsec(void);
void	lsleep(vlong);
void	sysquit(void);
void	quit(void);
void	initsys(void);

void	initrand(void);
void	xsrand(u64int);
float	xfrand(void);
u32int	xlrand(void);
u32int	xnrand(u32int);

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

void	newthread(void (*)(void*), void (*)(void*), void*, void*, char*, uint);
void*	threadstore(void*);

#define DPRINT(x,...)	do{ \
	if((debug&(x)) != 0) \
		dprintnl((x), __VA_ARGS__); \
}while(0)

/* single line, multiple print calls */
#define DPRINTS(x,...)	do{ \
	if((debug&(x)) != 0) \
		dprint((x), __VA_ARGS__); \
}while(0)
#define DPRINTN(x,...)	do{ \
	if((debug&(x)) != 0) \
		warn(__VA_ARGS__); \
}while(0)

/* FIXME: get something better */
#define	CLK0(c)	if((debug & Debugperf) != 0){ \
	(c).t = μsec(); \
	if((c).t0 == 0) \
		(c).t0 = (c).t; \
	(c).n++; \
}
#define	CLK1(c)	if((debug & Debugperf) != 0){ \
	vlong _t = μsec(); \
	(c).s += (_t - (c).t) / 1000000.0; \
	(c).n++; \
	if(_t - (c).t0 >= PerfΔt){ \
		(c).t0 = _t; \
		_t = (vlong)(((c).s / (c).n) * 1000000.0); \
		warn("[perf] %s: avg %lld μs over %lld samples\n", \
			(c).lab, _t, (c).n); \
	} \
}

#define TIME(fn,lab,t)	if(debug & Debugperf){ \
	vlong __t; \
	__t = μsec(); \
	warn("[perf] " fn ": " lab ": %.2f ms\n", (__t - (t)) / 1000.0); \
	(t) = __t; \
}
