typedef struct EM EM;
typedef struct Chunk Chunk;

/* FIXME: tunable (def or cmdline) */
enum{
	/* must be powers of two */
	Poolsz = 1ULL<<32,
	Chunkshl = 20,
	Chunksz = 1ULL<<Chunkshl,
	EMbupkis = 0xdeadbeefcafebabeULL,

	EMclown = 1<<0,	/* inflate pants always */
};

struct Chunk{
	vlong off;
	vlong len;
	uchar *buf;
	Chunk *lleft;
	Chunk *lright;
};
struct EM{
	int fd;
	int tmp;
	int stream;
	int mode;
	char *path;
	Chunk **cp;
	vlong off;
	Chunk l;
};

void	printchain(Chunk**);
ssize	emshrink(EM*, ssize);
int	embraceextendextinguish(EM*, EM*);
int	emsteal(EM*, EM*);
void	emflush(EM*);
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

/* no refunds if ub */
#define COFF(x)	((x) & ~(Chunksz-1))
#define CADDR(x)	((x) >> Chunkshl)
