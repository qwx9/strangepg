typedef struct Level Level;

struct Level{
	vlong noff;
	vlong eoff;
	usize nnodes;
	usize nedges;
};
struct Coarse{
	Level *levels;
};

int	readtree(Graph*, char*);

