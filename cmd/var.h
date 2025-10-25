typedef struct Core Core;

struct Core{
	char **labels;
	u32int *colors;
	Array *strs;	/* FIXME: val could be a refcount to allow freeing */
	Array *ptrs;
	Array *eptrs;
	Array *ids;
	Array *label;
	Array *length;
	Array *degree;
	Array *color;
	Array *sel;		/* FIXME: actually int hashset */
	Array *fx;
	Array *fy;
	Array *fz;
	Array *x0;
	Array *y0;
	Array *z0;
	Cell *csel;
};
extern Core core;

void	setnodeinitz(size_t, Value);
void	setnodeinity(size_t, Value);
void	setnodeinitx(size_t, Value);
void	setnodefixedz(size_t, Value);
void	setnodefixedy(size_t, Value);
void	setnodefixedx(size_t, Value);
void	updatenodelength(ioff, uint);
uint	getnodelength(voff);
void	setnodelength(size_t, Value);
void	setnodecolor(size_t, Value);
void	deselectnode(Cell*);
