#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"
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

typedef struct Special Special;
struct Special{
	char *tag;
	char *function;
};
static Special specials[] = {
	[TLN] = {"LN", nil},
	[Tfx] = {"fx", "fixx"},
	[Tfy] = {"fy", "fixy"},
	[Tfz] = {"fz", "fixz"},
	[Tx0] = {"x0", "initx"},
	[Ty0] = {"y0", "inity"},
	[Tz0] = {"z0", "initz"},
};

void
setattr(Node *n, int type, char *val)
{
	int i;
	float f;

	switch(type){
	case TLN:
		i = atoi(val);
		if(i < 0)
			logerr(va("warning: nonsense segment length %d\n", i));
		if(n->attr.length > 0 && n->attr.length != i)
			logerr(va("warning: segment length already set to %d != %d\n",
				n->attr.length, i));
		n->attr.length = i;
		if(i <= 0)
			break;
		if(drawing.length.min > i){
			drawing.length.min = i;
			drawing.flags |= DFstalelen;
		}
		if(drawing.length.max < i){
			drawing.length.max = i;
			drawing.flags |= DFstalelen;
		}
		break;
	case Tfx:
		n->attr.flags |= FNfixedx;
		/* wet floor */
	case Tx0:
		f = atof(val);
		n->attr.pos0.x = f;
		n->attr.flags |= FNinitx;
		if(f < drawing.xbound.min)
			drawing.xbound.min = f;
		if(f > drawing.xbound.max)
			drawing.xbound.max = f;
		break;
	case Tfy:
		n->attr.flags |= FNfixedy;
		/* wet floor */
	case Ty0:
		f = atof(val);
		n->attr.pos0.y = f;
		n->attr.flags |= FNinity;
		if(f < drawing.ybound.min)
			drawing.ybound.min = f;
		if(f > drawing.ybound.max)
			drawing.ybound.max = f;
		break;
	case Tfz:
		n->attr.flags |= FNfixedz;
		/* wet floor */
	case Tz0:
		f = atof(val);
		n->attr.pos0.z = atof(val);
		n->attr.flags |= FNinitz;
		if(f < drawing.zbound.min)
			drawing.zbound.min = f;
		if(f > drawing.zbound.max)
			drawing.zbound.max = f;
		break;
	default: logerr(va("unknown attr type %d\n", type));
	}
}

/* calling the awk function directly will execute its side effect and
 * send back an attribute change message */
void
setnamedtag(char *name, char *tag, char *val)
{
	Special *t, *te;

	for(t=specials, te=t+nelem(specials); t<te; t++)
		if(strncmp(t->tag, tag, 12) == 0){	/* safety + special tables */
			pushcmd("%s(%d,\"%s\")", t->function, name, val);
			return;
		}
	pushcmd("%s[\"%s\"]=\"%s\"", tag, name, val);
}
/* before strawk is primed and ready, set special tags ourselves */
void
settag(Graph *g, ioff id, char type, char *tag, char *val)
{
	Special *t, *te;

	for(t=specials, te=t+nelem(specials); t<te; t++)
		if(strncmp(t->tag, tag, 12) == 0)
			setattr(g->nodes + id, t - specials, val);
	switch(type){
	case 'i':
	case 'f': pushcmd("%s[label[%d]]=%s", tag, id, val); break;
	case 'A':
	case 'Z':
	case 'J':
	case 'H':
	case 'B': pushcmd("%s[label[%d]]=\"%s\"", tag, id, val); break;
	default:
		warn("settag %s=%s: unknown type %c, defaulting to string\n",
			tag, val, type);
	}
}

static void
transmittags(Graph *g)
{
	ioff id, eid, *e, *ee;
	Node *n, *ne;

	for(id=eid=0, n=g->nodes, ne=n+dylen(n); n<ne; n++, id++){
		pushcmd("CL[label[%d]] = %x", id, n->attr.color);
		pushcmd("LN[label[%d]] = %d", id, n->attr.length);
		if((n->attr.flags & FNinitx) != 0){
			pushcmd("x0[label[%d]] = %f", (double)n->attr.pos0.x);
			if((n->attr.flags & FNfixedx) != 0)
				pushcmd("fx[label[%d]] = %f", (double)n->attr.pos0.x);
		}
		if((n->attr.flags & FNinity) != 0){
			pushcmd("y0[label[%d]] = %f", (double)n->attr.pos0.y);
			if((n->attr.flags & FNfixedy) != 0)
				pushcmd("fy[label[%d]] = %f", (double)n->attr.pos0.y);
		}
		if((n->attr.flags & FNinitz) != 0){
			pushcmd("z0[label[%d]] = %f", (double)n->attr.pos0.z);
			if((n->attr.flags & FNfixedz) != 0)
				pushcmd("fz[label[%d]] = %f", (double)n->attr.pos0.z);
		}
		for(e=g->edges+n->eoff,ee=e+n->nedges; e<ee; e++, eid++)
			pushcmd("addedge(%d,%d,%x)", eid, id, *e);
	}
}

