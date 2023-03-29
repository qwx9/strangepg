typedef struct Graph Graph;
typedef struct Shape Shape;
typedef struct Vertex Vertex;
typedef struct Vquad Vquad;
typedef struct Vnode Vnode;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layer Layer;
typedef struct Layout Layout;
typedef struct Render Render;
typedef struct View View;

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

/* FIXME: vectors of node id's or edge id's? */
struct Node{
	usize id;
	kvec_t(usize) in;
	kvec_t(usize) out;
};

struct Edge{
	usize u;
	usize v;
	double w;
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
	usize id;
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

struct Graph{
	khash_t(usize) *eid;
	khash_t(usize) *nid;
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
