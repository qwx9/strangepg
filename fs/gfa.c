#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "graph.h"
#include "drw.h"
#include "cmd.h"
#include "layout.h"
#include "lib/khashl.h"

/* assumptions:
 * - required field separator is \t
 * - tags are two characters long as per the spec
 * - length of inlined sequence trumps LN tag unless '*'
 * - no empty fields
 */

KHASHL_SET_INIT(KH_LOCAL, edgeset, edges, u64int, kh_hash_uint64, kh_eq_generic)

typedef struct Aux Aux;
struct Aux{
	edgeset *edges;
	int *length;
	ushort *degree;
	vlong *nodeoff;
	vlong *edgeoff;
	ioff nnodes;
	ioff nedges;
};

static int
readnodetags(Aux *a, File *f)
{
	int nerr;
	char *s;
	ioff id;
	vlong off, *o, *oe;

	for(nerr=0, id=0, o=a->nodeoff, oe=o+dylen(o); o<oe; o++, id++){
		if((off = *o) < 0){
			DPRINT(Debugmeta, "readnodetags: [%d] off=%lld, skipping", id, off);
			continue;
		/* missing S records will have uninitialized offsets and parsing
		 * will fail since offset 0 won't correspond to a tag */
		}else if(off == 0){
			werrstr("readnodetags: missing node indexed at %d\n", id);
			return -1;
		}
		if(seekfs(f, off) < 0)
			warn("readnodetags: %s\n", error());
		if(readline(f) == nil)
			sysfatal("readnodetags: %s", error());
		while((s = nextfield(f)) != nil){
			DPRINT(Debugmeta, "readnodetags: [%d] %s", id, s);
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':' || f->toksz > 128){
				warn("invalid segment tag \"%s\"\n", s);
				if(nerr++ > 10){
					werrstr("too many errors");
					return -1;
				}
				continue;
			}
			s[2] = 0;
			settag(s, id, s+5);
			nerr = 0;
		}
	}
	return 0;
}

static int
readedgetags(Aux *a, File *f)
{
	int nerr;
	char *s;
	ioff id;
	vlong off, *o, *oe;

	if(status & FSnoetags)
		return 0;
	for(nerr=0, id=0, o=a->edgeoff, oe=o+dylen(o); o<oe; o++, id++){
		if((off = *o) < 0)
			continue;
		if(seekfs(f, off) < 0)
			warn("readedgetags: %s\n", error());
		if(readline(f) == nil)
			sysfatal("readedgetags: %s", error());
		if((s = nextfield(f)) == nil)
			sysfatal("readedgetags: bug: short read, line %d", f->nr);
		if(strcmp(s, "*") != 0)
			setedgetag("cigar", id, s);
		while((s = nextfield(f)) != nil){
			DPRINT(Debugmeta, "readedgetags: [%d] %s", id, s);
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':' || f->toksz > 128){
				warn("invalid edge tag \"%s\"\n", s);
				if(nerr++ > 10){
					werrstr("too many errors");
					return -1;
				}
				continue;
			}
			s[2] = 0;
			setedgetag(s, id, s+5);
			nerr = 0;
		}
	}
	return 0;
}

static void
initedges(Aux *a)
{
	ioff off, nn, ne, x, u, v;
	Node *n;
	u64int e;
	edgeset *h;
	khint_t k;

	h = a->edges;
	nn = a->nnodes;
	ne = kh_size(h);
	x = dylen(redges);	/* mooltigraph */
	dyresize(redges, x + ne);
	ne = a->nedges + dylen(edges);
	dyresize(edges, ne);
	kh_foreach(h, k){
		e = kh_key(h, k);
		u = e >> 32 & 0x7fffffff;
		v = e >> 2 & 0x3fffffff;
		assert(u < nn && v < nn);
		n = nodes + u;
		off = n->eoff + n->nedges++;
		assert(off >= 0 && off < ne);
		x = v << 2 | e & 3;
		DPRINT(Debugfs, "map %d%c%d%c → edge[%d]=%08x",
			u, e&1?'-':'+', v, e&2?'-':'+', off, x);
		edges[off] = x;
		if(u == v)
			continue;
		n = nodes + v;
		off = n->eoff + n->nedges++;
		assert(off >= 0 && off < ne);
		x = u << 2 | (e >> 1 & 1 | e << 1 & 2) ^ 3;
		DPRINT(Debugfs, "map %d%c%d%c → edge[%d]=%08x",
			v, e&2?'+':'-', u, e&1?'+':'-', off, x);
		edges[off] = x;
	}
}

