typedef struct EM EM;
typedef struct Chunk Chunk;

/* FIXME: tunable (def or cmdline) */
enum{
	/* must be powers of two */
	Poolsz = 1ULL<<32,
	Chunkshl = 13,
	Chunksz = 1ULL<<Chunkshl,
	EMbupkis = 0xdeadbeefcafebabeULL,
	EMexist = 1ULL<<63,

	// FIXME: clown is hacky; regularize
	EMclown = 1<<0,	/* inflate pants always */
	EMshutit = 1<<1,	/* don't spew to file */
	EMpipe = 1<<2,	/* streaming to output pipe */
	EMondisk = 1<<3,	/* has been created */
	EMclone = 1<<4,	/* cloned, fully read file (not sparse) */
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
	int flags;
	char *path;
	Chunk **cp;
	Chunk l;
};

void	printchain(Chunk**);
void	emflushtofs(EM*, File*);
ssize	emwrite(EM*, vlong, uchar*, ssize);
void	emw64(EM*, ssize, usize);
usize	emr64(EM*, ssize);
EM*	emnew(int);
EM*	emfdopen(int, int);
EM*	emopen(char*);
EM*	emclone(char*);
void	emclose(EM*);

/* no refunds if ub */
#define CMOD(x)	((x) & (Chunksz-1))
#define COFF(x)	((x) & ~(Chunksz-1))
#define CADDR(x)	((x) >> Chunkshl)
