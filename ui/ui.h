enum{
	Mlmb = 1<<0,
	Mmmb = 1<<1,
	Mrmb = 1<<2,
	Mscrlup = 1<<3,	// FIXME: shouldn't be
	Mscrldn = 1<<4,

	K← = 0,
	K→,
	K↑,
	K↓,
};

int	panview(Vertex);

void	initui(void);
int	evloop(void);
void	errmsg(char*, ...);
void	resetui(void);

int	mouseevent(Vertex, Vertex, int);
int	keyevent(Rune);
