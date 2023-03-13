typedef struct Layout Layout;
typedef struct Layer Layer;

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
	kvec_t(Vertex) nodes;
};

void	initlayout(void);
Layer	dolayout(Graph*, int);

extern Layer ZL;
