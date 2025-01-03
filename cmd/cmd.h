extern int noreset;

extern char *awkprog;
extern int infd[2], outfd[2];

int	awknamedstr(char*, char*, char*);
int	awknamedfloat(char*, char*, double);
int	awknamedint(char*, char*, s64int);
int	awkstr(char*, ioff, char*);
int	awkfloat(char*, ioff, double);
int	awkint(char*, ioff, s64int);

void	killcmd(void);
void	flushcmd(void);
void	pushcmd(char*, ...);
int	sysinitcmd(void);
int	initrepl(void);
void	awk(void*);
int	initcmd(void);
