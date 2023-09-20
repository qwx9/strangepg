typedef struct Pair Pair;
typedef struct Super Super;
typedef struct Level Level;
typedef struct Cindex Cindex;
typedef struct Ctree Ctree;

struct Pair{
	usize u;
	usize v;
};
struct Super{
	usize u;		/* index 1..N */
	usize s;		/* supernode id > N */
	int w;
};
struct Level{
	vlong noff;
	usize eoff;
	usize nnodes;
	usize nedges;
};
struct Cindex{
	File *f;
	usize nnodes;	/* ie. N */
	usize nedges;	/* FIXME: technically, redundant */
	Pair *edges;
};
struct Ctree{
	Level *levels;
	Super *supers;	/* level node mappings */
	usize *edgei;	/* indices to Index.edges */
};
struct Coarse{
	Cindex i;
	Ctree t;
	int level;		/* deepest level of any one expansion */
	EM *inodes;
	EM *iedges;
};
// FIXME: make sure to free stuff when added, in nukeindex

void	unloadlevels(Graph*, int, int);
void	loadlevels(Graph*, int, int);

/* FIXME: → fs function, like gfa etc.? */
int	setgraphdepth(Graph *, int);
void	nukeindex(Graph*);
Coarse*	initindex(Graph*);
