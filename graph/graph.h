typedef struct Special Special;

ioff	pushinode(Graph*, ioff);
ioff	pushiedge(Graph*, ioff, ioff);
int	mergenodes(Graph*, ioff, ioff);

ioff	str2idx(char*);

enum{
	TCL,
	TLN,	/* reevaluated during layouting */
	Tfx,
	Tfy,
	Tfz,
	Tx0,
	Ty0,
	Tz0,
	Tnil,

	Tlayout = Tfx,	/* following affect layout */
};
struct Special{
	char *tag;
	char *function;
};
extern Special specials[Tnil];
void	setattr(Node*, ioff, int, char*);
void	setnamedtag(char*, char*, char*);

ioff	newnode(Graph*, char*);
ioff	newedge(Graph*, ioff, ioff, int, int, char*, ushort*);
void	lockgraph(Graph*, int);
void	unlockgraph(Graph*, int);
Graph*	pushgraph(Graph*);
void	initgraph(Graph*, int);
