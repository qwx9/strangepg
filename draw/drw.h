typedef struct Quad Quad;
typedef struct View View;

enum{
	Nodesz = 8,
	Ptsz = 2,	// FIXME: mostly unused
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
#define	vinrect(u,r)	((u).x>=(q).min.x && (u).x<(q).max.x && (u).y>=(q).min.y && (u).y<(q).may.y)
#define	eqv(u,v)	((u).x==(v).x && (u).y==(v).y && (u).z==(v).z)
#define Q(a,b,c,d)	((Quad){(a), (b), (c), (d)})
#define qaddv(q,v)	Q(addv((q).tl,(v)), addv((q).tr,(v)), addv((q).bl,(v)), addv((q).br,(v)))
#define qsubv(q,v)	Q(subv((q).tl,(v)), subv((q).tr,(v)), subv((q).bl,(v)), subv((q).br,(v)))
#define	qinset(q,s)	Q(subv((q).tl,V((s),(s))), subv((q).tr,V((s),(s))), addv((q).bl,V((s),(s))), addv((q).br,V((s),(s))))

#define	ZV	V(0.0f, 0.0f, 0.0f)

enum{
	VFhaxx0rz = 1<<0,
	VFdrawarrows = 1<<1,
	VFdrawlabels = 1<<2,
};
struct View{
	int w;
	int h;
	int flags;
	float ar;
	Vertex eye;
	Vertex center;
	Vertex up;
	Vertex Δeye;
	Vertex pan;
	double zoom;
};
extern View view;

void	zoomdraw(float);
void	pandraw(float, float);

int	drawline(Vertex, Vertex, double, int, s32int, Color*);
int	drawbezier(Vertex, Vertex, double, s32int, Color*);
int	drawquad(Vertex, Vertex, s32int, Color*);
int	drawlabel(Node*, Color*);
void	drawui(void);
void	cleardraw(void);
int	redraw(int);
int	reshape(int);

KHASH_MAP_INIT_INT(cmap, Color*)

extern khash_t(cmap) *cmap;
enum{
	Cbg,
	Ctext,
	Cnode,
	Cedge,
	Cemph,
	Cend,
};
extern u32int *theme;

int	setnodecolor(Graph*, Node*, u32int);
Color*	newcolor(u32int);
void	freecolor(Color*);
u32int	col2int(Color*);
Color*	color(u32int);
Color*	somecolor(Graph*);
void	settheme(void);

enum{
	Reqresetdraw = 1<<0,	/* reset and redo everything */
	Reqresetui = 1<<1,		/* reset view position, etc., redraw */
	Reqrefresh = 1<<2,		/* render and redraw: while layouting */
	Reqshape = 1<<3,		/* force refresh: layout end */
	Reqredraw = 1<<4,		/* paint and flush canvas */
	Reqshallowdraw = 1<<5,	/* re-flush current canvas: for ui */
};

void	reqdraw(int);
void	stopdrawclock(void);
void	startdrawclock(void);
void	initsysdraw(void);
void	initdrw(void);
