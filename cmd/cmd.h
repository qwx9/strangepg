extern int epfd[2];

void	readcmd(char*);
void	sendcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	startengine(void);
char*	enterprompt(Rune);
