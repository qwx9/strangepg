extern int noreset;

extern char *awkprog;
extern int infd[2], outfd[2];

void	readcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
void	awk(void*);
int	initcmd(void);
