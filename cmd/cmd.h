extern int noreset;

extern char *awkprog;
extern int epfd[2];

void	readcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
void	awk(void*);
int	initcmd(void);
