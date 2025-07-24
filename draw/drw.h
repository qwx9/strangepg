typedef struct RNode RNode;
typedef struct REdge REdge;
typedef struct RLine RLine;
typedef struct Quad Quad;
typedef struct Color Color;
typedef struct Box Box;
typedef struct Drawing Drawing;
typedef struct Range Range;

#pragma incomplete Color

#define	Nodesz	1.0f
#define	Ptsz	1.0f
#define	Minsz	0.5f
#define	Maxsz	40.0f
#define	Maxthic	128.0f

struct Range{
	float min;
	float max;
};
enum{
	DFhaxx0rz = 1<<0,
	DFdrawarrows = 1<<1,
	DFdrawlabels = 1<<2,
	DFnodepth = 1<<3,
	DFstalelen = 1<<4,
	DFmsaa = 1<<5,
	DFhidpi = 1<<6,
	DFnope = 1<<7,
	DF3d = 1<<8,
	DFnoalpha = 1<<9,
	DFnodraw = 1<<10,
	DFnorend = 1<<11,
	DFnolayout = 1<<12,
	DFfreeze = 1<<13,
	DFiwasfrozentoday = DFnodraw | DFnorend | DFnolayout,
};
struct Drawing{
	int flags;
	Range length;
	Range xbound;
	Range ybound;
	Range zbound;
	float nodesz;
	float fatness;
	uvlong frames;	/* FIXME: remove after fixing scheduling */
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
/* FIXME: selection, etc. */
struct RLine{
	float pos1[4];
	float col1[4];
	float pos2[4];
	float col2[4];
};
extern RNode *rnodes;
extern REdge *redges;
extern RLine *rlines;
extern ssize ndnodes, ndedges, ndlines;

struct Quad{
	Vertex tl;
	Vertex tr;
	Vertex br;
	Vertex bl;
};

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
int	redraw(int);

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

void	highlightnode(ioff);
Color*	newcolor(u32int);
Color*	color(u32int);
u32int	setalpha(u32int);
u32int	somecolor(ioff, char**);
void	settheme(void);
u32int	getcolor(float*);
void	setcolor(float*, u32int);

enum{
	Reqresetdraw = 1<<0,	/* reset and redo everything */
	Reqresetview = 1<<1,		/* reset view position, etc., redraw */
	Reqrefresh = 1<<2,		/* re-render + redraw if needed */
	Reqshape = 1<<3,		/* change node shape */
	Reqredraw = 1<<4,		/* redraw current geometry */
	Reqshallowdraw = 1<<5,	/* re-render w/o buffer updates (ui) */
	Reqfocus = 1<<6,		/* focus node event */
	Reqpickbuf = 1<<7,		/* cache new mouse picking buffer */
	Reqthaw = 1<<28,		/* revert communication to standard */
	Reqfreeze = 1<<29,		/* ignore all orders until signaled */
	Reqsleep = 1<<30,		/* force renderer to wait for events */
	Reqstop = 1<<31,		/* cease all activity and operations */
};

void	updatenode(ioff);
void	resizenodes(void);
void	setnodeshape(int);
void	updatedraw(void);
void	wakedrawup(void);
void	thawworld(void);
void	freezeworld(void);
void	waitforit(void);
void	reqdraw(int);
void	initview(void);
void	initcol(void);
void	initdrw(void);
