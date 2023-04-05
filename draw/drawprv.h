void	flushdraw_(void);
void	cleardraw_(void);
int	resetdraw_(void);
int initdraw_(void);

Quad	scaletrans(Quad, double, Vertex);
int	rotate(Quad);
int	drawline(Quad, double);
int	drawline_(Quad, double);
int	drawquad(Quad);
int	drawquad_(Quad);
int	drawbezier(void);
char*	quadfmt_(Quad*);
char*	vertfmt_(Vertex*);
