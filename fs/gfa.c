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

KHASHL_MAP_INIT(KH_LOCAL, namemap, names, char*, ioff, kh_hash_str, kh_eq_str)
KHASHL_SET_INIT(KH_LOCAL, edgeset, edges, u64int, kh_hash_uint64, kh_eq_generic)

typedef struct Aux Aux;
struct Aux{
	namemap *names;
	edgeset *edges;
	vlong *nodeoff;
	int *length;
	ushort *degree;
	ioff *index;
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
		if((off = *o) < 0)
			continue;
		if(seekfs(f, off) < 0)
			warn("readnodetags: %s\n", error());
		if(readline(f) == nil)
			sysfatal("readnodetags: %s", error());
		while((s = nextfield(f)) != nil){
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':' || f->toksz > 128){
				warn("invalid segment tag \"%s\"\n", s);
				if(nerr++ > 10){
					werrstr("too many errors");
					return -1;
				}
				continue;
			}
			s[2] = 0;
			settag(s, id, s+5, 0);
			nerr = 0;
		}
	}
	return 0;
}

static int
readedgetags(Aux *a, File *f)
{
	int nerr;
	char *s, tag[8] = {'e', '\0'};
	ioff id;
	vlong off, *o, *oe;

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
			settag("cigar", id, s, 1);
		while((s = nextfield(f)) != nil){
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':' || f->toksz > 128){
				warn("invalid edge tag \"%s\"\n", s);
				if(nerr++ > 10){
					werrstr("too many errors");
					return -1;
				}
				continue;
			}
			/* FIXME: edge tags may have the same name as node tags and
			 * overwrite them; we need a better mechanism for this */
			/* FIXME: unhandled for now; these names aren't used, and we
			 * need an actual correspondence with cid's
			tag[1] = s[0];
			tag[2] = s[1];
			settag(tag, id, s+5, 1);
			*/
			nerr = 0;
		}
	}
	return 0;
}

/* UGH */
static void
initedges(ioff nedges, edgeset *eset, ioff *index, ushort *degree)
{
	ushort d, *deg;
	ioff off, i, j;
	vlong nn, nm;
	u64int e, u, v;
	edgeset *h;
	khint_t k;

	nn = dylen(edges);	/* mooltigraph */
	nm = nn + 2 * nedges;
	dyresize(edges, nm);
	dyresize(redges, nedges);
	h = eset;
	deg = degree;
	kh_foreach(h, k){
		e = kh_key(h, k);
		u = e >> 32ULL & 0xffffffff;
		v = e & 0xffffffff;
		i = u >> 1;
		j = v >> 1;
		if(debug & Debuggraph)
			warn("map %d%c %d%c → ", i, u&1?'-':'+', j, v&1?'-':'+');
		assert(i < dylen(nodes));
		assert(j < dylen(nodes));
		/* FIXME: now that we don't assume in edges are at the end, get rid
		 * of this shit */
		if(i == j){					/* self edge */
			deg[i]--;
			off = index[i]++;
			if(debug & Debuggraph)
				warn("self off=%d base=%d\n", off, nodes[j].eoff);
			edges[off] = v << 1 | u & 1;
		}else if(u & 1){			/* in edge */
			d = --deg[i];
			off = index[i] + d;
			if(debug & Debuggraph)
				warn("in off=%d base=%d, ", off, nodes[i].eoff);
			edges[off] = v << 1 | 1;
			deg[j]--;
			off = index[j]++;
			if(debug & Debuggraph)
				warn("out off=%d base=%d\n", off, nodes[j].eoff);
			edges[off] = (u ^ 1) << 1 | v & 1 ^ 1;
		}else{						/* out edge */
			deg[i]--;
			off = index[i]++;
			if(debug & Debuggraph)
				warn("out off=%d base=%d, ", off, nodes[i].eoff);
			edges[off] = v << 1 | 0;
			d = --deg[j];
			off = index[j] + d;
			if(debug & Debuggraph)
				warn("in off=%d base=%d\n", off, nodes[j].eoff);
			edges[off] = (u ^ 1) << 1 | v & 1 ^ 1;
		}
	}
}

static void
mkgraph(Aux *a)
{
	initnodes(a->nnodes, a->length, a->index, a->degree);
	dyfree(a->length);
	initedges(a->nedges, a->edges, a->index, a->degree);
	edges_destroy(a->edges);
	dyfree(a->degree);
	free(a->index);
	if(newlayout(-1) < 0)
		warn("initgraph: %s\n", error());
}

static void
mkbuckets(Aux *a)
{
	ioff *off, *offset, *idx, *t, *te, *totals;
	uint k, n, m;
	ushort d, *dp, *de;

	/* cumulative total number of edges per degree */
	totals = nil;
	for(dp=a->degree, de=dp+dylen(dp); dp<de; dp++){
		d = *dp;
		dyresize(totals, d+1);
		totals[d]++;
	}
	/* offset of node's current adjacency list */
	offset = emalloc(dylen(totals) * sizeof *offset);
	for(n=m=0, d=0, off=offset, t=totals, te=t+dylen(t); t<te; d++, off++){
		k = *t;
		n += k;
		*t++ = n;
		if(d > 0){
			*off = m;
			m += k * d;
		}
	}
	/* allot nodes by increasing degree number */
	a->index = emalloc(dylen(a->degree) * sizeof *a->index);
	for(idx=a->index, dp=a->degree, de=dp+dylen(dp); dp<de; dp++){
		d = *dp;
		*idx++ = offset[d];
		offset[d] += d;
	}
	free(offset);
	if(debug & Debugfs){
		for(n=0, t=totals, te=t+dylen(t); t<te; t++){
			m = *t;
			if(m > n){
				warn("totals[%zd] %d elements (%d total)\n", t-totals, m - n, m);
				n = m;
			}
		}
	}
	dyfree(totals);
	USED(totals);
}

