typedef struct Graph Graph;

struct Graph{
	khash_t(usize) *eid;
	khash_t(usize) *nid;
	kvec_t(Edge) edges;
	kvec_t(Node) nodes;
};

Graph*	initgraph(Graph*);
void	addnode(Graph*, usize);
void	addedge(Graph*, usize, usize, usize, double);

extern Graph *lolgraph;
