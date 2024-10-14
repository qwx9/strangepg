typedef struct Quad Quad;
typedef struct View View;
typedef struct Color Color;
typedef struct Box Box;

#pragma incomplete Color

enum{
	Nodesz = 4,
	Ptsz = 3,
};

struct Quad{
	Vertex tl;
	Vertex tr;
	Vertex br;
	Vertex bl;
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

enum{
	VFhaxx0rz = 1<<0,
	VFdrawarrows = 1<<1,
	VFdrawlabels = 1<<2,
	VFnodepth = 1<<3,
};
struct View{
	int w;
	int h;
	int flags;
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

void	zoomdraw(float, float, float);
void	pandraw(float, float);
void	rotdraw(Vertex);
void	worldview(Vertex);
ioff	mousepick(int, int);
void	drawselected(void);
int	redraw(void);

void	fixlengths(int, int);

enum{
	Cbg,
	Ctext,
	Cedge,
	Cemph,
	Chigh,
	Cend,
};
extern u32int *theme;

void	mixcolors(float*, u32int);
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
	Reqshape = 1<<3,		/* force refresh: layout end */
	Reqredraw = 1<<4,		/* paint and flush canvas */
	Reqshallowdraw = 1<<5,	/* re-flush current canvas: for ui */
	Reqfocus = 1<<6,		/* focus node event */
	Reqstop = 1<<31,		/* cease all activity and operations */
};

void	reqdraw(int);
void	initcol(void);
void	initsysdraw(void);
void	initdrw(void);
