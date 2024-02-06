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

#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))

int	errstr(char*, uint);

int	zoomgraph(Graph*, int);

void	expandnode(Graph*, Node*);
void	retractnode(Graph*, Node*);

Node*	getnode(Graph*, ssize);
Node*	getactivenode(Graph*, Node*);
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
Graph*	initgraph(void);

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

void	coffeetime(void);
void	coffeeover(void);
void	sysinit(void);
void	sysquit(void);

int	initsysdraw(void);
void	drawui(void);
void	redraw(void);
void	flushdraw(void);
void	reqdraw(int);
int	scrobj(Vertex);
void	showobj(Obj*);

void	initrend(void);
int	renderlayout(Graph*);
int	rerender(int);

Obj	mouseselect(Vertex);
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
