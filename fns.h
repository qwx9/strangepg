void	warn(char*, ...);
void	dprint(char*, ...);
void	vawarn(char*, va_list);
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
void*	veccopy(Vec*, void*, usize*);
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

void	removenode(Graph*, Node*);
void	removeedge(Graph*, usize);
Node*	id2n(Graph*, char*);
Node*	e2n(Graph*, usize);
void	nukegraph(Graph*);
Graph*	initgraph(void);
Node	newnode(void);
Edge	newedge(void);
int	addnode(Graph*, char*, char*);
int	addedge(Graph*, char*, char*, int, int, char*, double);

void	initlayout(void);
int	newlayout(Graph*, int);
int	resetlayout(Graph*);
void	stoplayout(Graph*);
void	runlayout(Graph*);
int	updatelayout(Graph*);
void	putnode(Node*, int, int);

#define	PBIT64(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;(p)[2]=(v)>>16;(p)[3]=(v)>>24;\
	(p)[4]=(v)>>32;(p)[5]=(v)>>40;(p)[6]=(v)>>48;(p)[7]=(v)>>56;}while(0)
#define	GBIT64(p)	((u32int)(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8)|\
	(((uchar*)(p))[2]<<16)|(((uchar*)(p))[3]<<24)) |\
	((uvlong)(((uchar*)(p))[4]|(((uchar*)(p))[5]<<8)|\
	(((uchar*)(p))[6]<<16)|(((uchar*)(p))[7]<<24)) << 32))

void	initfs(void);
int	loadfs(char*, int);
int	openfs(File*, char*, int);
File*	graphopenfs(char*, int, Graph*);
int	chlevel(Graph*, int);
int	writefs(File*, void*, int);
vlong	seekfs(File*, vlong);
vlong	tellfs(File*);
void	closefs(File*);
u8int	get8(File*);
u16int	get16(File*);
u32int	get32(File*);
u64int	get64(File*);
double	getdbl(File*);
int	put64(File*, u64int);

void	sysinit(void);
int	sysopen(File*, int);
int	syswrite(File*, void*, int);
int	sysread(File*, void*, int);
int	syswstatlen(File*, vlong);
vlong	sysftell(File*);
vlong	sysseek(File*, vlong);
void	sysflush(File*);
void	sysclose(File*);
void	sysquit(void);

int	initsysdraw(void);
int	updatedraw(void);
int	redraw(void);
int shallowdraw(void);
void	reqdraw(int);

void	initrend(void);
int	renderlayout(Graph*);

int	panview(Vector);
int	zoomview(Vector);
void	initui(void);
void	evloop(void);
void	errmsg(char*, ...);
void	resetui(int);
int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
char*	shitprint(int, void*);
void	quit(void);
