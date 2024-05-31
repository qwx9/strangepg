typedef struct Vertex Vertex;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Layout Layout;
typedef struct File File;
typedef struct Coarse Coarse;
typedef struct Color Color;
typedef struct Clk Clk;
typedef struct Thread Thread;

#pragma incomplete File
#pragma incomplete Coarse
#pragma incomplete Layout
#pragma incomplete Color
#pragma incomplete Thread

KHASH_MAP_INIT_STR(strmap, ssize)
KHASH_MAP_INIT_INT64(idmap, ssize)

enum{
	Vforward = 0,
	Vreverse = 1,
};

struct Vertex{
	float x;
	float y;
	float z;
};

enum{
	FNfixed = 1<<0,
	FNinitpos = 1<<1,
};
struct Node{
	ssize id;		/* key */
	ssize pid;		/* key (parent) */
	ssize idx;		/* index: leaf */
	ssize layid;	/* index: layout backref */
	int lvl;
	ssize *in;		/* dynamic array (edge indices) */
	ssize *out;		/* dynamic array (edge indices) */
	vlong metaoff;
	int weight;
	int length;
	Vertex pos;
	Vertex pos0;
	Vertex rot;
	Vertex dir;
	Color *col;
	u32int flags;
	ssize prev;		/* index */
	ssize next;		/* index */
	ssize ch;		/* index */
};
enum{
	FEfixed = 1<<0,
};
struct Edge{
	ssize id;
	ssize u;
	ssize v;
	u32int flags;
	ssize next;
	ssize prev;
	vlong metaoff;
	Color *col;
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
	Coarse *c;
	usize nnodes;	/* gfa-wide totals */
	usize nedges;
	usize nsuper;
	int nlevels;
	Node *nodes;	/* dynamic array */
	Edge *edges;	/* dynamic array */
	Node node0;		/* list */
	Edge edge0;		/* list */
	khash_t(idmap) *nmap;
	khash_t(idmap) *emap;
	khash_t(strmap) *strnmap;
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
