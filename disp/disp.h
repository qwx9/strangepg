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

void	initdisp(void);
void	resetdisp(void);
void	updatedisp(void);
