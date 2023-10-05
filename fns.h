void	warn(char*, ...);
void	dprint(int, char*, ...);
void	vawarn(char*, va_list);
void	vadebug(char*, char*, va_list);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
int	parseargs(int, char **);
#define panic(x) {warn((x)); abort();}

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

int	zoomgraph(Graph*, int);

Node*	getithnode(Graph*, usize);
Edge*	getithedge(Graph*, usize);
Edge	getedgedef(Graph*, usize);

void	printgraph(Graph*);
Node*	id2n(Graph*, char*);
usize	nodeid(Graph*, usize);
usize	packedid(Graph*, usize);
usize	pushpackededge(Graph*, usize, usize, usize);
usize	pushnode(Graph*, usize, int);
int	pushnamednode(Graph*, char*);
int	pushnamededge(Graph*, char*, char*, int, int);
void	nukegraph(Graph*);
Graph*	initgraph(void);

void	initlayout(void);
int	newlayout(Graph*, int);
int	resetlayout(Graph*);
void	stoplayout(Graph*);
void	runlayout(Graph*);
int	updatelayout(Graph*);

void	initfs(void);
void	freefs(File*);
int	loadfs(char*, int);

void	coffeetime(void);
void	coffeeover(void);
void	sysinit(void);
void	sysquit(void);

int	initsysdraw(void);
int	updatedraw(void);
int	redraw(void);
int shallowdraw(void);
void	reqdraw(int);

void	initrend(void);
int	renderlayout(Graph*);
int	rerender(int);

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
