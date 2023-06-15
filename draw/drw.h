int	drawline(Quad, double, int);
int	drawbezier(Quad, double);
int	drawquad(Quad, double, int);
int	drawquad2(Quad, Quad, Quad, double, int, int);
void	cleardraw(void);
Vertex	centerscalept2(Vertex);
Quad	centerscalequad(Quad);

enum{
	Cbg,
	Ctext,
	Cnode,
	Cedge,
	Cemph,
	Cend,
};
typedef struct Color Color;
struct Color{
	int r;
	int g;
	int b;
};
extern Color theme1[Cend], theme2[Cend];
extern Color palette[];
extern int palsz;