static void
initnodes(Aux *a)
{
	int i, *l;
	ushort *dp;
	ioff off;
	vlong nn;
	Node *n, *ne;

	nn = dylen(nodes);	/* for mooltigraph */
	if(nn > 0){
		n = nodes + nn - 1;
		off = n->eoff + n->nedges;
	}else
		off = 0;
	dyresize(nodes, (nn + a->nnodes));
	dyresize(rnodes, (nn + a->nnodes));
	l = a->length;
	dp = a->degree;
	for(i=nn, n=nodes+nn, ne=n+a->nnodes; n<ne; n++, i++){
		n->id = i;
		n->eoff = off;
		n->length = *l++;
		off += *dp++;
	}
}

/* FIXME: mooltigraph: can preprocess the gfas in parallel, then
 * once all threads are done, we can join everything here; we 
 * can use common stuff if we add locks; else we just have to load
 * gfas sequentially */
static void
mkgraph(Aux *a)
{
	initnodes(a);
	fixtabs(a->nnodes, a->length, a->degree);
	initedges(a);
	edges_destroy(a->edges);
	if(newlayout(-1) < 0)
		warn("initgraph: %s\n", error());
}

static ioff
pushnode(Aux *a, char *s)
{
	ioff id;

	if((id = pushname(s)) == a->nnodes){
		a->nnodes++;
		DPRINT(Debugfs, "node \"%s\" → %d", s, id);
	}
	return id;
}

/* -2: error; -1: warning; 0: success */
static inline int
readnode(Aux *a, File *f)
{
	int id, r, w;
	vlong off;
	char *s;

	r = 0;
	s = nextfield(f);
	id = pushnode(a, s);
	if(id < a->nnodes-1 && id < dylen(a->nodeoff) && a->nodeoff[id] != 0){
		werrstr("duplicate S record");
		return -2;
	}
	s = nextfield(f);
	w = f->toksz;
	if(s != nil){
		/* do not tolerate empty field here */
		if(w <= 0){
			werrstr("empty sequence field");
			r = -1;
			w = 0;
		}else if(w == 1 && s[0] == '*')
			w = 0;
	}else if(w <= 0){	/* if 0, field is just too long */
		werrstr("error reading segment sequence");
		r = -2;
		w = 0;
	}
	if(nextfield(f) != nil)
		off = f->foff;
	else
		off = -1LL;
	dypushat(a->nodeoff, id, off);
	dypushat(a->length, id, w);
	return r;
}

/* -2: error; -1: warning; 0: success */
static inline int
readedge(Aux *a, File *f)
{
	int i, j, abs;
	ioff x, u, v;
	vlong off;
	u64int id;
	char *s, *fld[4], **fp;

	for(fp=fld; fp<fld+nelem(fld); fp++){
		if((s = nextfield(f)) == nil){
			werrstr("truncated link record");
			return -2;
		}else if(f->toksz <= 0){
			werrstr("field %d: prohibited empty field", f->nf);
			return -2;
		}else if((fp - fld) % 2 == 1 && (f->toksz > 1 || s[0] != '+' && s[0] != '-')){
			werrstr("field %d: invalid orientation", f->nf);
			return -2;
		}
		*fp = s;
	}
	u = pushnode(a, fld[0]);
	v = pushnode(a, fld[2]);
	i = *fld[1] == '+' ? 0 : 1;
	j = *fld[3] == '+' ? 0 : 1;
	if(nextfield(f) != nil)
		off = f->foff;
	else
		off = -1LL;
	/* precedence rule: always flip edge st. u < v or u is forward if self edge */
	if(u > v || u == v && i == 1)
		id = (u64int)v << 32 | u << 2 | (i << 1 | j) ^ 3;
	else
		id = (u64int)u << 32 | v << 2 | j << 1 | i;
	/* guard against duplicate records */
	edges_put(a->edges, id, &abs);
	if(!abs){
		werrstr("duplicate edge, ignored");
		return -1;
	}
	DPRINT(Debugfs, "edge \"%s%c,%s%c\" → %d%c%d%c",
		fld[0], *fld[1], fld[2], *fld[3],
		u, i?'-':'+', v, j?'-':'+');
	dypush(a->edgeoff, off);
	x = MAX(u, v);
	dygrow(a->degree, x);
	a->degree[u]++;
	if(v != u){
		a->degree[v]++;
		a->nedges += 2;
	}else
		a->nedges++;
	return 0;
}

