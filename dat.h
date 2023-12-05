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

#pragma incomplete File
#pragma incomplete Coarse

enum{
	Vforward = 0,
	Vreverse = 1,
};

#define Htab khash_t(id)

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
	LLconga,
	LLrandom,
	LLforce,
	LLnil,
};
struct Layout{
	char *name;
	void (*compute)(Graph*);
};
extern int deflayout;

struct Layouting{
	int tid;
	void *aux;
	Layout *ll;
};

struct Node{
	usize idx;
	usize sid;
	usize *in;		/* dynamic array (v indices) */
	usize *out;		/* dynamic array (v indices) */
	vlong metaoff;
	int weight;
	Quad q1;		/* bounding polygon */
	Quad q2;
	Quad shape;
	Quad vrect;		/* direction/length vector */
	double θ;
};
enum{
	Edgesense = 0<<0,
	Edgeantisense = 1<<0,
};
struct Edge{
	usize u;
	usize v;
};
struct Graph{
	int type;
	File *f;
	Coarse *c;
	usize nnodes;	/* gfa-wide totals */
	usize nedges;
	usize nsuper;
	int nlevels;
	Node *nodes;	/* dynamic array */
	Edge *edges;	/* dynamic array */
	Htab *id2n;
	Layouting layout;
	Quad dim;
	Vertex off;
};
extern Graph *graphs;	/* dynamic array */

#define Bupkis 0xdeadbeefcafebabeULL	/* NA */

enum{
	FFgfa,
	FFindex,
	FFlevel,
	FFnil,
};

enum{
	Reqresetdraw,
	Reqredraw,
	Reqshallowdraw,
	Reqresetui,
	Reqrefresh,
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
extern int multiplier;
