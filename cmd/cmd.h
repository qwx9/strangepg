enum{
	ALOAD,
	ALOADBATCH,
};

enum{
	TLN,	/* reevaluated during layouting */
	Tfx,
	Tfy,
	Tfz,
	Tx0,
	Ty0,
	Tz0,
	Tlayout,	/* marker for last tag affecting layout */
	Tnode = Tlayout,
	Tedge,
	Tlabel,
	TCL,
	Tnil,
};

extern int noreset;

extern char *awkprog;
extern int infd[2], outfd[2];

int	gettab(char*);
void	settag(char*, ioff, char, char*, int);
void	setspectag(int, ioff, char*);
void	setnamedtag(char*, char*, char*);
void	initext(void);

void	killcmd(void);
void	flushcmd(void);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
void	awk(void*);
int	initcmd(void);
