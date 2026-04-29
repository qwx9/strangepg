typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Layout Layout;
typedef struct Clk Clk;
typedef struct Thread Thread;
typedef union Vec3 Vec3;

#pragma incomplete Layout
#pragma incomplete Thread

/* glue */
union Vec3{
	struct{
		float x;
		float y;
		float z;
	};
	float p[3];
};

typedef	s32int	ioff;
typedef s32int	voff;	/* FIXME */

enum{
	FNfixedx = 1<<0,
	FNfixedy = 1<<1,
	FNfixedz = 1<<2,
	FNinitx = 1<<3,
	FNinity = 1<<4,
	FNinitz = 1<<5,
	FNorphan = 1<<6,	/* FIXME: get rid of these */
	FNinitpos = FNinitx | FNinity | FNinitz,
	FNfixed = FNfixedx | FNfixedy | FNfixedz | FNorphan,

	/* FIXME: revisit ctab flags */
	FNCalias = 1<<0,	/* FIXME: privatize */

	FNDhigh = 1<<0,
	FNDvis = 1<<1,

	FNUhigh = 1<<0,
};
struct Node{
	/* FIXME: updated when we also update rlen, and only then; so
	 * maybe just compute this length on the fly from ctab */
	vlong length;	/* collapsed length */	/* FIXME: get rid of this */
	ioff id;
	uchar flags;	/* positional flags */
	uchar cflags;	/* coarsening flags */
	uchar dflags;	/* drawing flags */
	uchar uflags;	/* ui flags */
	ioff eoff;
	ioff nedges;	/* FIXME: get rid of this */
};
extern Node *nodes;	/* visible only */
extern ioff *edges;	/* visible only */

enum{
	GFlayme = 1<<0,
	GFdrawme = 1<<1,
	GFarmed = 1<<2,
	GFfrozen = 1<<3,
	GFctarmed = 1<<4,
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
	Debugem = 1<<5,
	Debugcmd = 1<<6,
	Debugperf = 1<<7,
	Debugmeta = 1<<8,
	Debugawk = 1<<9,
	Debugload = 1<<10,
	Debuggraph = 1<<11,
	Debuginfo = 1<<12,
	Debugstrawk = 1<<13,
	Debugui = 1<<14,
	Debugbih = 1<<15,
	Debugray = 1<<16,
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

enum{
	FSlockedctab = 1<<0,
	FSquiet = 1<<1,
	FSnoetags = 1<<2,
	FSdontmindme = 1<<3,
};
extern int status;

extern int debug;
extern char logbuf[], lastmsg[3][64], iserrmsg[3];
extern int nlog, logsz;
extern int onscreen;	/* FIXME: state flag */
