typedef struct Vertex Vertex;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Layout Layout;
typedef struct Clk Clk;
typedef struct Thread Thread;

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
	FNfixedz = 1<<2,
	FNfixed = FNfixedx | FNfixedy | FNfixedz,
	FNinitx = 1<<3,
	FNinity = 1<<4,
	FNinitz = 1<<5,
	FNinitpos = FNinitx | FNinity | FNinitz,
	FNemptyinside = 1<<6,
	FNnew = 1<<7,
};
struct Node{
	uchar flags;
	Vertex pos0;	/* FIXME: very annoying to get rid of */
	short nedges;
	ioff eoff;
};
extern Node *nodes;	/* immutable */
extern ioff *edges;	/* immutable */
extern Node *vnodes;	/* visible only */
extern ioff *vedges;	/* visible only */

enum{
	GFlayme = 1<<0,
	GFdrawme = 1<<1,
	GFarmed = 1<<2,
};
struct Graph{
	int flags;
	Layout *layout;
};
extern Graph graph;

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
	Debugawk = 1<<9,
	Debugload = 1<<10,
	Debuggraph = 1<<11,
	Debuginfo = 1<<12,
	Debugstrawk = 1<<13,
	Debugtheworld = 0xffffffff,

	PerfΔt = 1000000,
};

struct Clk{
	char lab[32];
	vlong t;
	vlong t0;
	double s;
	vlong n;
};

extern int debug;
extern int gottagofast;	/* FIXME: state flag */
extern char logbuf[], lastmsg[3][64], iserrmsg[3];
extern int nlog, logsz;
extern int onscreen;	/* FIXME: state flag */
