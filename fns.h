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
void*	vecpush(Vec*, void*);
Vec	vec(usize, usize);

void	init(void);
void	run(void);
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

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
Graph*	initgraph(Graph*);
void	addnode(Graph*, char*);
void	addedge(Graph*, char*, usize, usize, double);

void	initlayout(void);
int	dolayout(Graph*, int);

int	pushcmd(int, char*, ...);
int	flushcmd(void);

int	initvdraw(void);
int	resetdraw(void);
int	updatedraw(Graph*);
int	redraw(Graph*);

void	initrend(void);
int	render(Graph*);
