void	flushdraw_(void);
void	cleardraw_(void);
void	resetdraw_(void);
int initdraw_(void);

Vquad	scaletrans(Vquad, double, Vertex);
int	rotate(Vquad);
int	drawline(Vertex, Vertex, double);
int	drawline_(Vertex, Vertex, double);
int	drawquad(Vertex, Vertex);
int	drawquad_(Vertex, Vertex);
int	drawbezier(void);
