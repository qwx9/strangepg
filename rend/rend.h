typedef struct Render Render;

struct Render{
	kvec_t(Vnode) nodes;
	Vquad dim;
};

void	initrend(void);
Render	render(Graph *, Layer *);

extern Render lolrender;
