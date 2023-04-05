typedef struct Vec Vec;
typedef struct Graph Graph;
typedef struct Shape Shape;
typedef struct Vertex Vertex;
typedef struct Fertex Fertex;
typedef struct Quad Quad;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layout Layout;
typedef struct View View;

struct Vec{
	void *buf;
	int elsz;
	usize len;
	usize bufsz;
};

enum{
	Onode,
	Oedge,
};
struct Obj{
	uchar type;
	void *p;
};

struct Vertex{
	int x;
	int y;
};
struct Quad{
	Vertex u;
	Vertex v;
};
extern Vertex ZV;

struct Fertex{
	float x;
	float y;
};

enum{
	SHline = 0<<0,
	SHrect = 1<<0,
};
struct Shape{
	usize id;	// LSB: type
	Quad r;
};

enum{
	LLconga,
	LLnil,
};
struct Layout{
	char *name;
	int (*compute)(Graph*);
};

struct Node{
	char *label;
	Vec in;
	Vec out;
	Quad q;
};
struct Edge{
	char *label;
	Node *u;
	Node *v;
	double w;
};
struct Graph{
	Vec edges;
	Vec nodes;
	Layout *ll;
	Vertex dim;
};
extern Graph *graph;

enum{
	COMload,
	COMnil,
};

typedef struct Filefmt Filefmt;
typedef struct Dat Dat;

enum{
	FFgfa,
	FFgfa2,
	FFnil,
};
struct Filefmt{
	char *name;
	Graph* (*load)(char *);
	int (*save)(Graph*);
};
extern Filefmt fftab[FFnil];

struct Dat{
	int type;
	Filefmt *ff;
	void *p;
	usize sz;
};

void	initfs(void);
void	regfs(Filefmt*);
Graph*	loadfs(int, char*);


enum{
	Vdefw = 800,
	Vdefh = 600,
};
struct View{
	int w;
	int h;
	Vertex pan;
	Vertex vpan;
	double zoom;
};
extern View view;

enum{
	Mlmb = 1<<0,
	Mmmb = 1<<1,
	Mrmb = 1<<2,
	Mscrlup = 1<<3,	// FIXME: shouldn't be
	Mscrldn = 1<<4,

	K← = 0,
	K→,
	K↑,
	K↓,
};

int	panview(Vertex);

void	initui(void);
int	evloop(void);
void	errmsg(char*, ...);
void	resetui(void);

int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
