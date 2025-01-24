ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*, ushort*);
void	explode(ioff);
ioff	str2idx(char*);
Graph*	initgraph(int);
