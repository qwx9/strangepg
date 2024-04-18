KHASH_MAP_INIT_INT(cmap, Color*)

extern khash_t(cmap) *cmap;
enum{
	Cbg,
	Ctext,
	Cnode,
	Cedge,
	Cemph,
	Cend,
};
extern u32int *theme;

extern int drawlabels;

int	drawline(Quad, double, int, s32int, Color*);
int	drawbezier(Quad, double, s32int, Color*);
int	drawquad(Quad, Quad, Quad, double, s32int, Color*);
int	drawlabel(Node*, Quad, Quad, Quad, vlong, Color*);
void	cleardraw(void);
Vertex	centerscalept2(Vertex);
Quad	centerscalequad(Quad);
void	zoomdraw(float);
void	pandraw(float, float);
int	setnodecolor(Graph*, Node*, u32int);
Color*	newcolor(u32int);
void	freecolor(Color*);
u32int	col2int(Color*);
Color*	color(u32int);
Color*	somecolor(Graph*);
void	settheme(void);
void	initdrw(void);
