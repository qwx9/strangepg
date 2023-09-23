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
	vlong cacheoff;
	usize nbuf;
	uchar *buf;
	usize bufsz;
	int norm;
	vlong age;
};

EM*	emcreate(usize);
EM*	emopen(char*);
EM*	emclone(char*, vlong, usize);
uchar*	emfetch(EM*, vlong, ssize);
ssize	embarf(EM*, vlong, uchar*, ssize);
u64int	emget64(EM*, vlong);
ssize	emput64(EM*, vlong, u64int);
void	emshrink(EM*, usize);
ssize	emflush(EM*);
vlong	emseek(EM*, vlong, int);
void	emnuke(EM*);
void	emclose(EM*);
ssize	emsysopen(char*);
ssize	emsyscreate(char*);
ssize	emsysread(EM*, uchar*, ssize);
ssize	emsyswrite(EM*, uchar*, ssize);
vlong	emsysseek(EM*, vlong, int);
