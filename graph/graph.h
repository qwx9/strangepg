typedef struct Vertex Vertex;
typedef struct Obj Obj;
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Edge Edge;

KHASH_MAP_INIT_INT64(usize, usize);

struct Graph{
	khash_t(usize) *eid;
	khash_t(usize) *nid;
	kvec_t(Edge) edges;
	kvec_t(Node) nodes;
};

struct Node{
	usize id;
	kvec_t(usize) in;
	kvec_t(usize) out;
};

struct Edge{
	usize u;
	usize v;
	double w;
};

Graph*	initgraph(Graph*);
void	addnode(Graph*, usize);
void	addedge(Graph*, usize, usize, usize, double);

struct Vertex{
	int x;
	int y;
};

extern Vertex ZV;
int	eqv(Vertex, Vertex);

extern Graph *lolgraph;
