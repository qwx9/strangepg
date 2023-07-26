#include "strpg.h"
#include "fs.h"

/* iterative edge agglomeration: coarsen by coloring neighbors on a first
 * come first serve basis, starting from low degree nodes first, merging
 * edges between nodes of the same color, starting over on each
 * iteration.  avoids any memory overhead by overwriting the edge and
 * node lists for each subsequent iteration: using the u,v edge list
 * sorted by degree and by u, for each u, each u,v is either an external
 * edge and must be preserved (v has already been assigned a color and
 * cannot be merged), or an internal one which must be removed (v is now
 * part of u).  this implicitely performs relabeling of merged nodes into
 * a new supernode and avoids going through every v's adjacencies to
 * update their offset: all old v nodes point to u and the remaining ones
 * can later be rechecked with no extra cost or effort.  more general and
 * much simpler method than first approach.
 */

static int external;

typedef struct Index Index;
typedef struct IGraph IGraph;
enum{
	Minedges = 0,
};
struct IGraph{
	u64int nnodes;
	u64int nedges;
	vlong idxoff;
	vlong degoff;
	vlong edgeoff;
	u64int *nodei;
	u64int *super;
	u32int *weight;
	u32int *degree;
	u64int *edge;
};
static IGraph graph;

/* everything is 1-indexed because of awk... */
static void
loadgraph(IGraph *g, File *f)
{
	u64int i, *t, *e, *s;
	u32int *w, *d;

	dyprealloc(g->nodei, g->nnodes);
	dyprealloc(g->super, g->nnodes);
	dyprealloc(g->degree, g->nnodes);
	dyprealloc(g->weight, g->nnodes);
	dyprealloc(g->edge, g->nedges);
	seekfs(f, g->idxoff);
	for(t=g->nodei, e=t+dylen(g->nodei), w=g->weight, s=g->super, i=0; t<e;){
		*w++ = 1;
		*s++ = i++;
		*t++ = get64(f) - 1;
	}
	for(t=g->nodei, e=t+dylen(g->nodei), d=g->degree; t<e;)
		d[*t++] = get32(f);
	for(t=g->edge, e=t+dylen(g->edge); t<e;)
		*t++ = get64(f) - 1;
}

static void
readidx(IGraph *g, File *f)
{
	g->nnodes = get64(f);
	g->nedges = get64(f);
	g->idxoff = tellfs(f);
	g->degoff = g->idxoff + g->nnodes * sizeof(u64int);
	g->edgeoff = g->degoff + g->nnodes * sizeof(u64int);
}

static void
usage(void)
{
	warn("usage: %s [SORTED_EDGES]\n", argv0);
	sysfatal("usage");
}

int
main(int argc, char **argv)
{
	int alreadymerged, hasedges, issuper, d, oldd;
	u64int i, u, v, t, e, ee, s, newi, ne, nn, newe, news, olds;
	File f = {0};
	IGraph *g;

	ARGBEGIN{
	case 'D': debug = -1UL; break;
	case 'e': external = 1; break;
	}ARGEND
	if(*argv == nil)
		usage();
	if(openfs(&f, *argv, OREAD) < 0)
		sysfatal("openfs: %r");
	g = &graph;
	readidx(g, &f);
	if(!external)
		loadgraph(g, &f);
	closefs(&f);
	if(external)
		sysfatal("no external memory implementation to hand");
	olds = g->nnodes;
	news = olds;	// FIXME: use pointers maybe
	ne = g->nedges;
	nn = g->nnodes;
	while(ne > Minedges){
		if(debug){
			for(i=0; i<g->nnodes; i++){
				u = g->nodei[i];
				warn("∗ node %llux\tu=%llux\ts=%llux\tdeg %d\n",
					i, u, g->super[u], g->degree[u]);
			}
			for(i=0, e=0; e<ne; i++){
				u = g->nodei[i];
				for(ee=e+g->degree[u]; e<ee; e++){
					v = g->edge[e];
					warn("∗ edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
						u, v, g->super[v], e, g->degree[v]);
				}
			}
		}
		newi = newe = 0;
		for(e=0, i=0; e<ne; i++){
			u = g->nodei[i];
			s = g->super[u];
			dprint(Debugtheworld, "- check node %llux\tu=%llux\ts=%llux\tdeg %d\n",
					i, u, g->super[u], g->degree[u]);
			oldd = g->degree[u];
			g->degree[u] = 0;
			alreadymerged = issuper = hasedges = 0;
			if(s >= olds){
				dprint(Debugtheworld, "\t→ add missing edges from node %llux already part of super %llux\n",
					u, s);
				hasedges = issuper = 1;
			}
			for(d=0; d<oldd; d++){
				v = g->edge[e+d];
				t = g->super[v];
				dprint(Debugtheworld, "\t→ edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
					u, v, t, e+d, g->degree[v]);
				if(alreadymerged && v < u){
					dprint(Debugtheworld, "\t→ skipping redundant edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
						u, v, t, e+d, g->degree[v]);
					continue;
				}
				if(issuper++ == 0){
					s = news++;
					warn("#%llux,%d(%llux)::", u, g->weight[u], s);
					g->super[u] = s;
				}
				if(t >= olds){
					if(hasedges++ == 0)
						g->nodei[newi++] = u;
					g->edge[newe++] = v;
					g->degree[u]++;
					dprint(Debugtheworld, "\t\tadd ext edge %llux,%llux (u=%llux)\n",
						s, t, u);
					continue;
				}
				g->weight[u] += g->weight[v];
				g->super[v] = s;
				dprint(Debugtheworld, "\t\tremove int edge %llux,%llux (u=%llux)\n",
					s, t, u);
				warn("%llux,%d:", v, g->weight[v]);
				nn--;
			}
			e += d;
			warn("|");
		}
		olds = news;
		ne = newe;
		warn("\n");
	}
	if(debug){
		warn("threshold reached, remaining:\n");
		for(i=0, e=0; i<nn; i++){
			u = g->nodei[i];
			for(ee=e+g->degree[u]; e<ee; e++){
				v = g->edge[e];
				warn("∗ node u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
					u, v, g->super[v], e, g->degree[v]);
			}
		}
		for(i=0, e=0; e<ne; i++){
			u = g->nodei[i];
			for(ee=e+g->degree[u]; e<ee; e++){
				v = g->edge[e];
				warn("∗ edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
					u, v, g->super[v], e, g->degree[v]);
			}
		}
	}
	sysquit();
	return 0;
}
