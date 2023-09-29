typedef struct EM EM;
typedef struct Chunk Chunk;

enum{
	Maxmem = 256*1024*1024,
};

struct Chunk{
	ssize start;
	ssize end;
	uchar *buf;
	ssize bufsz;
	int fd;
	Chunk *left;
	Chunk *right;
	Chunk *lleft;
	Chunk *lright;
};
struct EM{
	int fd;
	int tmpfd;
	char *path;
	ssize sz;
	ssize off;
	Chunk *cp;
	Chunk c;
};

int	emshrink(EM*, ssize);
int	emappend(EM*, EM*);
int	emflip(EM*, EM*);
void	emflush(EM*);
vlong	emseek(EM*, vlong, int);
int	emread(EM*, vlong, uchar*, ssize);
u64int	empget64(EM*, vlong);
u64int	emget64(EM*);
int	emwrite(EM*, vlong, uchar*, ssize);
int	empput64(EM*, vlong, u64int);
int	emput64(EM*, u64int);
EM*	emnew(void);
EM*	emfdopen(int);
EM*	emopen(char*);
EM*	emclone(char*);
void	emclose(EM*);
