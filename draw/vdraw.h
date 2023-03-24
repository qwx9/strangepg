typedef struct View View;

enum{
	Vdefw = 800,
	Vdefh = 600,
};
struct View{
	int w;
	int h;
	Vertex pan;
	double zoom;
};
extern View view;

int	initvdraw(void);
int	resetdraw(Graph*, Render*);
int	updatedraw(Graph*, Render*);
int	redraw(Graph*, Render*);
