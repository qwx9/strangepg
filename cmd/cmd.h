typedef union V V;
union V{
	s64int i;
	u64int u;
	double f;
	char *s;
};

enum{
	TLN,	/* reevaluated during layouting */
	Tfx,
	Tfy,
	Tfz,
	Tx0,
	Ty0,
	Tz0,

	Tdegree,
	TCL,
	Tnil,

	Tlayout = Tdegree,	/* end of tags affecting layout */
};

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
	ALOAD,
	ALOADBATCH,
	ANODECOLOR,
	AREALEDGE,
	AREFRESH,
	AUNSHOW,
};

extern int noreset;	// FIXME: flag

extern char *awkprog;
extern int infd[2], outfd[2];

char*	getname(voff);
voff	getid(char*);
voff	pushname(char*);
void	setcoretag(int, voff, V);
void	fixtabs(voff, int*, ushort*);
void	initvars(void);
int	gettab(char*);
void	settag(char*, ioff, char*, int);
void	setspectag(int, ioff, V);
void	setnamedtag(char*, char*, char*);
void	initext(void);

void	killcmd(void);
void	flushcmd(void);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
void	initstdin(void);
int	initrepl(void);
int	initcmd(void);

int	setattr(int, ioff, V);
void	setnodelength(Node*, vlong);
