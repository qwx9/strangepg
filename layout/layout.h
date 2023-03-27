typedef struct Layout Layout;
typedef struct Layer Layer;
typedef struct Vnode Vnode;

struct Vnode{
	usize id;
	Vquad q;
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
};

void	initlayout(void);
Layer	dolayout(Graph*, int);

extern Layer ZL;

extern Layer lolayer;
