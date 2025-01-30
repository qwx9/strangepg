typedef struct RNode RNode;
typedef struct REdge REdge;
typedef struct RLine RLine;
typedef struct Quad Quad;
typedef struct View View;
typedef struct Color Color;
typedef struct Box Box;
typedef	struct Rekt Rekt;
typedef struct Drawing Drawing;
typedef struct Range Range;

#pragma incomplete Color

#define	Nodesz	2.0f
#define	Ptsz	1.5f
#define	Minsz	0.2f
#define	Maxsz	(100.0f * Minsz)

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
};
struct Drawing{
	int flags;
	Range length;
	Range xbound;
	Range ybound;
	Range zbound;
	float nodesz;
	float fatness;
	char *layfile;
};
extern Drawing drawing;

/* FIXME: later, vertices or HMM vectors? also, alignment */
struct RNode{
	float pos[3];
	float dir[3];
	float col[4];
	float len;
};
struct REdge{
	float pos1[3];
	float pos2[3];
};
/* FIXME: selection, etc. */
struct RLines{
	float pos1[3];
	float col1[4];
	float pos2[3];
	float col2[4];
};
extern RNode *rnodes;
extern REdge *redges, selbox[4];
extern ssize ndnodes, ndedges;

struct Quad{
	Vertex tl;
	Vertex tr;
	Vertex br;
	Vertex bl;
};

struct Rekt{
	int x1;
	int y1;
	int x2;
	int y2;
};

#define	V(x,y,z)	((Vertex){(x), (y), (z)})
#define	addv(u,v)	((Vertex){(u).x+(v).x, (u).y+(v).y, (u).z+(v).z})
#define	subv(u,v)	((Vertex){(u).x-(v).x, (u).y-(v).y, (u).z-(v).z})
#define	mulv(u,s)	((Vertex){(u).x*(s), (u).y*(s), (u).z*(s)})
#define	divv(u,s)	((Vertex){(u).x/(s), (u).y/(s), (u).z/(s)})
#define	vinrect(u,r)	((u).x>=(r).x1 && (u).x<(r).x2 && (u).y>=(r).y1 && (u).y<(r).y2)
#define	eqv(u,v)	((u).x==(v).x && (u).y==(v).y && (u).z==(v).z)
#define Q(a,b,c,d)	((Quad){(a), (b), (c), (d)})
#define qaddv(q,v)	Q(addv((q).tl,(v)), addv((q).tr,(v)), addv((q).bl,(v)), addv((q).br,(v)))
#define qsubv(q,v)	Q(subv((q).tl,(v)), subv((q).tr,(v)), subv((q).bl,(v)), subv((q).br,(v)))
#define	qinset(q,s)	Q(subv((q).tl,V((s),(s))), subv((q).tr,V((s),(s))), addv((q).bl,V((s),(s))), addv((q).br,V((s),(s))))

#define zrotv(v,cosθ,sinθ)	V((v).x * (cosθ) - (v).y * (sinθ), (v).x * (sinθ) + (v).y * (cosθ), 0.0f)
#define centerscalev(v)	addv(subv(mulv((v), view.zoom), view.pan), view.center)

#define	ZV	V(0.0f, 0.0f, 0.0f)

struct Box{
	float x1;
	float y1;
	float x2;
	float y2;
};

struct View{
	int w;
	int h;
	float ar;
	float fov;
	float tfov;
	float θ;
	float φ;
	Vertex eye;
	Vertex center;
	Vertex up;
	Vertex Δeye;
	Vertex pan;
	double zoom;
	Box prompt;
};
extern View view;

void	endmove(void);
void	zoomdraw(float, float, float);
void	pandraw(float, float);
void	rotdraw(Vertex);
void	worldview(Vertex);
u32int	mousepick(int, int);
int	redraw(int);

enum{
	Cbg,
	Ctext,
	Cedge,
	Cemph,
	Chigh,
	Cend,
};
extern u32int *theme;

void	highlightnode(ioff);
Color*	newcolor(u32int);
Color*	color(u32int);
u32int	somecolor(ioff, char**);
void	settheme(void);
u32int	getcolor(float*);
void	setcolor(float*, u32int);

enum{
	Reqresetdraw = 1<<0,	/* reset and redo everything */
	Reqresetui = 1<<1,		/* reset view position, etc., redraw */
	Reqrefresh = 1<<2,		/* render and redraw: while layouting */
	Reqshape = 1<<3,		/* change node shape */
	Reqredraw = 1<<4,		/* paint and flush canvas */
	Reqshallowdraw = 1<<5,	/* re-flush current canvas: for ui */
	Reqfocus = 1<<6,		/* focus node event */
	Reqpickbuf = 1<<7,		/* cache new mouse picking buffer */
	Requpdatedraw = 1<<8,	/* nodes have been added or removed */
	Reqstop = 1<<31,		/* cease all activity and operations */
};

void	resizenodes(void);
void	setnodeshape(int);
void	updatedraw(void);
void	reqdraw(int);
void	initcol(void);
void	noloop(void);
void	initsysdraw(void);
void	initdrw(void);
