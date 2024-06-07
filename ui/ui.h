typedef struct Obj Obj;

enum{
	Klmb = Kmouse | 1<<0,
	Kmmb = Kmouse | 1<<1,
	Krmb = Kmouse | 1<<2,
	Kscrlup = Kmouse | 1<<3,
	Kscrldn = Kmouse | 1<<4,
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

void	resetprompt(void);
void	prompt(Rune);
u32int	scrobj(int, int);
void	showobj(Obj*);
Obj	mouseselect(int, int);
int	mouseevent(Vertex, Vertex);
int	keyevent(Rune, int);
void	evloop(void);
void	resetui(void);
void	initsysui(void);
void	initui(void);
