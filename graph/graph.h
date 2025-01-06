ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	str2idx(char*);

int	setattr(Node*, ioff, int, char*);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*, ushort*);
void	lockgraph(Graph*, int);
void	unlockgraph(Graph*, int);
Graph*	pushgraph(Graph*);
void	initgraph(Graph*, int);
