#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "../lib/khashl.h"
#include "stubs.h"

KHASHL_MAP_INIT(KH_LOCAL, namemap, names, char*, ioff, kh_hash_str, kh_eq_str)
KHASHL_SET_INIT(KH_LOCAL, edgeset, edges, u64int, kh_hash_uint64, kh_eq_generic)

typedef struct Aux Aux;
struct Aux{
	namemap *names;
	edgeset *edges;
	ushort *degree;
	ioff nnodes;
	ioff nedges;
};

static void
initedges(ioff nedges, edgeset *eset)
{
	ioff off, x, u, v;
	Node *n;
	u64int e;
	edgeset *h;
	khint_t k;

	dyresize(edges, nedges);
	h = eset;
	kh_foreach(h, k){
		e = kh_key(h, k);
		u = e >> 32 & 0x7fffffff;
		v = e >> 2 & 0x3fffffff;
		assert(u < dylen(nodes) && v < dylen(nodes));
		n = nodes + u;
		off = n->eoff + n->nedges++;
		assert(off >= 0 && off < dylen(edges));
		x = v << 2 | e & 3;
		DPRINT(Debugfs, "map %d%c%d%c → edge[%d]=%08x",
			u, e&1?'-':'+', v, e&2?'-':'+', off, x);
		edges[off] = x;
		if(u == v)
			continue;
		n = nodes + v;
		off = n->eoff + n->nedges++;
		assert(off >= 0 && off < dylen(edges));
		x = u << 2 | (e >> 1 & 1 | e << 1 & 2) ^ 3;
		DPRINT(Debugfs, "map %d%c%d%c → edge[%d]=%08x",
			v, e&2?'+':'-', u, e&1?'+':'-', off, x);
		edges[off] = x;
	}
}

static void
initnodes(ioff nnodes, ushort *deg)
{
	ioff i, off;
	Node *n, *ne;

	dyresize(nodes, nnodes);
	for(off=0, i=0, n=nodes, ne=n+nnodes; n<ne; n++, i++){
		n->id = i;
		n->eoff = off;
		off += *deg++;
	}
}

static void
mkgraph(Aux *a)
{
	khint_t k;

	initnodes(a->nnodes, a->degree);
	dyfree(a->degree);
	kh_foreach(a->names, k)
		free(kh_key(a->names, k));
	names_destroy(a->names);
	initedges(a->nedges, a->edges);
	edges_destroy(a->edges);
}

static ioff
pushnode(Aux *a, char *s, int *abs)
{
	ioff id;
	khint_t k;
	namemap *h;

	h = a->names;
	k = names_put(h, s, abs);
	if(*abs){
		id = a->nnodes++;
		s = estrdup(s);
		kh_key(h, k) = s;
		kh_val(h, k) = id;
		DPRINT(Debugfs, "node \"%s\" → %d", s, id);
	}else
		id = kh_val(h, k);
	return id;
}

/* -2: error; -1: warning; 0: success */
static inline int
readnode(Aux *a, File *f)
{
	int r, w, abs;
	char *s;

	r = 0;
	s = nextfield(f);
	pushnode(a, s, &abs);
	if(!abs){
		werrstr("possibly duplicate S record");	/* not checking offset */
		return -1;
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
	USED(w);
	return r;
}

/* -2: error; -1: warning; 0: success */
static inline int
readedge(Aux *a, File *f)
{
	int i, j, abs;
	ioff x, u, v;
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
	int nerr, r;
	char *s;

	nerr = 0;
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
			break;
		case 'L':
			if((r = readedge(a, f)) < 0)
				break;
			break;
		default:
			DPRINT(Debuginfo, "line %d: unhandled record type %c", f->nr, s[0]);
		}
		if(r < 0){
			if(debug & Debuginfo)
				warn("line %d: %s\n", f->nr, error());
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
	assert(dylen(a->degree) <= a->nnodes);
	dyresize(a->degree, a->nnodes);
	return 0;
}

static File *
opengfa(Aux *a, char *path)
{
	File *f;

	if((f = openfs(path, OREAD)) == nil)
		return nil;
	if((a->names = names_init()) == nil
	|| (a->edges = edges_init()) == nil)
		sysfatal("opengfa: %s", error());
	return f;
}

int
main(int argc, char **argv)
{
	File *f;
	Aux a = {0};

	if(argc < 2)
		sysfatal("usage: %s GFA", argv[0]);
	else if(argc > 2)
		debug |= Debugfs | Debuggraph;
	if((f = opengfa(&a, argv[1])) == nil || readgfa(&a, f) < 0)
		sysfatal("loadgfa: %s", error());
	mkgraph(&a);
	if(debug){
		printgraph();
		warn("%d nodes %d edges\n", a.nnodes, a.nedges);
	}
	freefs(f);
	return 0;
}
