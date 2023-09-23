#include "strpg.h"
#include "fs.h"
#include "em.h"

enum{
	Minedges = 0,
	IOunit = 64*1024,
};
static u64int TOTN, N, M, S, L_M, NL;
static int tmpuuid;	/* FIXME: cleanup */
static char prefix[64];
static File *noutf, *eoutf, *loutf;
static uchar iobuf[IOunit];
static EM *fedgeuv, *fedge, *fnode, *fweight, *flast, *flastm;

static int
reverse(Graph *g)
{
	int i, x;
	vlong noff, eoff;
	u64int n, m;
	File *f, *fi;

	f = emalloc(sizeof *f);	// FIXME: unfuck the fucking api
	if(fdopenfs(f, 1, OWRITE) < 0)
		sysfatal("fdopenfs: %r");
	fi = emalloc(sizeof *f);
	put64(f, S+1);
	put64(f, g->nedges);
	put64(f, NL);
	noff = tellfs(f);
	// FIXME: isn't TOTN just S+1?
	eoff = noff + TOTN * 4 * sizeof(u64int);
	// FIXME: dict could probably stay in memory, it's just 2*8 bytes per level
	for(i=NL-1; i>=0; i--){
		snprint(prefix, sizeof prefix, ".%x_coarse_%08x_0", tmpuuid, i);
		if(openfs(fi, prefix, OREAD) < 0)
			sysfatal("openfs %s: %r", prefix);
		n = get64(fi);
		m = get64(fi);
		closefs(fi);	/* FIXME: fix this api */
		free(fi->path);
		fi->path = nil;
		sysremove(prefix);
		put64(f, noff);
		put64(f, eoff);
		put64(f, n);
		put64(f, m);
		noff += n * 4 * sizeof(u64int);
		eoff += m * 1 * sizeof(u64int);
	}
	for(x=1; x<3; x++)
		for(i=NL-1; i>=0; i--){
			snprint(prefix, sizeof prefix, ".%x_coarse_%08x_%d", tmpuuid, i, x);
			if(openfs(fi, prefix, OREAD) < 0)
				sysfatal("openfs %s: %r", prefix);
			while((n = readfs(fi, iobuf, sizeof iobuf)) > 0)
				writefs(f, iobuf, n);
			closefs(fi);
			free(fi->path);
			fi->path = nil;
			sysremove(prefix);
		}
	freefs(f);
	return 0;
}

static void
endlevel(void)
{
	uchar u[2*sizeof(u64int)], *p;

	// FIXME: wtf why do we do this, we have put64 and emput64
	p = u;
	PBIT64(p, N);
	p += sizeof(u64int);
	PBIT64(p, L_M);
	writefs(loutf, u, sizeof u);
	closefs(loutf);
	free(loutf->path);
	closefs(noutf);
	free(noutf->path);
	closefs(eoutf);
	free(eoutf->path);
	emnuke(flast);
	emshrink(fedge, M*8);
	eoutf->path = noutf->path = loutf->path = nil;
}
static void
outputlevel(void)
{
	int i;
	u64int m, u, v, e;

	dprint(Debugcoarse, "ending level with N=%lld M=%lld\n", N, M);
	endlevel();
	for(i=0; i<M; i++){
		e = emget64(fedge, i*8);
			m = e * 8*2 + 2*8;
			u = emget64(fedgeuv, m);
			v = emget64(fedgeuv, m+8);
		warn("E[%d] %lld → %lld,%lld\n", i, e, u, v);
	}
}

static void
outputedge(u64int e, u64int u, u64int v, u64int s, u64int t)
{
	uchar buf[1*sizeof(u64int)], *p;

	p = buf;
	PBIT64(p, e);
	writefs(eoutf, buf, sizeof buf);
	dprint(Debugcoarse, "discard edge i=%lld mapping to %llx,%llx (u,v %llx,%llx)\n", e, s, t, u, v);
	L_M++;
}

static void
outputnode(u64int u, u64int s, u64int s´, u64int w)
{
	uchar buf[4*sizeof(u64int)], *p;

	p = buf;
	PBIT64(p, u);
	p += sizeof(u64int);
	PBIT64(p, s);
	p += sizeof(u64int);
	PBIT64(p, s´);
	p += sizeof(u64int);
	PBIT64(p, w);
	writefs(noutf, buf, sizeof buf);
	dprint(Debugcoarse, "merge node %llx (%llx) → %llx, weight %lld\n", u, s, s´, w);
	N++;
	TOTN++;
}
static void
outputsuper(u64int u, u64int s, u64int s´, u64int w)
{
	dprint(Debugcoarse, "new supernode %llx: ", s);
	outputnode(u, s, s´, w);
}

static void
newlevel(void)
{
	int i;
	u64int m, u, v, e;

	for(i=0; i<M; i++){
		e = emget64(fedge, i*8);
			m = e * 8*2 + 2*8;
			u = emget64(fedgeuv, m);
			v = emget64(fedgeuv, m+8);
		warn("E[%d] %lld → %lld,%lld\n", i, e, u, v);
	}
	N = M = L_M = 0;
	warn("\t>> NEW LEVEL %lld\n", NL+1);
	snprint(prefix, sizeof prefix, ".%x_coarse_%08llx_X", tmpuuid, NL++);
	/* FIXME: yuck */
	prefix[strlen(prefix)-1] = '0';
	if(openfs(loutf, prefix, OWRITE) < 0)
		sysfatal("newlevel %s: %r", prefix);
	prefix[strlen(prefix)-1] = '1';
	if(openfs(noutf, prefix, OWRITE) < 0)
		sysfatal("newlevel %s: %r", prefix);
	prefix[strlen(prefix)-1] = '2';
	if(openfs(eoutf, prefix, OWRITE) < 0)
		sysfatal("newlevel %s: %r", prefix);
}

