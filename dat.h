typedef struct Vec Vec;
typedef struct Graph Graph;
typedef struct Shape Shape;
typedef struct Vertex Vertex;
typedef struct Vquad Vquad;
typedef struct Vnode Vnode;
typedef struct Fertex Fertex;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layer Layer;
typedef struct Layout Layout;
typedef struct Render Render;
typedef struct View View;

struct Vec{
	void *buf;
	void *latch;
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

KHASH_MAP_INIT_INT64(usize, usize);

struct Vertex{
	int x;
	int y;
};
struct Vquad{
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
	Vquad r;
};

struct Vnode{
	Node *u;
	Vquad q;
};

enum{
	LLconga,
	LLnil,
};
struct Layout{
	char *name;
	Layer (*compute)(Graph*);
};
struct Layer{
	Layout *ll;
	kvec_t(Vnode) nodes;
};

struct Render{
	kvec_t(Vnode) nodes;
	Vquad dim;
};

extern Layer ZL;

/* FIXME: what we need:
 * - node vec
 * - edge vec
 * in node: in/out, Edge* vec
 * in layer: Vertex
 * in render: Quad
 */
/* FIXME: vectors of node pointers */
struct Node{
	usize lab; 
	kvec_t(Edge*) in;
	kvec_t(Edge*) out;
	Vquad q;
};

// FIXME: nodes and edges have a name
struct Edge{
	Node *u;
	Node *v;
	double w;
};

struct Graph{
	kvec_t(Edge) edges;
	kvec_t(Node) nodes;
	Layer l;
	Render r;
};
extern Graph *graph;

enum{
	COMload,
	COMnil,
};

enum{
	Vdefw = 800,
	Vdefh = 600,
};
struct View{
	int w;
	int h;
	Vertex pan;
	double zoom;
};
extern View view;
