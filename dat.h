typedef struct Vertex Vertex;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Layout Layout;
typedef struct File File;
typedef struct Coarse Coarse;
typedef struct Clk Clk;
typedef struct Thread Thread;

#pragma incomplete File
#pragma incomplete Coarse
#pragma incomplete Layout
#pragma incomplete Thread

typedef	s32int	ioff;

struct Vertex{
	float x;
	float y;
	float z;
};

enum{
	FNfixedx = 1<<0,
	FNfixedy = 1<<1,
	FNfixed = FNfixedx | FNfixedy,
	FNinitx = 1<<2,
	FNinity = 1<<3,
	FNinitpos = FNinitx | FNinity,
};
struct Node{
	ioff *in;		/* dynamic array (edge indices) */
	ioff *out;		/* dynamic array (edge indices) */
	/* FIXME: attr ht */
	int length;
	Vertex pos0;
	u32int flags;
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
	RWLock lock;
	Node *nodes;	/* dynamic array */
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
extern int gottagofast;
extern char logbuf[], *lastmsg[3], iserrmsg[3];
extern int nlog, logsz;

enum{
	Vdefw = 1366,
	Vdefh = 768,
};
