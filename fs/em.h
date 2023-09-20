typedef struct EM EM;

enum{
	Maxmem = 256*1024*1024,
};

struct EM{
	int fd;
	char *path;
	uchar *buf;
	usize bufsz;
	usize nbuf;
};

EM*	emcreate(usize);
int	emopen(EM*, char*);
ssize	emprefetch(EM*);
uchar*	emreadn(EM*, uchar*, ssize);
ssize	emwrite(EM*, uchar*, ssize);
u64int	emget64(EM*, vlong);
ssize	emput64(EM*, vlong, u64int);
int	emseek(EM*, vlong, int);
void	emclose(EM*);
ssize	emsysopen(char*);
ssize	emsysread(EM*, uchar*, ssize);
ssize	emsyswrite(EM*, uchar*, ssize);
int	emsysseek(EM*, vlong, int);
