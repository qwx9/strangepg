typedef struct Level Level;

KHASH_MAP_INIT_INT64(tosuper, ssize)	/* leafi → superi */
KHASH_MAP_INIT_INT64(toinode, ssize)	/* superi → nodei */

struct Level{
	vlong noff;
	usize eoff;
	usize nnodes;
	usize nedges;
};
struct Coarse{
	Level *levels;
	int level;	/* deepest level of any one expansion */
	khash_t(tosuper)	*supers;
	khash_t(toinode)	*inodes;
};

int	remapnode(Graph*, usize, ssize, ssize, usize);
void	loadlevel(Graph*, int, int);
int	setgraphdepth(Graph*, int);
void	clearindex(Graph*);
void	nukeindex(Graph*);
Coarse*	initindex(Graph*);
