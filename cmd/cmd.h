extern char *awkprog;

void	readcmd(char*);
void	pollcmd(void);
void	sendcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
int	initcmd(void);