static void
cleanup1(namemap *h)
{
	char *lab;
	khint_t k;

	kh_foreach(h, k){
		lab = kh_key(h, k);
		free(lab);
	}
}

static int
readnodetags(Aux *a, Graph *g, File *f)
{
	int nerr;
	char *s;
	ioff id, off;
	vlong *o, *oe;

	for(nerr=0, id=0, o=a->nodeoff, oe=o+dylen(o); o<oe; o++, id++){
		if((off = *o) < 0)
			continue;
		seekfs(f, off);
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
			settag(g, id, s[3], s, s+5);
			nerr = 0;
		}
	}
	return 0;
}

static int
readedgetags(Aux *a, Graph *g, File *f)
{
	int nerr;
	char *s;
	ioff id, off;
	vlong *o, *oe;

	for(nerr=0, id=0, o=a->edgeoff, oe=o+dylen(o); o<oe; o++, id++){
		if((off = *o) < 0)
			continue;
		seekfs(f, off);
		if(readline(f) == nil)
			sysfatal("readedgetags: %s", error());
		if((s = nextfield(f)) == nil)
			sysfatal("readedgetags: bug: short read, line %d", f->nr);
		settag(g, id, 'Z', "cigar", s);
		while((s = nextfield(f)) != nil){
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':' || f->toksz > 128){
				warn("invalid edge tag \"%s\"\n", s);
				if(nerr++ > 10){
					werrstr("too many errors");
					return -1;
				}
				continue;
			}
			s[2] = 0;
			settag(g, id, s[3], s, s+5);
			nerr = 0;
		}
	}
	return 0;
}

static void
initnodes(Graph *g, ioff nnodes, int *len, ioff *off, ushort *deg)
{
	int i;
	u32int c;
	Node *n, *ne;
	RNode *r;

	dyresize(g->nodes, nnodes);
	dyresize(rnodes, nnodes);
	for(i=0, r=rnodes, n=g->nodes, ne=n+nnodes; n<ne; n++, r++){
		n->attr.length = *len++;
		n->nedges = *deg++;
		n->nin = 0;
		n->eoff = *off++;
		c = somecolor(i++, nil);
		n->attr.color = c;
		r->len = 1.0f;
		setcolor(r->col, c);
		if(n->attr.length <= 0)
			continue;
		if(drawing.length.min > n->attr.length){
			drawing.length.min = n->attr.length;
			drawing.flags |= DFstalelen;
		}
		if(drawing.length.max < n->attr.length){
			drawing.length.max = n->attr.length;
			drawing.flags |= DFstalelen;
		}
	}
}

/* UGH */
static void
initedges(Aux *a, Graph *g, ioff nedges, ioff *index)
{
	ushort d, *degree;
	ioff off, i, j;
	u64int e, u, v;
	edgeset *h;
	khint_t k;

	dyresize(g->edges, 2 * nedges);
	dyresize(redges, nedges + nelem(selbox));
	h = a->edges;
	degree = a->degree;
	kh_foreach(h, k){
		e = kh_key(h, k);
		u = e >> 32ULL & 0xffffffff;
		v = e & 0xffffffff;
		i = u >> 1;
		j = v >> 1;
		if(debug & Debuggraph)
			warn("map %zd%c %zd%c → ", i, u&1?'-':'+', j, v&1?'-':'+');
		assert(i < dylen(g->nodes));
		assert(j < dylen(g->nodes));
		if(i == j){					/* self edge */
			degree[i]--;
			off = index[i]++;
			if(debug & Debuggraph)
				warn("self off=%d base=%d\n", off, g->nodes[j].eoff);
			g->edges[off] = v << 1 | u & 1;
		}else if(u & 1){			/* in edge */
			d = --degree[i];
			off = index[i] + d;
			if(debug & Debuggraph)
				warn("in off=%d base=%d, ", off, g->nodes[i].eoff);
			g->edges[off] = v << 1 | 1;
			g->nodes[i].nin++;
			degree[j]--;
			off = index[j]++;
			if(debug & Debuggraph)
				warn("out off=%d base=%d\n", off, g->nodes[j].eoff);
			g->edges[off] = (u ^ 1) << 1 | v & 1 ^ 1;
		}else{						/* out edge */
			degree[i]--;
			off = index[i]++;
			if(debug & Debuggraph)
				warn("out off=%d base=%d, ", off, g->nodes[i].eoff);
			g->edges[off] = v << 1 | 0;
			d = --degree[j];
			off = index[j] + d;
			if(debug & Debuggraph)
				warn("in off=%d base=%d\n", off, g->nodes[j].eoff);
			g->edges[off] = (u ^ 1) << 1 | v & 1 ^ 1;
			g->nodes[j].nin++;
		}
	}
}

