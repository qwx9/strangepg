typedef struct Obj Obj;

enum{
	Mlmb = 1<<0,
	Mmmb = 1<<1,
	Mrmb = 1<<2,
	Mscrlup = 1<<3,	// FIXME: shouldn't be
	Mscrldn = 1<<4,

	/* unicode arrows, children's compilers, mandrake */
	KBleft = 0x110000,	/* outside unicode range */
	KBright,
	KBup,
	KBdown,
	KBescape,
};

enum{
	Onil,
	Onode,
	Oedge,
};
struct Obj{
	Graph *g;
	int type;
	ssize idx;
};
extern Obj selected;	// FIXME: only one
extern Obj aintnothingthere;

u32int	scrobj(int, int);
void	showobj(Obj*);
Obj	mouseselect(int, int);
int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
void	evloop(void);
void	resetui(void);
void	initsysui(void);
void	initui(void);
