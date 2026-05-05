int	exportct(char*);
void	printgraph(void);
void	explode(ioff, float);
int	expand(ioff, int);
void	expandall(int);
int	collapse(ioff, int);
int	collapseall(int);
int	coarsen(void);
int	uncoarsen(void);
void	armgraph(void);
void	initct(void);

uint	getnodelength(voff);

ioff	getnodeidx(ioff);
ioff	getrealid(ioff);

/* FIXME: should be voff or usize */
extern ioff nnodes, nedges;		/* totals */

extern int cthresh;
