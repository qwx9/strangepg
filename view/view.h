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

void	initview(void);
void	resetview(void);
void	updateview(void);
void	show(Graph*, Render*);
