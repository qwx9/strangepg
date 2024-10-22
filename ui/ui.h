enum{
	Klmb = Kmouse | 1<<0,
	Kmmb = Kmouse | 1<<1,
	Krmb = Kmouse | 1<<2,
	Kscrlup = Kmouse | 1<<3,
	Kscrldn = Kmouse | 1<<4,
};

extern char selstr[], hoverstr[];

void	resetprompt(void);
void	prompt(Rune);
void	focusobj(void);
void	focusnode(ioff);
void	showobject(char*);
void	showselected(char*, ioff);
int	mouseevent(Vertex, Vertex);
int	keyevent(Rune, int);
void	evloop(void);
void	resetui(void);
void	initsysui(void);
void	initui(void);
