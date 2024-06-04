typedef struct Obj Obj;

enum{
	/* unicode arrows, children's compilers, mandrake */
	KBleft = 0x110000,	/* outside unicode range */
	KBright,
	KBup,
	KBdown,
	KBescape,
	KBctrl,
	KBshift,
	KBalt,
	KBenter,
	KMlmb,
	KMmmb,
	KMrmb,
	KMscrlup,
	KMscrldn,
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
