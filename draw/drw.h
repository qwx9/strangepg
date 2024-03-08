struct Pal{
	uchar r;
	uchar g;
	uchar b;
	Color *c;
};

enum{
	Palsz = 6+9+12+6,
};
extern Pal *theme;

enum{
	Cbg,
	Ctext,
	Cnode,
	Cedge,
	Cemph,
	Cend,
};

int	drawline(Quad, double, int, int, Color*);
int	drawbezier(Quad, double, int, Color*);
int	drawquad(Quad, Quad, Quad, double, int, Color*);
int	drawlabel(Node*, Quad, Quad, Quad, vlong, Color*);
void	cleardraw(void);
Vertex	centerscalept2(Vertex);
Quad	centerscalequad(Quad);
/* FIXME: see notes in draw/color.c */
int	setnodecolor(Graph*, Node*, u32int);
Pal*	newcolor(u32int);
void	freecolor(Pal*);
Pal*	somecolor(Graph*);
void	somepalette(Graph*);
void	initcol(Graph*);
void	settheme(void);
void	initdrw(void);
