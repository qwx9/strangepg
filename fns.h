void	warn(char*, ...);
void	dprint(char*, ...);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
int	parseargs(int, char **);
#define panic(x) {warn((x)); abort();}

void	vecnuke(Vec*);
void*	vecget(Vec*, usize);
void*	vecpoptail(Vec*);
usize	vecindexof(Vec*, void*);
void*	vecpush(Vec*, void*, usize*);
void*	vecp(Vec*, usize);
void	vecresize(Vec*, usize);
Vec	vec(usize, usize);

Quad	Qd(Vertex, Vector);
double	qΔx(Quad);
double	qΔy(Quad);
int	ptinquad(Vertex, Quad);
Quad	insetquad(Quad, int);
Quad	quadaddpt2(Quad, Vector);
Quad	quadsubpt2(Quad, Vector);
Vertex	floorpt2(Vertex);
int	eqpt2(Point2, Point2);

#define idmap() kh_init(id)
void	idnuke(Htab*);
int	idput(Htab*, char*, usize);
int	idget(Htab*, char*, usize*);
void	iddump(Htab*);

void	init(void);
void	run(void);
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int	errstr(char*, uint);

Node*	id2n(Graph*, char*);
Node*	e2n(Graph*, usize);
void	nukegraph(Graph*);
Graph*	initgraph(void);
int	addnode(Graph*, char*, char*);
int	addedge(Graph*, char*, char*, int, int, char*, double);
void	rendernew(void);

void	initlayout(void);
int	dolayout(Graph*, int);
int runlayout(Graph*);
int	earlyexit(void);

int	pushcmd(int, usize, int, uchar*);
int	flushcmd(void);

#define	PBIT64(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;(p)[2]=(v)>>16;(p)[3]=(v)>>24;\
	(p)[4]=(v)>>32;(p)[5]=(v)>>40;(p)[6]=(v)>>48;(p)[7]=(v)>>56;}while(0)
#define	GBIT64(p)	((u32int)(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8)|\
	(((uchar*)(p))[2]<<16)|(((uchar*)(p))[3]<<24)) |\
	((uvlong)(((uchar*)(p))[4]|(((uchar*)(p))[5]<<8)|\
	(((uchar*)(p))[6]<<16)|(((uchar*)(p))[7]<<24)) << 32))

void	initfs(void);
Graph*	loadfs(int, char*);
int	openfs(File*, char*, int);
File*	graphopenfs(char*, int, Graph*);
int	writefs(File*, void*, int);
vlong	seekfs(File*, vlong);
void	closefs(File*);
u8int	get8(File*);
u16int	get16(File*);
u32int	get32(File*);
u64int	get64(File*);
int	put64(File*, u64int);

void	sysinit(void);
int	sysopen(File*, int);
int	syswrite(File*, void*, int);
int	sysread(File*, void*, int);
int	syswstatlen(File*, vlong);
vlong	sysseek(File*, vlong);
void	sysflush(File*);
void	sysclose(File*);

// FIXME: initourdraw?
int	initdrw(void);	/* plan9 already has initdraw(2) */
int	resetdraw(void);
int	updatedraw(void);
int	redraw(void);
int shallowdraw(void);
void	triggerdraw(ulong);
void	triggerlayout(Graph*);

void	initrend(void);
int	render(Graph*);

int	panview(Vertex);
void	initui(void);
int	evloop(void);
void	errmsg(char*, ...);
void	resetui(int);
int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
char*	shitprint(int, void*);
