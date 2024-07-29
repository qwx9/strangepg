extern int noreset;

extern char *awkprog;
extern int epfd[2];

void	readcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
int	initcmd(void);
