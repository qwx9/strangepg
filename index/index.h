typedef struct Level Level;

// FIXME: figure out how to slim all of this
KHASH_MAP_INIT_INT64(s2n, ssize)

struct Level{
	vlong noff;
	usize eoff;
	usize nnodes;
	usize nedges;
};
struct Coarse{
	Level *levels;
	int level;	/* deepest level of any one expansion */
	khash_t(s2n) *s2n;
	EM *fimap;
};

void	remapnode(Graph*, usize, usize, usize, usize);
void	unloadlevels(Graph*, int, int);
void	loadlevels(Graph*, int, int);
int	setgraphdepth(Graph*, int);
void	nukeindex(Graph*);
Coarse*	initindex(Graph*);
