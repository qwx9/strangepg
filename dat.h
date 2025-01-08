typedef struct Vertex Vertex;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Layout Layout;
typedef struct Clk Clk;
typedef struct Thread Thread;
typedef struct Attr Attr;

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
	FNvisible = 1<<6,
};
/* FIXME: ht, or sth for some of these: idx → {pos0} */
struct Attr{
	u32int color;
	int length;
	uchar flags;
	Vertex pos0;
};
struct Node{
	Attr attr;
	short nedges;
	short nin;
	ioff eoff;
};
enum{
	GFlayme = 1<<0,
	GFdrawme = 1<<1,
	GFarmed = 1<<2,
};
struct Graph{
	int type;
	uchar flags;
	Node *nodes;
	ioff *edges;
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
	Debugawk = 1<<9,
	Debugload = 1<<10,
	Debuggraph = 1<<11,
	Debuginfo = 1<<12,
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
extern int gottagofast;
extern char logbuf[], lastmsg[3][64], iserrmsg[3];
extern int nlog, logsz;
extern int onscreen;
