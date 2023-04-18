void	warn(char*, ...);
void	dprint(char*, ...);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
int	parseargs(int, char **);

void	vecnuke(Vec*);
void*	vecget(Vec*, usize);
void*	vecpoptail(Vec*);
usize	vecindexof(Vec*, void*);
void*	vecpush(Vec*, void*, usize*);
void*	vecp(Vec*, usize);
Vec	vec(usize, usize);

#define idmap() kh_init(id)
#define idnuke(h) kh_destroy(id, h)
int	idput(Htab*, usize, usize);
int	idget(Htab*, usize, usize*);

void	init(void);
void	run(void);
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

#define	PBIT64(p,v)	do{(p)[0]=(v);(p)[1]=(v)>>8;(p)[2]=(v)>>16;(p)[3]=(v)>>24;\
	(p)[4]=(v)>>32;(p)[5]=(v)>>40;(p)[6]=(v)>>48;(p)[7]=(v)>>56;}while(0)
#define	GBIT64(p)	((u32int)(((uchar*)(p))[0]|(((uchar*)(p))[1]<<8)|\
	(((uchar*)(p))[2]<<16)|(((uchar*)(p))[3]<<24)) |\
	((uvlong)(((uchar*)(p))[4]|(((uchar*)(p))[5]<<8)|\
	(((uchar*)(p))[6]<<16)|(((uchar*)(p))[7]<<24)) << 32))

// FIXME: this is stupid
#define NID(u) ((u)->id >> 1)
#define NDIR(u) ((u)->id & 1)
#define NDIRS(u) (((u)->id & 1) == 0 ? "+" : "-")

int	errstr(char*, uint);
int	eqvx(Vertex, Vertex);
int	dxvx(Quad);
int	dyvx(Quad);
Quad	vx2r(Vertex, Vertex);
Vertex	addvx(Vertex, Vertex);
Vertex	subvx(Vertex, Vertex);
Vertex	divvx(Vertex, float);
Vertex	mulvx(Vertex, float);
Quad	insetvx(Vertex, int);
Vertex	Vx(int, int);
int	vxinquad(Vertex, Quad);
Quad	quadaddvx(Quad, Vertex);

Node*	id2n(Graph*, usize);
void	nukegraph(Graph*);
Graph*	initgraph(void);
int	addnode(Graph*, usize, char*);
int	addedge(Graph*, usize, usize, char*, double);
void	rendernew(void);

void	initlayout(void);
int	dolayout(Graph*, int);

int	pushcmd(int, usize, int, uchar*);
int	flushcmd(void);

void	initfs(void);
Graph*	loadfs(int, char*);

int	initdrw(void);	/* plan9 already has initdraw(2) */
int	resetdraw(void);
int	updatedraw(void);
int	redraw(void);

void	initrend(void);
int	render(Graph*);

int	panview(Vertex);
void	initui(void);
int	evloop(void);
void	errmsg(char*, ...);
void	resetui(void);
int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
char*	quadfmt(Quad*);
char*	vertfmt(Vertex*);