static ioff
pushnode(Aux *a, char *s, int *abs)
{
	ioff id;
	khint_t k;
	namemap *h;
	V v;

	h = a->names;
	k = names_put(h, s, abs);
	if(*abs){
		id = a->nnodes++;
		s = estrdup(s);
		kh_key(h, k) = s;
		kh_val(h, k) = id;
		v.s = s;
		setspectag(Tnode, id, v);
	}else
		id = kh_val(h, k);
	DPRINT(Debugfs, "node[%d] %s", id, s);
	return id;
}

/* -2: error; -1: warning; 0: success */
static inline int
readnode(Aux *a, File *f)
{
	int id, r, w, abs;
	ioff off;
	char *s;

	r = 0;
	s = nextfield(f);
	id = pushnode(a, s, &abs);
	if(!abs && id < dylen(a->nodeoff) && a->nodeoff[id] != 0){
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
		off = -1;
	dyinsert(a->nodeoff, id, off);
	dyinsert(a->length, id, w);
	return r;
}

/* -2: error; -1: warning; 0: success */
static inline int
readedge(Aux *a, File *f)
{
	int i, j, abs;
	ioff n, u, v;
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
	u = pushnode(a, fld[0], &abs);
	v = pushnode(a, fld[2], &abs);
	i = *fld[1] == '+' ? 0 : 1;
	j = *fld[3] == '+' ? 0 : 1;
	DPRINT(Debugfs, "edge[%d] %d%c,%d%c", a->nedges, u, *fld[1], v, *fld[3]);
	if(nextfield(f) != nil)
		off = f->foff;
	else
		off = -1;
	/* precedence rule: always flip edge st. u < v or u is forward if self edge */
	if(u > v || u == v && i == 1){
		n = u;
		u = v;
		v = n;
		n = i ^ 1;
		i = j ^ 1;
		j = n;
	}
	/* guard against duplicate records */
	id = ((u64int)u << 1 | i) << 32ULL | v << 1 | j;
	edges_put(a->edges, id, &abs);
	if(!abs){
		werrstr("duplicate edge, ignored");
		return -1;
	}
	dypush(a->edgeoff, off);
	dyresize(a->degree, v+1);
	a->degree[u]++;
	if(v != u)
		a->degree[v]++;
	a->nedges++;
	return 0;
}

static int
readgfa(Aux *a, File *f)
{
	int nerr, nwarn, r;
	char *s;

	nerr = nwarn = 0;
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
			if(a->nnodes % 1000000 == 0)
				warn("readgfa: %.3g nodes...\n", (double)a->nnodes);
			break;
		case 'L':
			if((r = readedge(a, f)) < 0)
				break;
			if(a->nedges % 1000000 == 0)
				warn("readgfa: %.3g edges...\n", (double)a->nedges);
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
	if(dylen(a->nodeoff) < a->nnodes){	/* the reverse is fine */
		werrstr("missing S lines: links reference non-existent segments");
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
	a->names = names_init();
	a->edges = edges_init();
	return f;
}

#define	TIME(lab)	if(d) do{ \
	t = μsec(); \
	warn("readgfa: " lab ": %.2f ms\n", (t - t0)/1000); \
	t0 = t; \
}while(0)

static void
loadgfa1(void *arg)
{
	int d;
	double t, t0;
	File *f;
	Aux a = {0};

	d = debug & Debugperf;
	t0 = μsec();
	if((f = opengfa(&a, arg)) == nil || readgfa(&a, f) < 0)
		sysfatal("loadgfa: %s", error());
	pushcmd("loadbatch()");
	flushcmd();
	TIME("readgfa");
	if(a.nnodes == 0)
		sysfatal("loadgfa: no nodes\n");
	else if(a.nedges == 0)
		sysfatal("loadgfa: no edges\n");
	logmsg("loadgfa: initializing graph...\n");
	mkbuckets(&a);			/* compute slots for each node's edges */
	TIME("mkbuckets");
	mkgraph(&a);		/* batch initialize nodes and edges from indexes */
	TIME("mkgraph");
	if(gottagofast){	/* FIXME: very much unsafe, won't work with csv, etc. */
		pushcmd("cmd(\"OPL753\")");	/* load what we have and start layouting */
		flushcmd();					/* awk must be in on it or it won't work */
	}
	names_destroy(a.names);
	logmsg("loadgfa: reading node tags...\n");
	if(d)
		t0 = μsec();
	if(readnodetags(&a, f) < 0)
		sysfatal("loadgfa: readnodetags: %s", error());
	TIME("readnodetags");
	dyfree(a.nodeoff);
	if(!gottagofast && (debug & Debugload) == 0){
		pushcmd("cmd(\"FHJ142\")");
		flushcmd();
	}
	logmsg("loadgfa: reading edge tags...\n");
	if(d)
		t0 = μsec();
	/* FIXME: nothing is loaded besides overlap */
	if(readedgetags(&a, f) < 0)
		sysfatal("loadgfa: readedgetags: %s", error());
	TIME("readedgetags");
	pushcmd("loadbatch()");
	if(debug & Debugload)
		pushcmd("cmd(\"FJJ142\")");
	flushcmd();
	logmsg("loadgfa: done\n");
	dyfree(a.edgeoff);
	freefs(f);
	USED(t0);
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
