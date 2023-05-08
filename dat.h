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

/* FIXME: very unsafe mix of pointers and primitives with generic Vec */
struct Level{
	vlong noff;	/* absolute offset */
	usize nlen;	/* length in bytes */
	usize ntot;	/* cumulated total of elements */
	vlong linkoff;	/* in/out link array */
	vlong eoff;
	usize elen;
	usize etot;
};
struct Node{
	char *id;
	char *seq;
	Vec in;
	Vec out;
	Quad q;
	double θ;
};
struct Edge{
	usize from;
	usize to;
	char *overlap;
	double w;
};
struct Graph{
	int stale;
	int working;
	Level *level;
	char *index;
	Vec levels;
	Vec edges;
	Vec nodes;
	Htab *id2n;	// FIXME ← ?
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

enum{
	Mlmb = 1<<0,
	Mmmb = 1<<1,
	Mrmb = 1<<2,
	Mscrlup = 1<<3,	// FIXME: shouldn't be
	Mscrldn = 1<<4,

	K← = 0x110000,	/* outside unicode range */
	K→,
	K↑,
	K↓,
	Kescape,
};

extern int debug;
