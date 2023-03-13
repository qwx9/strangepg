enum{
	Onode,
	Oedge,
};
struct Obj{
	uchar type;
	void *p;
};

void	warn(char*, ...);
void	dprint(char*, ...);
char*	estrdup(char*);
void*	erealloc(void*, usize, usize);
void*	emalloc(usize);
int	parseargs(int, char **);

void	init(void);
void	run(void);
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))
