int epfd[2];

void	parseresponse(char*);
void	sendcmd(char*);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	startengine(void);
