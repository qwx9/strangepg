typedef struct EM EM;

enum{
	Maxmem = 256*1024*1024,
};

struct EM{
	int fd;
	char *path;
	vlong realsz;
	usize totsz;
	vlong off;
	vlong lastoff;
	vlong cacheoff;
	usize nbuf;
	uchar *buf;
	usize bufsz;
	int artwork;
};

EM*	emcreate(usize);
EM*	emopen(char*);
EM*	emclone(char*, vlong, usize);
u64int	empget64(EM*, vlong);
ssize	empput64(EM*, vlong, u64int);
u64int	emget64(EM*);
ssize	emput64(EM*, u64int);
void	emshrink(EM*, usize);
ssize	emflush(EM*);
vlong	emseek(EM*, vlong, int);
void	emnuke(EM*);
void	emclose(EM*);
ssize	empreload(EM*);
ssize	emsysopen(char*);
ssize	emsyscreate(char*);
ssize	emsysread(EM*, uchar*, ssize);
ssize	emsyswrite(EM*, uchar*, ssize);
vlong	emsysseek(EM*, vlong, int);
int	emsysclose(EM*);
