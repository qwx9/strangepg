enum{
	Klmb = Kmouse | 1<<0,
	Kmmb = Kmouse | 1<<1,
	Krmb = Kmouse | 1<<2,
	Kscrlup = Kmouse | 1<<3,
	Kscrldn = Kmouse | 1<<4,
};

extern ioff selected;	/* FIXME: only one */
extern char *selstring;

void	resetprompt(void);
void	prompt(Rune);
void	showobject(char*);
int	mouseevent(Vertex, Vertex);
int	keyevent(Rune, int);
void	evloop(void);
void	resetui(void);
void	initsysui(void);
void	initui(void);
