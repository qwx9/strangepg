typedef struct Layout Layout;
typedef struct Layer Layer;
typedef struct Vnode Vnode;
typedef struct Vedge Vedge;

struct Vedge{
	usize id;
	Vertex u;
	Vertex v;
	double w;
};

struct Vnode{
	usize id;
	Vertex v;
	double w;
};

enum{
	LLconga,
	LLnil,
};
struct Layout{
	char *name;
	Layer (*compute)(Graph*);
};
struct Layer{
	Layout *ll;
	kvec_t(Vnode) nodes;
	kvec_t(Vedge) edges;
};

void	initlayout(void);
Layer	dolayout(Graph*, int);

extern Layer ZL;

extern Layer lolayer;
