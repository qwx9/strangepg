extern int noreset;

extern char *awkprog;
extern int infd[2], outfd[2];

void	killcmd(void);
void	flushcmd(void);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
void	awk(void*);
int	initcmd(void);
