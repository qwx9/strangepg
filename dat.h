typedef struct Vec Vec;
typedef struct Graph Graph;
typedef struct Level Level;
typedef struct Shape Shape;
typedef struct Quad Quad;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layout Layout;
typedef struct View View;
typedef struct File File;

// FIXME: possibly avoidable; see after command impl
#pragma incomplete File

enum{
	Vforward = 0,
	Vreverse = 1,
};

struct Vec{
	void *buf;
	void *tail;
	int elsz;
	usize len;
	usize bufsz;
};

/* FIXME: get rid of this, isolate it in its own corner, with
 * pragma incomplete or w/e */
#include "khash.h"
KHASH_MAP_INIT_STR(id, usize)
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
	int (*compute)(Graph*);
};
extern int deflayout;

struct Level{
	vlong noff;	/* absolute offset */
	usize nnel;	/* number of nodes/supernodes for level */
	usize ntot;	/* cumulated total of nodes/supernodes */
	vlong eoff;
	usize enel;
	usize etot;
};
struct Node{
	vlong id;
	vlong seq;
	//char *id;
	//char *seq;
	Vec in;
	Vec out;
	double w;
	Quad q1;		/* bounding polygon */
	Quad q2;
	Quad vrect;		/* direction/length vector */
	double θ;
	int erased;
	int parent;
};
struct Edge{
	usize from;
	usize to;
	//char *overlap;
	vlong overlap;
	double w;
	int erased;
	int parent;
};
struct Graph{
	int stale;
	int working;
	int type;
	File *infile;
	int level;
	vlong doff;	/* FIXME: foutoir + layer violation */
	vlong noff;
	vlong eoff;
	vlong moff;
	usize nnodes;	/* totals for all levels */
	usize nedges;
	usize nlevels;
	Vec levels;
	Vec edges;
	Vec nodes;
	Htab *id2n;
	Layout *ll;
	Quad dim;
	Vertex off;
};
extern Graph *graphs;
extern int ngraphs;

enum{
	COMload = 'l',
	COMredraw = 'R',
	COMnil,
};

enum{
	FFgfa,
	FFindex,
	FFnil,
};

enum{
	DTrender,
	DTreset,
	DTredraw,
	DTmove,
	DTresetui,
};

enum{
	Vdefw = 800,
	Vdefh = 600,
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

	/* unicode arrows, children's compilers, mandrake */
	KBleft = 0x110000,	/* outside unicode range */
	KBright,
	KBup,
	KBdown,
	KBescape,
};

extern int indexed;
extern int debug;
extern int noui;
