int epfd[2];

void	parseresponse(char*);
void	sendcmd(char*);
void	pushcmd(char*, ...);
int	initcmd(void);
int	sysinitcmd(void);
