typedef struct EM EM;
typedef struct Chunk Chunk;

/* FIXME: tunable (def or cmdline) */
enum{
	Poolsz = 1ULL<<32,
	Chunksz = 1ULL<<20,
	EMbupkis = 0xdeadbeefcafebabeULL,
};

struct Chunk{
	ssize off;
	uchar *buf;
	Chunk *left;
	Chunk *right;
	Chunk *lleft;
	Chunk *lright;
};
struct EM{
	int fd;
	int tmp;
	int stream;
	int farthest;
	char *path;
	vlong off;
	Chunk c;
};

void	printchain(Chunk*);	// FIXME: debug
ssize	emshrink(EM*, ssize);
int	emappend(EM*, EM*);
int	emembraceextendextinguish(EM*, EM*);
void	emflush(EM*);
uchar*	emread(EM*, vlong, ssize*);
u64int	empget64(EM*, vlong);
u64int	emget64(EM*);
ssize	emwrite(EM*, vlong, uchar*, ssize);
int	empput64(EM*, vlong, u64int);
int	emput64(EM*, u64int);
EM*	emnew(int);
EM*	emfdopen(int, int);
EM*	emopen(char*);
EM*	emclone(char*);
void	emclose(EM*);
