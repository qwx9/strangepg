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
Vec*	vecpush(Vec*, void*);
Vec	vec(usize, usize);

void	init(void);
void	run(void);
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int	errstr(char*, uint);
int	eqvx(Vertex, Vertex);
int	dxvx(Vquad);
int	dyvx(Vquad);
Vquad	vx2r(Vertex, Vertex);
Vertex	addvx(Vertex, Vertex);
Vertex	subvx(Vertex, Vertex);
Vertex	scalevx(Vertex, float);
Vquad	insetvx(Vertex, int);
Vertex	Vx(int, int);
Vquad	quadaddvx(Vquad, Vertex);
Graph*	initgraph(Graph*);
void	addnode(Graph*, usize);
void	addedge(Graph*, usize, usize, usize, double);

void	initlayout(void);
int	dolayout(Graph*, int);

int	pushcmd(int, char*, ...);
int	flushcmd(void);

int	initvdraw(void);
int	resetdraw(Graph*);
int	updatedraw(Graph*);
int	redraw(Graph*);

void	initrend(void);
int	render(Graph*);
