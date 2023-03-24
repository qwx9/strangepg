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

KHASH_MAP_INIT_INT64(usize, usize);

typedef struct Shape Shape;
typedef struct Vertex Vertex;
typedef struct Vquad Vquad;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Edge Edge;


struct Vertex{
	int x;
	int y;
};
struct Vquad{
	Vertex u;
	Vertex v;
};
extern Vertex ZV;

/* FIXME: vectors of node id's or edge id's? */
struct Node{
	usize id;
	kvec_t(usize) in;
	kvec_t(usize) out;
};

struct Edge{
	usize u;
	usize v;
	double w;
};

enum{
	SHline = 0<<0,
	SHrect = 1<<0,
};
struct Shape{
	usize id;	// LSB: type
	Vquad r;
};
