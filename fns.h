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
Vertex	scalevx(Vertex, float);
Quad	insetvx(Vertex, int);
Vertex	Vx(int, int);
Quad	quadaddvx(Quad, Vertex);

Node*	id2n(Graph*, usize);
void	nukegraph(Graph*);
Graph*	initgraph(void);
int	addnode(Graph*, usize, char*);
int	addedge(Graph*, usize, usize, char*, double);

void	initlayout(void);
int	dolayout(Graph*, int);

int	pushcmd(int, char*, ...);
int	flushcmd(void);

char*	readrecord(File*);
void	initfs(void);
Graph*	loadfs(int, char*);

int	initvdraw(void);
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
