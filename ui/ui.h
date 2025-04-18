enum{
	Klmb = Kmouse | 1<<0,
	Kmmb = Kmouse | 1<<1,
	Krmb = Kmouse | 1<<2,
	Kscrlup = Kmouse | 1<<3,
	Kscrldn = Kmouse | 1<<4,
};

extern char selstr[512], hoverstr[256];

void	resetprompt(void);
void	prompt(Rune);
void	focusobj(void);
void	focusnode(ioff);
int	mouseevent(float, float, float, float);
int	keyevent(Rune, int);
void	evloop(void);
void	resetview(void);
void	initsysui(void);
void	initui(void);
