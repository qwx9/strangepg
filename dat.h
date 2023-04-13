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
typedef struct File File;

// FIXME: possibly avoidable; see after command impl
#pragma incomplete File

enum{
	Vforward = 0,
	Vreverse = 1,
};

struct Vec{
	void *buf;
	int elsz;
	usize len;
	usize bufsz;
};

#include "khash.h"
KHASH_MAP_INIT_INT64(id, usize)
#define Htab khash_t(id)

struct Vertex{
	int x;
	int y;
};
struct Quad{
	Vertex u;
	Vertex v;
};
extern Vertex ZV;

// FIXME: hengliesque generic 2d/3d version? point + rect
struct Fertex{
	float x;
	float y;
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
	usize id;
	char *seq;
	Vec in;
	Vec out;
	Quad q;
};
struct Edge{
	usize u;
	usize v;
	char *overlap;
	double w;
};
struct Graph{
	Vec edges;
	Vec nodes;
	Htab *id2n;
	Layout *ll;
	Vertex dim;
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
	FFgfa2,
	FFnil,
};

enum{
	Vdefw = 800,
	Vdefh = 600,
};
struct View{
	Quad dim;
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

extern int debug;
