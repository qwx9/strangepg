typedef struct Graph Graph;
typedef struct Shape Shape;
typedef struct Quad Quad;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layout Layout;
typedef struct Layouting Layouting;
typedef struct View View;
typedef struct File File;
typedef struct Coarse Coarse;
typedef struct Color Color;

#pragma incomplete File
#pragma incomplete Coarse
#pragma incomplete Layout
#pragma incomplete Color

KHASH_MAP_INIT_STR(strmap, ssize)
KHASH_MAP_INIT_INT64(idmap, ssize)

enum{
	Vforward = 0,
	Vreverse = 1,
};

typedef Point2 Vertex;
typedef Point2 Vector;

extern Vertex ZV;
extern Quad ZQ;

struct Quad{
	Vertex o;	/* position in original reference */
	Vector v;	/* dimensions and orientation */
};

enum{
	Nodesz = 8,
	Ptsz = 2,
};

enum{
	LFarmed = 1<<0,
	LFonline = 1<<1,
};
struct Layouting{
	int f;
	Layout *ll;
	void *thread;
};

struct Node{
	ssize id;		/* key */
	ssize idx;		/* index (leaf) */
	ssize pid;		/* key */
	int lvl;
	ssize *in;		/* dynamic array (edge indices) */
	ssize *out;		/* dynamic array (edge indices) */
	vlong metaoff;
	int weight;
	Quad q1;		/* bounding polygon */
	Quad q2;
	Quad shape;
	Quad vrect;		/* direction/length vector */
	Color *col;
	double θ;
	ssize prev;		/* index */
	ssize next;		/* index */
	ssize ch;		/* index */
};
enum{
	Sbit = 1ULL<<63,
};
// FIXME
enum{
	Edgesense = 0<<0,
	Edgeantisense = 1<<0,
};
struct Edge{
	ssize id;
	ssize u;
	ssize v;
	ssize next;
	ssize prev;
	vlong metaoff;
	Color *col;
};
struct Graph{
	int type;
	int loading;
	File *f;
	Coarse *c;
	usize nnodes;	/* gfa-wide totals */
	usize nedges;
	usize nsuper;
	int nlevels;
	Node *nodes;
	Edge *edges;
	Node node0;
	Edge edge0;
	khash_t(idmap) *nmap;
	khash_t(idmap) *emap;
	khash_t(strmap) *strnmap;
	Layouting layout;
	Quad dim;
	Vertex off;
};
extern Graph *graphs;	/* dynamic array */

enum{
	Onil,
	Onode,
	Oedge,
};
struct Obj{
	Graph *g;
	int type;
	ssize idx;
};
extern Obj selected;	// FIXME: only one
extern Obj aintnothingthere;

enum{
	FFdead,	/* deallocated */
	FFgfa,
	FFindex,
	FFnil,
};

enum{
	Reqresetdraw = 1<<0,
	Reqredraw = 1<<1,
	Reqshallowdraw = 1<<2,
	Reqresetui = 1<<3,
	Reqrefresh = 1<<4,
	Reqrender = 1<<5,
};

struct View{
	Quad dim;
	Vector pan;
	Vector center;
	double zoom;
};
extern View view;
extern int showarrows, drawstep;
extern int haxx0rz;

enum{
	Mlmb = 1<<0,
	Mmmb = 1<<1,
	Mrmb = 1<<2,
	Mscrlup = 1<<3,	// FIXME: shouldn't be
	Mscrldn = 1<<4,

	Debugdraw = 1<<0,
	Debugrender = 1<<1,
	Debuglayout = 1<<2,
	Debugfs = 1<<3,
	Debugcoarse = 1<<4,
	Debugextmem = 1<<5,
	Debugcmd = 1<<6,
	Debugtheworld = 0xffffffff,

	/* unicode arrows, children's compilers, mandrake */
	KBleft = 0x110000,	/* outside unicode range */
	KBright,
	KBup,
	KBdown,
	KBescape,
};

extern int debug;
extern int noui;
extern int norefresh;

extern int mainstacksize;	// FIXME
