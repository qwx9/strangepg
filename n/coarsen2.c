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
typedef struct Grøph Grøph;
typedef struct Løvel Løvel;
enum{
	Minedges = 0,
};
struct Løvel{
	u64int nnodes;
	u64int nedges;
	u64int nsuper;
	vlong off;
	vlong len;
};
struct Grøph{
	u64int nnodes;
	u64int nedges;
	vlong idxoff;
	vlong degoff;
	vlong edgeoff;
	Løvel *level;
	u64int *nodei;
	u64int *super;
	u32int *weight;
	u32int *degree;
	u64int *edge;
};
static Grøph graph;
static File inf, outf, emf, tmpf;

static void
printtab(Grøph *g, u64int nn, u64int ne)
{
	u64int i, u, v, e, ee;

	if((debug & Debugcoarse) == 0)
		return;
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

/* final output, reversing level order from coarsest to full graph.
 * output: hdr[] levels[]
 * hdr: nnodes[8] nedges[8] nsuper[8] nlevel[8] leveltab[]
 * leveltab: { offset[8] length[8] }[nlevel]
 * levels: { nlsupers[8] supers[] }
 * supers: { super[8] u[8] weight[4] deg[4] vlist[] }[nsupers]
 * vlist: { v[8] weight[4] }[deg]
 */
static void
reverse(char *tmppath, usize nextsuper, Grøph *g)
{
	int n, m, k;
	File f = {0}, tf = {0};
	uchar buf[64*1024];
	vlong doff, op, o;
	Løvel *l;

	// FIXME: we need external edges! ie. v,_

	if(openfs(&tf, tmppath, OREAD) < 0)
		sysfatal("fdopenfs stdout: %r");
	if(fdopenfs(&f, 1, OWRITE) < 0)
		sysfatal("openfs %s: %r", tmppath);
	put64(&f, g->nnodes);
	put64(&f, g->nedges);
	put64(&f, nextsuper - g->nnodes);
	put64(&f, dylen(g->level));
	doff = tellfs(&f);
	/* placeholders */
	for(l=g->level; l<g->level+dylen(g->level); l++){
		put64(&f, -1);
		put32(&f, 0);
	}
	op = tellfs(&f);
	vlong ass = 0;
	for(l=g->level; l<g->level+dylen(g->level); l++){
		/* level header */
		put64(&f, l->nsuper);
		/* level data */
		for(k=l->nsuper, n=l->len; k>0; k--){		// FIXME: don't have length
			m = n < sizeof buf ? n : sizeof buf;
			if(readfs(&tf, buf, m) <= 0)
				sysfatal("reverse: short read");
			ass += m;
			dprint(Debugcoarse, "level %zd: %lld bytes, %d so far\n",
				l-g->level, l->len, m);
			if(writefs(&f, buf, m) != m)
				sysfatal("reverse: short write");
		}
		/* update level index */
		o = tellfs(&f);
		seekfs(&f, doff);
		put64(&f, op);		/* offset */
		put32(&f, o - op);	/* length */
		doff = tellfs(&f);
		seekfs(&f, o);
		op = o;
	}
	closefs(&tf);
	closefs(&f);
}

static void
writetop(File *f, u64int u, u64int s, Grøph *g)
{
	vlong off;
	u32int w;
	Løvel l = {0};

	w = g->weight[u];
	dprint(Debugcoarse, "#%llux:%llux,%d::", u, s, w);
	put64(f, s);
	put64(f, u);
	put32(f, w);
	put32(f, 0);	/* placeholder for number of u,_ edges */
	off = tellfs(f);
	l.off = off;
	dypush(g->level, l);
}

static void
writechild(File *f, u64int v, Grøph *g)
{
	u32int w;

	w = g->weight[v];
	dprint(Debugcoarse, "%llux,%d:", v, w);
	put64(f, v);
	put32(f, w);
}

/* fill placeholder */
static void
writerecsize(File *f, u64int nedge, Grøph *g)
{
	vlong o;
	Løvel *l;

	dprint(Debugcoarse, "|");
	o = tellfs(f);
	l = g->level + dylen(g->level) - 1;
	seekfs(f, l->off - sizeof(u32int));
	put32(f, nedge);	/* can be 0 */
	seekfs(f, o);
}

static void
writelevel(File *f, int nsuper, int nnode, int nedge, Grøph *g)
{
	vlong off;
	Løvel *l;

	l = g->level + dylen(g->level) - 1;
	dprint(Debugcoarse, "writelevel %zd %d %d %d\n", l-g->level, nsuper, nnode, nedge);
	off = tellfs(f);
	l->len = off - l->off;
	seekfs(f, l->off - 4);
	put32(f, l->len / (8 + 4));
	assert(l->len % (8 + 4) == 0);
	seekfs(f, off);
	l->nedges = nedge;
	l->nnodes = nnode;
	l->nsuper = nsuper;
	dprint(Debugcoarse, "\n");
}

/* everything is 1-indexed because of awk... */
static void
loadgraph(Grøph *g, File *f)
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
readidx(Grøph *g, File *f)
{
	dprint(Debugcoarse, "readidx %s\n", f->path);
	g->nnodes = get64(f);
	g->nedges = get64(f);
	g->idxoff = tellfs(f);
	g->degoff = g->idxoff + g->nnodes * sizeof(u64int);
	g->edgeoff = g->degoff + g->nnodes * sizeof(u64int);
	dprint(Debugcoarse, "read %llud nodes %llud edges\n",
		g->nnodes, g->nedges);
}

static void
usage(void)
{
	warn("usage: %s SORTED_EDGES\n", argv0);
	sysfatal("usage");
}

int
main(int argc, char **argv)
{
	int alreadymerged, hasedges, issuper, d, oldd;
	u64int i, u, v, t, e, s, newi, ne, nn, newe, news, olds, newd;
	char *a, *path;
	File f = {0}, tf = {0};
	Grøph g = {0};

	ARGBEGIN{
	case 'D':
		a = EARGF(usage());
		if(strcmp(a, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(a, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(a, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(a, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(a, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(a, "all") == 0)
			debug |= Debugtheworld;
		else{
			warn("unknown debug component %s\n", a);
			usage();
		}
		break;
	case 'e': external = 1; break;
	}ARGEND
	if(argc < 1)
		usage();
	if(openfs(&f, argv[0], OREAD) < 0)
		sysfatal("openfs: %r");
	readidx(&g, &f);
	if(external)
		sysfatal("no external memory implementation to hand");
	else
		loadgraph(&g, &f);
	if(opentmpfs(&tf) < 0)
		sysfatal("fdopenfs: %r");
	path = tf.path;
	olds = g.nnodes;
	news = olds;	// FIXME: use pointers maybe
	ne = g.nedges;
	nn = g.nnodes;
	while(ne > Minedges){
		printtab(&g, nn, ne);
		newi = newe = 0;
		for(e=0, i=0; i<nn; i++){
			u = g.nodei[i];
			oldd = g.degree[u];
			d = g.degree[u] = 0;
			s = g.super[u];
			dprint(Debugtheworld, "- check node %llux\tu=%llux\ts=%llux\tdeg %d\n",
					i, u, g.super[u], g.degree[u]);
			alreadymerged = issuper = hasedges = 0;
			if(s >= olds){
				// FIXME: does this work??
				dprint(Debugtheworld, "\t→ add missing edges from node %llux already part of super %llux\n",
					u, s);
				alreadymerged = hasedges = issuper = 1;
			}
			for(newd=0; d<oldd; d++){
				v = g.edge[e+d];
				t = g.super[v];
				dprint(Debugtheworld, "\t→ edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
					u, v, t, e+d, g.degree[v]);
				if(alreadymerged && v < u){
					dprint(Debugtheworld, "\t→ skipping redundant edge u=%llux\tv=%llux\tt=%llux\te=%llux\tdeg %d\n",
						u, v, t, e+d, g.degree[v]);
					continue;
				}
				if(issuper++ == 0){
					s = news++;
					writetop(&tf, u, s, &g);
					g.super[u] = s;
				}
				if(t >= olds){
					if(hasedges++ == 0)
						g.nodei[newi++] = u;
					g.edge[newe++] = v;
					g.degree[u]++;
					dprint(Debugtheworld, "\t\tadd ext edge %llux,%llux (u=%llux)\n",
						s, t, u);
					continue;
				}
				g.weight[u] += g.weight[v];
				g.super[v] = s;
				dprint(Debugtheworld, "\t\tremove int edge %llux,%llux (u=%llux)\n",
					s, t, u);
				writechild(&tf, v, &g);
				newd++;
			}
			/* child can be just u, or u and some adjacencies */
			if(issuper)
				writerecsize(&tf, newd, &g);
			e += d;
		}
		writelevel(&tf, news - olds, newi - nn, newe - ne, &g);
		olds = news;
		ne = newe;
		nn = newi;
	}
	if(ne > 0)
		dprint(Debugcoarse, "threshold reached\n");
	printtab(&g, nn, ne);
	dprint(Debugtheworld, "wrote %lld bytes\n", tellfs(&tf));
	closefs(&tf);
	closefs(&f);
	reverse(path, news, &g);
	free(path);
	return 0;
}
