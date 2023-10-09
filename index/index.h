typedef struct Level Level;

struct Level{
	vlong firstnode;	/* dynamic */
	vlong firstedge;	/* dynamic */
	vlong noff;
	usize eoff;
	usize nnodes;
	usize nedges;
};
struct Coarse{
	Level *levels;
	int level;	/* deepest level of any one expansion */
};

void	unloadlevels(Graph*, int, int);
void	loadlevels(Graph*, int, int);
int	setgraphdepth(Graph *, int);
Coarse*	initindex(Graph*);
