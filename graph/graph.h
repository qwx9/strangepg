int	exportct(char*);
void	printgraph(void);
void	explode(ioff);

int	setclength(Node*, int);
void	expandall(void);
int	coarsen(void);
int	uncoarsen(void);
int	commit(void);
int	expand(ioff);
int	collapseup(ioff*);
int	collapsedown(ioff*);
ioff*	collapseall(void);
ioff*	pushcollapseop(ioff, ioff*);
int	buildct(void);

ioff	getnodeidx(ioff);
ioff	getrealid(ioff);
u64int	getrealedge(ioff);