static void
mkgraph(Aux *a, Graph *g, File *f)
{
	initgraph(g, FFgfa);
	initnodes(g, a->nnodes, a->length, a->index, a->degree);
	dyfree(a->length);
	initedges(a, g, a->nedges, a->index);
	edges_destroy(a->edges);
	dyfree(a->degree);
	free(a->index);
	assert(dylen(g->edges) == 2*(dylen(redges)-4));
	assert(dylen(g->nodes) == a->nnodes);
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
	dyfree(totals);
	USED(totals);
}

static inline ioff
pushnode(Aux *a, char *s, int *abs)
{
	ioff id;
	khint_t k;
	namemap *h;

	h = a->names;
	k = names_put(h, s, abs);
	if(*abs){
		id = a->nnodes++;
		kh_key(h, k) = estrdup(s);
		kh_val(h, k) = id;
		pushcmd("addnode(%d,\"%s\")", id, s);	
	}else
		id = kh_val(h, k);
	DPRINT(Debugfs, "node[%d] %s", id, s);
	return id;
}

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
		r = -1;
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
			return -1;
		}else if(f->toksz <= 0){
			werrstr("field %d: prohibited empty field", f->nf);
			return -1;
		}else if((fp - fld) % 2 == 1 && (f->toksz > 1 || s[0] != '+' && s[0] != '-')){
			werrstr("field %d: invalid orientation", f->nf);
			return -1;
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
		warn("readgfa: line %d: duplicate edge, ignored\n", f->nr);
		return 0;
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
	char *s;
	ioff nerr;

	nerr = 0;
	while(readline(f) != nil){
		if((s = nextfield(f)) == nil)
			continue;
		switch(s[0]){
		err:
			warn("readgfa: line %d: %s\n", f->nr, error());
			nerr++;
			break;
		case '#':
		case 'H':
		case 'P': continue;
		case 'S':
			if(readnode(a, f) < 0)
				goto err;
			if(a->nnodes % 10000000 == 0)
				warn("readgfa: %.2g nodes...\n", (double)a->nnodes);
			break;
		case 'L':
			if(readedge(a, f) < 0)
				goto err;
			if(a->nedges % 10000000 == 0)
				warn("readgfa: %.2g edges...\n", (double)a->nedges);
			break;
		default: DPRINT(Debugfs, "line %d: unhandled record type %c", f->nr, s[0]);
		}
		if(nerr >= 10){
			werrstr("too many errors");
			return -1;
		}
	}
	if(a->nnodes <= 0){
		werrstr("empty graph");
		return -1;
	}
	if(dylen(a->nodeoff) < a->nnodes){	/* the reverse is fine */
		werrstr("missing S lines: links reference non-existent segments");
		return -1;
	}
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

#define	TIME(lab)	if((debug & Debugperf) != 0) do{ \
	t = μsec(); \
	warn("readgfa: " lab ": %.2f ms\n", (t - t0)/1000); \
	t0 = t; \
}while(0)

static void
loadgfa1(void *arg)
{
	double t, t0;
	File *f;
	Aux a = {0};
	Graph gr, *g;

	t0 = μsec();
	g = &gr;
	if((f = opengfa(&a, arg)) == nil || readgfa(&a, f) < 0)
		sysfatal("loadgfa: %s", (char*)arg, error());
	TIME("readgfa");
	if(a.nnodes == 0)
		sysfatal("loadgfa: no nodes\n");
	else if(a.nedges == 0)
		sysfatal("loadgfa: no edges\n");
	logmsg("loadgfa: initializing graph...\n");
	mkbuckets(&a);			/* compute slots for each node's edges */
	TIME("mkbuckets");
	mkgraph(&a, g, f);		/* batch initialize nodes and edges from indexes */
	TIME("mkgraph");
	g = pushgraph(g);		/* register new graph, maybe start layout */
	TIME("pushgraph");
	cleanup1(a.names);	/* push node labels to strawk */
	names_destroy(a.names);
	TIME("cleanup");
	logmsg("loadgfa: reading node tags...\n");
	if(readnodetags(&a, g, f) < 0)	/* load node tags, push unknowns to strawk */
		sysfatal("loadgfa: readnodetags: %s", error());
	dyfree(a.nodeoff);
	TIME("readnodetags");
	pushcmd("cmd(\"FHJ142\")");	/* signal read next file or start layouting */
	flushcmd();
	logmsg("loadgfa: syncing awk...\n");
	/* FIXME: have awk decide on this based on argv <- or not */
	//if(!gottagofast)
	transmittags(g);		/* push remaining node and edge info to awk */
	TIME("transmittags");
	logmsg("loadgfa: reading edge tags...\n");
	if(readedgetags(&a, g, f) < 0)	/* process and push edge tags */
		sysfatal("loadgfa: readedgetags: %s", error());
	/* done transmitting the important stuff */
	pushcmd("cmd(\"FGD135\")");	/* issue go code (one per input) */
	if((debug & Debugload) != 0)
		pushcmd("quit()");
	flushcmd();
	dyfree(a.edgeoff);
	logmsg("loadgfa: done\n");
	TIME("readedgetags");
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
