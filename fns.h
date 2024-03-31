void	warn(char*, ...);
void	dprint(int, char*, ...);
void	vawarn(char*, va_list);
void	vadebug(char*, char*, va_list);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
#define panic(x) {warn((x)); abort();}
vlong	μsec(void);

Quad	Qd(Vertex, Vector);
double	qΔx(Quad);
double	qΔy(Quad);
int	ptinquad(Vertex, Quad);
Quad	insetquad(Quad, int);
Quad	quadaddpt2(Quad, Vector);
Quad	quadsubpt2(Quad, Vector);
Vertex	floorpt2(Vertex);
int	eqpt2(Point2, Point2);

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int	errstr(char*, uint);

int	zoomgraph(Graph*, int);

void	expandnode(Graph*, Node*);
void	retractnode(Graph*, Node*);

Node*	getnode(Graph*, ssize);
Node*	getactivenode(Graph*, Node*);
Node*	str2node(Graph*, char*);
Edge*	getedge(Graph*, ssize);
int	ischild(Graph*, ssize, ssize);
Node*	touchnode(Graph*g, ssize id, ssize pid, ssize idx, int w);
Node*	pushnode(Graph*, ssize, ssize, ssize, int);
Node*	touchnode(Graph*g, ssize id, ssize pid, ssize idx, int w);
Node*	pushsibling(Graph*, ssize, Node*, ssize, int);
Node*	pushnamednode(Graph*, char*);
Node*	pushchild(Graph*, ssize, Node*, ssize, int);
Edge*	pushnamededge(Graph*, char*, char*, int, int);
Edge*	pushedge(Graph*, Node*, Node*, int, int);
void	poptree(Graph*, Node*);
void	printgraph(Graph*);

void	clearmeta(Graph*);
void	cleargraph(Graph*);
void	nukegraph(Graph*);
void	pushgraph(Graph);
void	lockgraphs(int);
void	unlockgraphs(int);
Graph	initgraph(int);

void	initlayout(void);
int	newlayout(Graph*, int);
int	resetlayout(Graph*);
void	stoplayout(Graph*);
void	runlayout(Graph*);
int	updatelayout(Graph*);

int	initcmd(void);

void	initfs(void);
void	freefs(File*);
int	loadfs(char*, int);

void	sysinit(void);
void	sysquit(void);

void	initsysdraw(void);
void	drawui(void);
void	redraw(void);
void	reqdraw(int);
int	scrobj(Vertex);
void	showobj(Obj*);
void	stopdrawclock(void);
void	startdrawclock(void);

void	initrend(void);
int	renderlayout(Graph*);
int	rerender(int);

Obj	mouseselect(Vertex);
int	panview(Vector);
int	zoomview(Vector);
void	initsysui(void);
void	initui(void);
void	evloop(void);
void	errmsg(char*, ...);
void	resetui(int);
int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
void	quit(void);

/* you gotta wonder what will come next */
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