static void
outputtop(u64int u, u64int s, u64int s´, u64int w)
{
	newlevel();
	outputsuper(u, s, s´, w);
	outputlevel();
}

int
coarsen(Graph *g, char *index)
{
	int i, wu, wv;
	u64int w, u, v, s, t, m, e, a, b;
	ssize top;

	// FIXME: separate functions
	//fedgeuv = emclone(index, 2*8, 2 * g->nedges * sizeof(u64int));
	if((fedgeuv = emopen(index)) == nil)
		sysfatal("coarsen: %r");
	fweight = emcreate(g->nnodes * sizeof(u64int));
	fnode = emcreate(g->nnodes * sizeof(u64int));
	fedge = emcreate(g->nedges * sizeof(u64int));
	/* FIXME: these might never get big enough to need to be in external memory */
	flastm = emcreate(g->nnodes * sizeof(u64int));
	flast = emcreate(g->nnodes * sizeof(u64int));
	warn("N %lld M %lld\n", N, M);
	for(i=0; i<g->nnodes; i++){
		emput64(fweight, i*8, 1);
		emput64(fnode, i*8, i);
	}
	for(i=0; i<g->nedges; i++)
		emput64(fedge, i*8, i);
	warn("N %lld M %lld\n", g->nnodes, g->nedges);
	N = g->nnodes;
	M = g->nedges;
	S = N - 1;
	w = S;
	top = -1;	/* cannot happen */
	s = wu = 0;	/* cannot happen */
	while(M > 0){
		m = M;
		newlevel();
		/* FIXME: optimize (later) for less seeking/jumping around */
		for(i=0; i<m; i++){
			e = emget64(fedge, i*8);
			m = e * 8*2 + 2*8;
			u = emget64(fedgeuv, m);
			v = emget64(fedgeuv, m+8);
			warn("getnode %d %zd %zd %zd\n", i, e, u, v);
			s = emget64(fnode, u*8);
			/* unvisited node: make new supernode */
			if(s <= w){
				if(top >= 0)
					emput64(fweight, top*8, wu);
				top = u;
				S++;
				wu = emget64(fweight, u*8);
				outputsuper(u, s, S, wu);
				s = S;
				emput64(flastm, (s-g->nnodes)*8, S);
				emput64(flast, (s-w)*8, i);
				emput64(fnode, u*8, s);
			}
			t = emget64(fnode, v*8);
			a = 0;
			if(t > w)
				a = emget64(flastm, (t-g->nnodes)*8);
			/* unvisited adjacency or self: merge internal edges */
			warn("check for redundancy: top %lld u %lld v %lld s %lld t %lld w %lld\n", top, u, v, s, t, w);
			if(t <= w && a <= w || t == s && u != v){
				/* edges not starting from the top node are mirrors, skip them */
				if(u == top){
					wv = emget64(fweight, v*8);
					outputnode(v, t, s, wv);
					wv += wu;
					wu = wv;
					emput64(fweight, v*8, wv);
					emput64(fnode, v*8, s);
					outputedge(e, u, v, s, t);
				}
			/* adjacency previously merged elsewhere: fold external edges */
			}else{
				a = emget64(flast, (t-w)*8);
				b = emget64(flast, (s-w)*8);
				warn("check for redundancy 2: a %lld b %lld NL %lld\n",
					a, b, NL);
				if(a >= b && (u != v || NL > 1)){
					/* already retained one edge, discard following redundant ones */
					dprint(Debugcoarse, "discarding redundant edge: t:%llx >= s:%llx\n", a, b);
					outputedge(e, u, v, s, t);
				}else{
					/* retain edge for next round */
					//a = emget64(fedge, i*8);
					emput64(fedge, M*8, e);
					M++;
					dprint(Debugcoarse, "retain edge[%x] %lld,%lld at %llx slot %lld\n", i, s, t, a, M);
					emput64(flast, (t-w)*8, i);
				}
			}
		}
		outputlevel();
		w = S;
	}
	/* add one node to rule them all if none remain */
	if(M > 0)
		dprint(Debugcoarse, "stopping at %lld remaining edges\n", M);
	if(N > 0){
		dprint(Debugcoarse, "push artificial node to rule all remaining");
		outputtop(top, s, ++S, wu);
	}
	emclose(fedgeuv);
	emclose(fnode);
	emclose(fweight);
	emclose(fedge);
	emclose(flast);
	emclose(flastm);
	return 0;
}

static int
readindex(Graph *g, char *path)
{
	File *f;

	f = emalloc(sizeof *f);
	if(openfs(f, path, OREAD) < 0)
		sysfatal("readindex: %r");
	g->nnodes = get64(f);
	g->nedges = get64(f);
	warn("N %lld M %lld\n", g->nnodes, g->nedges);
	freefs(f);
	return 0;
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
	char *s;
	Graph g;

	ARGBEGIN{
	case 'D':
		s = EARGF(usage());
		if(strcmp(s, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(s, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(s, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(s, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(s, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(s, "all") == 0)
			debug |= Debugtheworld;
		else{
			warn("unknown debug component %s\n", s);
			usage();
		}
		break;
	}ARGEND
	if(argc < 1)
		usage();
	if(readindex(&g, argv[0]) < 0)
		sysfatal("readindex: %r");
	srand(time(nil));
	tmpuuid = nrand(1000000);
	loutf = emalloc(sizeof *loutf);
	noutf = emalloc(sizeof *noutf);
	eoutf = emalloc(sizeof *eoutf);
	if(coarsen(&g, argv[0]) < 0)
		sysfatal("coarsen: %r");
	if(reverse(&g) < 0)
		sysfatal("reverse: %r");
	return 0;
}
