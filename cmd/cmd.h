/* FIXME: remove */
typedef union V V;
union V{
	s64int i;
	u64int u;
	double f;
	char *s;
};

/* FIXME: remove */
enum{
	/* FIXME: reevaluated how? */
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

	/* FIXME: get rid of this? */
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
	AREALEDGE,
	AREFRESH,
	AUNSHOW,
};

extern int noreset;	// FIXME: flag

extern char *awkprog;
extern int infd[2], outfd[2];

char*	getname(voff);
voff	getid(char*);
void	loadbatch(void);
void	loadvars(void);
void	settag(char*, voff, char*);
void	setnamedtag(char*, char*, char*);
voff	pushname(char*);
void	fixtabs(voff, int*, ushort*);
void	initvars(void);

/* FIXME */
int	gettab(char*);

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
