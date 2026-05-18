typedef struct RNode RNode;
typedef struct REdge REdge;
typedef struct RLine RLine;
typedef struct Quad Quad;
typedef struct Box Box;
typedef struct Drawing Drawing;
typedef struct IRange IRange;
typedef struct Range Range;
typedef struct AABB AABB;
typedef struct Tree Tree;
typedef struct B B;

/* FIXME: dynamic or adopt different curves */
#define	Nodesz	1.0f
#define	Ptsz	(Nodesz * 0.8f)
#define	Minsz	0.1f
#define	Maxsz	60.0f

#define	Maxthic	128.0f

struct IRange{
	vlong min;
	vlong max;
};
struct Range{
	double min;
	double max;
};
struct AABB{
	Vec3 tl;
	Vec3 br;
};

enum{
	Fxaxis = 0,
	Fyaxis = 1,
	Fzaxis = 2,
	Fleaf = 3,
};
struct B{
	uint child;			/* 1:0: axis, 31:2: index to child pair */
	union{
		struct{			/* leaves */
			ioff off;
			ioff n;
		};
		struct{			/* inner nodes */
			float l;
			float r;
		};
	};
};
struct Tree{
	B root;
	B *b;
	ioff *idx;
};

enum{
	DFhaxx0rz = 1<<0,
	DFdrawarrows = 1<<1,
	DFdrawlabels = 1<<2,
	DFnodepth = 1<<3,
	DFstalelen = 1<<4,
	DFnomsaa = 1<<5,
	DFhidpi = 1<<6,
	DFnope = 1<<7,
	DF3d = 1<<8,
	DFnoalpha = 1<<9,
	DFnoray = 1<<10,
	DFrecalclen = 1<<29,
	DFfiring = 1<<30,
	DFarmed = 1<<31,
};
struct Drawing{
	int flags;
	IRange length;	/* FIXME: make it actual LN range */
	Range rlen;
	double k;
	double meansz;
	float mid[3];
	float nodesz;
	float fatness;
	Tree tree;		/* space partitioning */
	AABB bb;	/* FIXME: compute scene box here while drawing */
};
extern Drawing drawing;

/* FIXME: later, vertices or HMM vectors? also, alignment */
struct RNode{
	float pos[3];
	float dir[4];
	float col[4];
	float len;
};
/* FIXME: should be able to reuse the node vertices from rnodes */
struct REdge{
	float pos1[3];
	float pos2[3];
};
struct RLine{
	float pos1[4];
	float col1[4];
	float pos2[4];
	float col2[4];
};
extern RNode *rnodes, *vnodes;
extern REdge *redges;
extern RLine *rlines;
extern ioff *v2id;

struct Box{
	float x1;
	float y1;
	float x2;
	float y2;
};
extern Box selbox, promptbox;

enum{
	DSstalepick = 1<<0,
	DSmoving = 1<<1,
};
extern int drawstate;

void	endmove(void);
u32int	mousepick(int, int);

enum{
	Cbg,
	Ctext,
	Cedge,
	Cemph,
	Chigh,
	Cxaxis,
	Cyaxis,
	Czaxis,
	Cend,
};
extern u32int *theme;

void	highlightnode(RNode*);
u32int	setdefalpha(u32int);
void	settheme(void);
void	mixcolors(float*, u32int);
u32int	getcolor(float*);
void	setcolor(float*, u32int);

enum{
	Reqresetdraw = 1<<0,	/* reset and redo everything */
	Reqresetview = 1<<1,	/* reset view position, etc., redraw */
	Reqrefresh = 1<<2,		/* re-render + redraw if needed */
	Reqshape = 1<<3,		/* change node shape */
	Reqredraw = 1<<4,		/* redraw current geometry */
	Reqshallowdraw = 1<<5,	/* re-render w/o buffer updates (ui) */
	Reqfocus = 1<<6,		/* focus node event */
	Reqpickbuf = 1<<7,		/* cache new mouse picking buffer */
	Reqsleep = 1<<29,		/* force renderer to wait for events */
	Reqstop = 1<<30,		/* cease all activity and operations */
	Reqnone = 1<<31,		/* dummy event */
	Reqanydraw = Reqrefresh | Reqredraw | Reqshallowdraw | Reqfocus,
	Reqinit = Reqredraw,
};

void	nuketree(Tree*);
int	mktree(Tree*);

ioff	raytrace(Tree*);

void	spawnrnode(RNode*, RNode*, voff);

void	resizedraw(void);
void	renderframe(ulong, int);
void	frame(void);
int	redraw(void);
void	setnodeshape(int);
void	updatedraw(void);
void	wakedrawup(void);
void	thawworld(int, int, RNode*);
void	freezeworld(void);
void	reqdraw(int);
void	lockrend(void);
int	canlockrend(void);
void	unlockrend(void);
void	wlockdraw(void);
void	wunlockdraw(void);
int	canlockdraw(void);
void	lockdraw(void);
void	unlockdraw(void);
void	initscreen(int, int);
void	waitforit(void);
void	initview(void);
void	initdrw(void);
