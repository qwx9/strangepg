ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	str2idx(char*);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*);
void	lockgraph(Graph*, int);
void	unlockgraph(Graph*, int);
void	nukegraph(Graph*);
void	pushgraph(Graph*);
Graph	initgraph(int);
