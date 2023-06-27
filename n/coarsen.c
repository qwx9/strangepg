#include "strpg.h"
#include <thread.h>
#include <bio.h>

/* given a binary index (see format.rc), apply multilevel coarsening
 * heuristics to generate a hierarchy */
/* note: could shrink file size by not using 64 bits: use segments, or
 * variable-width integers encoding */

// FIXME: reuse strpg code: fs functions, etc.

int noui;

void
warn(char* fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprint(2, fmt, arg);
	va_end(arg);
}

static int external;

typedef struct EMgraph EMgraph;
typedef struct Deg Deg;
typedef struct Edge Edge;
struct Edge{
	usize u;
	usize v;
};
struct Deg{
	int deg;
	vlong off;
	usize idx;
	usize n;
	Edge *edges;
};
struct EMgraph{
	usize nnodes;
	usize nedges;
	int ndegs;
	vlong fsize;
	vlong nodeoff;
	vlong edgeoff;
	vlong degsoff;
	Deg *deg;
};
static EMgraph graph;

/* merge two adjacent nodes into a supernode */
static int
merge(Edge *e)
{
	// FIXME: ...
}

/* merge in an non-overlapping fashion ≤2-degree nodes:
 *	-u-v- → -S-; =a-u-v-b- → =a-S-b- (=a-S´- on subsequent iteration) */
static vlong
unchop(void)
{
	usize u;
	Edge *e;
	Deg *d;

	// FIXME: from the set of ≤ 2-degree nodes' edges, merge each
	//	non-overlapping only: a-b-c-d: merge a-b, c-d or b-c only
	//	say we merge b-c: we cannot merge _,b or b,_
	//	b,_ is easy, just skip to next
	//  _,b: skip if node id < current
	//  b is degree ≤ 2, meaning it would've been in the list already
	for(d=graph.degs; d->deg<=2; d++){
		for(e=d->edges; e<d->edges+d->ne;){
			u = e->u;
			/* if u is a 2-degree node, we'd have seen it already */
			if(d->deg == 1 || e->v >= u)
				merge(e++);
			/* skip any other u,_ edges */
			while(e->u == u)
				e++;
		}
	}
}

static vlong
burst(void)
{
	// FIXME: from the set of 3-degree nodes, find 3-2-3 patterns
}

static vlong
fuse(void)
{
	// FIXME: from the highest degree set, fuse all non-overlapping neighborhoods
}

/* FIXME: ... lookup() function: EM or not; transparent */
/* FIXME: cmd line option to use EM, default read all in */
/* FIXME: input validation */

// FIXME: sort -ud doesn't enforce this, use the old formulation for $1, $2!
/* edges u,v are sorted by u's degree, then u's id, then v's id */
static int
loadgraph(Biobuf *bf, EMgraph *g)
{
	usize *u, *uu;
	Deg *d;
	Edge *e, *ee;

	// FIXME: external: do the same, just don't load the actual edges or nodes
		// so we end up with a bucket array
		// each bucket has a file offset to nodes[] and a size
		// for intermediate results, we'd have to have a second temp file maybe
		// have to think about this; the stupidest approach would be to copy
		// the input and modify this one in place; better would be to have a
		// map, maybe as an additional layer, that remaps nodes/edges after
		// merges

	Bseek(bf, g->degsoff, 0);
	dyprealloc(g->degs, g->ndegs);
	p = d = g->degs;
	while(ndeg-- > 0){
		d->deg = get32(bf);
		d->idx = get64(bf);
		d->off = g->edgeoff + d->idx * sizeof *d->edges;
		if(d > g->degs){
			p->ne = d->idx - p->idx;
			p->nn = p->ne / MAX(p->deg, 1);
		}
		p = d++;
	}
	if(d > p){
		p->ne = graph.nedges - p->idx;
		p->nn = p->ne / MAX(p->deg, 1);
	}
	if(external)
		return 0;
	Bseek(bf, g->edgeoff, 0);
	// FIXME: recheck
	for(d=g->degs, u=0; d<g->degs+g->ndegs; d++){
		dyprealloc(d->edges, d->ne);
		for(uu=u+d->nn, e=d->edges; u<uu; u++)
			for(ee=e+d->deg; e<ee; e++){
				e->u = u;
				e->v = get64(bf);
			}
	}
	return 0;
}

/* FIXME: use fs functions */
static int
readdict(Biobuf *bf)
{
	vlong sz;

	graph.fsize = Bseek(bf, 0, 2);
	Bseek(bf, -3 * sizeof(vlong), 1);
	graph.nnodes = get64(bf);
	graph.nedges = get64(bf);
	graph.ndegs = get64(bf);
	graph.nodeoff = 0;
	graph.edgeoff = graph.nnodes * sizeof(usize);
	graph.degsoff = graph.edgeoff + graph.edges * sizeof(usize);
	Bseek(bf, graph.nodeoff, 0);
	return 0;
}

static void
usage(void)
{
	fprint(2, "usage: %s [SORTED_EDGES]\n", argv0);
	sysfatal("usage");
}

void
threadmain(int argc, char **argv)
{
	char *s, cur[64] = {0}, *fld[2];
	usize u, v, w, n;
	uchar e[16];
	Htab *h;
	Biobuf *bf;

	ARGBEGIN{
	}ARGEND
	if(*argv == nil)
		usage();
	if((bf = Bopen(path, OREAD)) == nil)
		return -1;
	if(readdict(bf) < 0)
		sysfatal("readdict: %r");
	if(!external && loadgraph(bf) < 0)
		sysfatal("loadgraph: %r");
	// FIXME: lookup functions
	if(external)
		sysfatal("no external memory implementation to hand");
	while(graph.nnodes > Minnodes){
		if(unchop())
			continue;
		if(burst())
			continue;
		if(fuse())
			continue;
		fprint(2, "premature exit with %zd > %d min node threshold\n",
			graph.nnodes, Minnodes);
	}
	Bterm(bf);
	threadexits(nil);
}
