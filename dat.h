typedef struct Vertex Vertex;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layout Layout;
typedef struct File File;
typedef struct Coarse Coarse;
typedef struct Clk Clk;
typedef struct Thread Thread;
typedef struct RNode RNode;
typedef struct REdge REdge;

#pragma incomplete File
#pragma incomplete Coarse
#pragma incomplete Layout
#pragma incomplete Thread

/* FIXME */
typedef	ssize	ioff;

enum{
	Vforward = 0,
	Vreverse = 1,
};

struct Vertex{
	float x;
	float y;
	float z;
};

/* FIXME: later, vertices or HMM vectors? */
struct RNode{
	float pos[2];
	float dir[2];
	float col[4];
};
struct REdge{
	float pos1[2];
	float pos2[2];
	float col[4];
};
extern RNode *rnodes;
extern REdge *redges;

enum{
	FNfixed = 1<<0,
	FNinitpos = 1<<1,
};
struct Node{
	ioff *in;		/* dynamic array (edge indices) */
	ioff *out;		/* dynamic array (edge indices) */
	/* FIXME: attr ht */
	int weight;
	int length;
	Vertex pos0;
	u32int flags;
	ioff pid;		/* key (parent) */
	ioff ch;		/* index */
};
struct Edge{
	ioff u;	/* always packed with direction bit */
	ioff v;
};
enum{
	GFlayme = 1<<0,
	GFdrawme = 1<<1,
	GFarmed = 1<<2,
};
struct Graph{
	int type;
	u32int flags;
	File *f;
	vlong *nodeoff;
	vlong *edgeoff;
	Coarse *c;
	int nlevels;
	Node *nodes;	/* dynamic array */
	Edge *edges;	/* dynamic array */
	Layout *layout;
};
extern Graph *graphs;	/* dynamic array */

enum{
	Debugdraw = 1<<0,
	Debugrender = 1<<1,
	Debuglayout = 1<<2,
	Debugfs = 1<<3,
	Debugcoarse = 1<<4,
	Debugextmem = 1<<5,
	Debugcmd = 1<<6,
	Debugperf = 1<<7,
	Debugmeta = 1<<8,
	Debugtheworld = 0xffffffff,

	PerfΔt = 1000000,
};

struct Clk{
	char lab[32];
	vlong t;
	vlong t0;
	vlong Δt;
	int nsamp;
};

extern int debug;

enum{
	Vdefw = 768,
	Vdefh = 768,
};
