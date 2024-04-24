#include "strpg.h"
#include "graph.h"
#include "drw.h"
#include "threads.h"

static inline int
faceyourfears(Graph *g, Node *u, ssize *ep, float *res, int rev)
{
	int n;
	float θ;
	ssize *ee;
	Edge *e;
	Node *v;
	Vertex Δ;

	ee = ep + dylen(ep);
	if(ep >= ee)
		return -1;
	for(n=0, θ=0.; ep<ee; ep++){
		if((e = getedge(g, *ep)) == nil){
			warn("rend: dangling edge reference in adjacency array\n");
			continue;
		}
		v = getnode(g, e->v >> 1);
		assert(v != nil);
		if(u == v){
			u = v;
			v = getnode(g, e->u >> 1);
			if(u == v)
				continue;
		}
		Δ = rev ? subv(u->pos, v->pos) : subv(v->pos, u->pos);
		θ += atan2(Δ.y, Δ.x);
		n++;
	}
	if(n == 0)
		return -1;
	*res = (rev ? -θ : θ) / n;
	return 0;
}

static inline float
setzangle(Graph *g, Node *u)
{
	float θ;

	/* face outgoing */
	if(faceyourfears(g, u, u->out, &θ, 0) < 0
	/* face away from incoming */
	&& faceyourfears(g, u, u->in, &θ, 1) < 0)
		return 0;
	θ -= PI / 4;	/* upper left to bottom right corner */
	while(θ >= 2*PI)
		θ -= 2*PI;
	while(θ < 0)
		θ += 2*PI;
	return θ;
}

static inline void
shapenode(Graph *g, Node *u)
{
	u->rot.z = setzangle(g, u);
}

static void
shapegraph(Graph *g)
{
	ssize i;
	Node *u;

	for(i=g->node0.next; i>=0; i=u->next){
		u = g->nodes + i;
		shapenode(g, u);
	}
}

int
reshape(int force)
{
	int r;
	Graph *g;
	static Clk clk = {.lab = "shape"};

	r = 0;
	CLK0(clk);
	lockgraphs(0);
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type == FFdead || (g->flags & GFlayme) == 0)
			continue;
		if((g->flags & GFdrawme) != 0 || force){
			if(g->layout == nil){
				DPRINT(Debugrender, "reshape: no layout");
				continue;
			}
			if(dylen(g->nodes) < 1){
				DPRINT(Debugrender, "reshape: empty graph");
				continue;
			}
			shapegraph(g);
			r = 1;
		}
	}
	unlockgraphs(0);
	CLK1(clk);
	return r;
}
