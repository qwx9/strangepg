ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	str2idx(char*);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*);
void	nukegraph(Graph*);
void	pushgraph(Graph*);
void	lockgraphs(int);
void	unlockgraphs(int);
Graph	initgraph(int);
