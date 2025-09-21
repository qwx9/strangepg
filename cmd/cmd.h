enum{
	ACOLLAPSE,
	ACOMMIT,
	AEXPAND1,
	AEXPANDALL,
	AEXPLODE1,
	AEXPORTCOARSE,
	AEXPORTGFA,
	AEXPORTSVG,
	AINFO,
	ALOADBATCH,
	AREFRESH,
};

extern int noreset;	// FIXME: flag

extern char *awkprog;
extern int infd[2], outfd[2], eoutfd[2];

int	selectnodebyidx(ioff, int, int);
void	dragselection(float, float, void(*)(ioff, float, float));
void	showselection(void);
int	selectionsize(void);

char*	getname(voff);
voff	getid(char*);
void	loadbatch(void);
int	setedgetag(char*, voff, char, char*);
int	settag(char*, voff, char, char*);
int	setnamedtag(char*, char*, char*);
voff	pushname(char*);
void	fixtabs(voff, int*, ushort*);
void	initvars(void);
void	initext(void);

void	killcmd(void);
void	flushcmd(void);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
void	initstdin(void);
int	initrepl(void);
int	initcmd(void);
