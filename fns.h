void	warn(char*, ...);
void	dprint(int, char*, ...);
void	vawarn(char*, va_list);
void	vadebug(char*, char*, va_list);
char*	error(void);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
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

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int	errstr(char*, uint);

int	zoomgraph(Graph*, int);

// FIXME: use ssize for handles everywhere, make it a type
int	pushsupernode(Graph*, ssize, ssize, ssize, int);
int	pushsuperedge(Graph*, ssize, ssize, int, int);
Node*	getinode(Graph*, ssize);
ssize	getsupernode(Graph*, ssize);
Node*	gethypernode(Graph*, ssize);
Edge*	getedge(Graph*, ssize);
void	printgraph(Graph*);
void	popnode(Graph*, ssize);
void	popedge(Graph*, ssize);
ssize	id2n(Graph*, char*);
int	pushedge(Graph*, usize, usize, int, int);
ssize	pushnode(Graph*, usize);
int	pushnamednode(Graph*, char*);
ssize	pushnamededge(Graph*, char*, char*, int, int);
void	cleargraph(Graph*);
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
void	quit(void);

void	yield(void);	/* libthread */

/* you gotta wonder what will come next */
#define DPRINT(x,...)	do{ \
	if((debug&(x)) != 0) \
		dprint((x), __VA_ARGS__); \
	}while(0)
