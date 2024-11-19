ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	str2idx(char*);

enum{
	TLN,
	Tfx,
	Tfy,
	Tfz,
	Tx0,
	Ty0,
	Tz0,
};
void	setattr(Node*, int, char*);
void	setnamedtag(char*, char*, char*);
void	settag(Graph*, ioff, char, char*, char*);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*, ushort*);
void	lockgraph(Graph*, int);
void	unlockgraph(Graph*, int);
Graph*	pushgraph(Graph*);
void	initgraph(Graph*, int);