static int
readgfa(Aux *a, File *f)
{
	int ns, nl, nerr, nwarn, r;
	char *s;

	ns = nl = nerr = nwarn = 0;
	while(readline(f) != nil){
		if((s = nextfield(f)) == nil)
			continue;
		r = 0;
		switch(s[0]){
		case 'W':	/* v1.1 */
		case 'J':	/* v1.2 */
		case '#':
		case 'H':
		case 'C':
		case 'P':
			continue;
		case 'S':
			if((r = readnode(a, f)) < 0)
				break;
			if(++ns % 1000000 == 0)
				warn("readgfa: %.3g nodes...\n", (double)ns);
			break;
		case 'L':
			if((r = readedge(a, f)) < 0)
				break;
			if(++nl % 1000000 == 0)
				warn("readgfa: %.3g edges...\n", (double)nl);
			break;
		default:
			DPRINT(Debuginfo, "line %d: unhandled record type %c", f->nr, s[0]);
		}
		if(r < 0){
			if(debug & Debuginfo)
				warn("line %d: %s\n", f->nr, error());
			else
				nwarn++;
			if(r < -1 && ++nerr >= 10){
				werrstr("too many errors");
				return -1;
			}
		}else
			nerr = 0;
	}
	if(a->nnodes <= 0){
		werrstr("empty graph");
		return -1;
	}
	if(a->nedges <= 0){
		werrstr("no edges");
		return -1;
	}
	if(dylen(a->nodeoff) < a->nnodes){	/* the reverse is fine */
		werrstr("missing S lines: links reference non-existent segments %zd != %d", dylen(a->nodeoff), a->nnodes);
		return -1;
	}
	assert(dylen(a->degree) <= a->nnodes);
	dyresize(a->degree, a->nnodes);
	if(nwarn > 0)
		warn("readgfa: suppressed %d warning messages (use -W to display them)\n", nwarn);
	return 0;
}

static File *
opengfa(Aux *a, char *path)
{
	File *f;

	if((f = openfs(path, OREAD)) == nil)
		return nil;
	logmsg(va("loadgfa: processing file: %s...\n", path));
	free(path);
	if((a->edges = edges_init()) == nil)
		sysfatal("opengfa: %s", error());
	return f;
}

static void
loadgfa1(void *arg)
{
	int d;
	vlong t;
	File *f;
	Aux a = {0};

	d = debug & Debugperf;
	t = μsec();
	if((f = opengfa(&a, arg)) == nil || readgfa(&a, f) < 0)
		sysfatal("loadgfa: %s", error());
	pushcmd("loadbatch()");
	flushcmd();
	TIME("loadgfa1", "readgfa", t);
	logmsg("loadgfa: initializing graph...\n");
	mkgraph(&a);		/* batch initialize nodes and edges from indexes */
	TIME("loadgfa1", "mkgraph", t);
	printgraph();
	/* FIXME: always send two signals, one after topo is loaded and one
	 * after everything is done, and let strawk decide what to do */
	if(gottagofast){	/* FIXME: very much unsafe, won't work with csv, etc. */
		pushcmd("cmd(\"OPL753\")");	/* load what we have and start layouting */
		flushcmd();					/* awk must be in on it or it won't work */
	}
	logmsg("loadgfa: reading node tags...\n");
	if(d)
		t = μsec();
	if(readnodetags(&a, f) < 0)
		sysfatal("loadgfa: readnodetags: %s", error());
	TIME("loadgfa1", "readnodetags", t);
	dyfree(a.nodeoff);
	if(!gottagofast && (debug & Debugload) == 0){
		pushcmd("cmd(\"FHJ142\")");
		flushcmd();
	}
	logmsg("loadgfa: reading edge tags...\n");
	if(d)
		t = μsec();
	/* FIXME: nothing is loaded besides overlap */
	if(readedgetags(&a, f) < 0)
		sysfatal("loadgfa: readedgetags: %s", error());
	TIME("loadgfa1", "readedgetags", t);
	pushcmd("loadbatch()");
	if(debug & Debugload)
		pushcmd("cmd(\"FJJ142\")");
	flushcmd();
	logmsg("loadgfa: done\n");
	dyfree(a.edgeoff);
	freefs(f);
	USED(t);
}

static Filefmt ff = {
	.name = "gfa",
	.load = loadgfa1,
};

Filefmt *
reggfa(void)
{
	return &ff;
}
