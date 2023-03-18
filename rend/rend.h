typedef struct Render Render;

struct Render{
	kvec_t(Shape) shapes;
};

void	initrend(void);
Render	render(Graph *, Layer *);
