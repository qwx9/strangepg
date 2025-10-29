int	exportct(char*);
void	printgraph(void);
void	explode(ioff, float);

void	expandall(void);
int	coarsen(void);
int	uncoarsen(void);
int	commit(int);
int	expand(ioff, int);
int	collapseup(ioff*, ssize);
int	collapsedown(ioff*);
ioff*	collapseall(void);
ioff*	pushcollapseop(ioff, ioff*);
void	armgraph(void);
void	initct(void);

ioff	getnodeidx(ioff);
ioff	getrealid(ioff);
int	resetcolor(ioff);

/* FIXME: should be voff or usize */
extern ioff nnodes, nedges;		/* totals */
