enum{
	ACOLLAPSE,
	ACOMMIT,
	AEXPAND1,
	AEXPANDALL,
	AEXPLODE,
	AEXPORTCOARSE,
	AEXPORTGFA,
	AEXPORTSVG,
	AINFO,
	ALOADBATCH,
	AREFRESH,
};

extern int noreset;	// FIXME: flag

extern char *awkprog;
extern int infd[2], outfd[2];

char*	getname(voff);
voff	getid(char*);
void	loadbatch(void);
void	setedgetag(char*, voff, char*);
void	settag(char*, voff, char*);
void	setnamedtag(char*, char*, char*);
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
