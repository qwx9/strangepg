#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"
#include "lib/khashl.h"

KHASHL_MAP_INIT(KH_LOCAL, namemap, names, char*, ioff, kh_hash_str, kh_eq_str)
static namemap *names;

/* assumptions:
 * - required field separator is \t
 * - tags are two characters long as per the spec
 * - length of inlined sequence trumps LN tag
 */

/* usable only during first pass */
static ioff
getid(Graph *, char *s)
{
	ioff id;
	khint_t k;

	k = names_get(names, s);
	if(k == kh_end(names))
		return -1;
	id = kh_val(names, k);
	return id;
}
static ioff
pushid(Graph *, char *s, ioff id)
{
	int abs;
	khint_t k;

	k = names_put(names, s, &abs);
	assert(abs);
	kh_val(names, k) = id;
	return 0;
}

/* FIXME: no error checking */
void
setnamedtag(char *label, char *tag, char *val)
{
	if(strcmp(tag, "CL") == 0 || cistrcmp(tag, "color") == 0)
		pushcmd("nodecolor(\"%s\", %s)", label, val);
	else if(strcmp(tag, "fx") == 0)
		pushcmd("fixx(\"%s\", %s)", label, val);
	else if(strcmp(tag, "fy") == 0)
		pushcmd("fixy(\"%s\", %s)", label, val);
	else if(strcmp(tag, "x0") == 0)
		pushcmd("initx(\"%s\", %s)", label, val);
	else if(strcmp(tag, "y0") == 0)
		pushcmd("inity(\"%s\", %s)", label, val);
	else
		pushcmd("%s[\"%s\"] = \"%s\"", tag, label, val);
}
void
settag(Node *n, ioff id, char *tag, char *val)
{
	if(strcmp(tag, "LN") == 0){
		n->length = atoi(val);
		rnodes[id].len = n->length;
	}else if(strcmp(tag, "fx") == 0){
		n->pos0.x = atof(val);
		n->flags |= FNfixedx | FNinitx;
	}else if(strcmp(tag, "fy") == 0){
		n->pos0.y = atof(val);
		n->flags |= FNfixedy | FNinity;
	}else if(strcmp(tag, "x0") == 0){
		n->pos0.x = atof(val);
		n->flags |= FNinitx;
	}else if(strcmp(tag, "y0") == 0){
		n->pos0.y = atof(val);
		n->flags |= FNinity;
	}
	pushcmd("%s[label[%d]] = \"%s\"", tag, id, val);
}

static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int c, r, l, nerr, minl, maxl;
	ioff id, i, ie;
	vlong *off;
	Node *n;

	minl = 0x7fffffff;
	maxl = 1;
	off = g->nodeoff;
	for(i=0,ie=dylen(g->nodes),nerr=c=0; i<ie; i++, off++){
		r = 0;
		seekfs(f, *off);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		if((s = nextfield(f, s, nil, '\t')) == nil	/* S */
		|| (t = nextfield(f, s, nil, '\t')) == nil)     /* id */
			continue;
		if((id = getid(g, s)) < 0)
			sysfatal("collectgfanodes: bug: %s not found", s);
		DPRINT(Debugmeta, "collectgfanodes node[%d]: %d %s", id, f->trunc, s);
		n = g->nodes + id;
		s = t;
		t = nextfield(f, s, &l, '\t');	/* seq */
		if(l > 1 || s[0] != '*'){
			pushcmd("LN[label[%d]] = %d", id, l);
			c++;
			n->length = l;
			rnodes[id].len = l;
			r = 1;
		}
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, &l, '\t');
			if(l < 5 || s[2] != ':' || s[4] != ':'){
				warn("node[%zd]: invalid segment metadata field \"%s\"\n", n-g->nodes, s);
				continue;
			}
			s[2] = 0;
			/* ignore length if sequence was inlined */
			if(strcmp(s, "LN") == 0 && r){
				DPRINT(Debugmeta, "node[%x]: ignoring redundant length field", id);
				continue;
			}
			settag(n, id, s, s+5);
			c++;
		}
		nerr = 0;
		if(maxl < n->length)
			maxl = n->length;
		if(minl > n->length)
			minl = n->length;
	}
	if(maxl > 1)
		fixlengths(minl, maxl);
	return c;
}

static int
collectgfaedges(Graph *g, File *f)
{
	int l, nerr;
	ioff i, ie;
	vlong *off;
	char *s, *t;

	off = g->edgeoff;
	for(i=0,ie=dylen(g->edges),nerr=0; i<ie; i++, off++){
		seekfs(f, *off);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		DPRINT(Debugmeta, "collectgfaedges edges[%d]: %d %s", i, f->trunc, s);
		if((s = nextfield(f, s, nil, '\t')) == nil	/* L */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* u */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* du */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* v */
		|| (s = nextfield(f, s, nil, '\t')) == nil)	/* dv */
			continue;
		t = nextfield(f, s, nil, '\t');
		if(l > 0)
			pushcmd("cigar[ledge[%d]] = \"%s\"", i, s);
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, nil, '\t');
			if(l < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("%s[ledge[%d]] = \"%s\"", s, i, s+5);
		}
		nerr = 0;
	}
	return 0;
}

static int
collectgfameta(Graph *g)
{
	int n, m;
	File *f;

	f = g->f;
	if((n = collectgfanodes(g, f)) < 0)
		return -1;
	if((m = collectgfaedges(g, f)) < 0)
		return -1;
	g->flags |= GFarmed;
	return n + m;
}

static void
clearmetatempshit(Graph *g)
{
	dyfree(g->nodeoff);
	dyfree(g->edgeoff);
}

static void
cleargraphtempshit(Graph *)
{
	khint_t k;

	if(names == nil)
		return;
	kh_foreach(names, k)
		free(kh_key(names, k));
	names_destroy(names);
	names = nil;
}

static ioff
pushnode(Graph *g, char *s)
{
	ioff id;

	/* nodes may be defined after an edge references them, should
	 * not be considered as an error */
	if((id = getid(g, s)) >= 0){
		DPRINT(Debugfs, "duplicate node[%d] %s", id, s);
		return id;
	}
	s = estrdup(s);
	if((id = newnode(g, s)) < 0 || pushid(g, s, id) < 0){
		free(s);
		return -1;
	}
	return id;
}

static ioff
pushedge(Graph *g, char *eu, char *ev, int d1, int d2)
{
	char *s, *sf;
	ioff id, f, n, u, v;

	if((u = pushnode(g, eu)) < 0 || (v = pushnode(g, ev)) < 0)
		return -1;
	n = strlen(eu) + strlen(ev) + 8;
	s = emalloc(n);
	sf = nil;
	snprint(s, n, "%s%c\x1c%s%c", eu, d1 ? '-' : '+', ev, d2 ? '-' : '+');
	/* detect redundancies by always flipping edges such that u<v, or
	 * for self-edges if u is reversed */
	if(f = v > u || v == u && d1 == 1){
		sf = estrdup(s);
		snprint(s, n, "%s%c\x1c%s%c", ev, d2 ? '+' : '-', eu, d1 ? '+' : '-');
	}
	if((id = getid(g, s)) >= 0){
		DPRINT(Debugfs, "duplicate edge[%d] %s%s", id, s, f ? " (flipped)" : "");
		free(s);
		free(sf);
		return id;
	}
	u = u << 1 | d1;
	v = v << 1 | d2;
	/* always push only what was actually in the input */
	if((id = newedge(g, u, v, sf != nil ? sf : s)) < 0 || pushid(g, s, id) < 0){
		free(s);
		id = -1;
	}
	free(sf);
	return id;
}

static int
gfa1hdr(Graph *, File *, char *)
{
	return 0;
}

static int
gfa1seg(Graph *g, File *f, char *s)
{
	nextfield(f, s, nil, '\t');
	DPRINT(Debugfs, "gfa pushnode %s", s);
	if(pushnode(g, s) < 0)
		return -1;
	dypush(g->nodeoff, f->foff);
	return 0;
}

static int
todir(char *s)
{
	if(strncmp(s, "+", 2) == 0)
		return Vforward;
	else if(strncmp(s, "-", 2) == 0)
		return Vreverse;
	return -1;
}

static int
gfa1link(Graph *g, File *f, char *s)
{
	int d1, d2;
	char *t, *fld[4], **fp;

	for(fp=fld; fp<fld+nelem(fld); fp++, s=t){
		if(s == nil){
			werrstr("malformed link");
			return -1;
		}
		t = nextfield(f, s, nil, '\t');
		*fp = s;
	}
	if((d1 = todir(fld[1])) < 0 || (d2 = todir(fld[3])) < 0){
		werrstr("malformed link orientation");
		return -1;
	}
	DPRINT(Debugfs, "gfa pushedge %c%s,%c%s", d1?'-':'+', fld[0], d2?'-':'+', fld[2]);
	if(pushedge(g, fld[0], fld[2], d1, d2) < 0)
		return -1;
	dypush(g->edgeoff, f->foff);
	return 0;
}

static int
gfa1path(Graph *, File *, char *)
{
	return 0;
}

// FIXME: actually handle bad input
static void
loadgfa1(void *arg)
{
	int (*parse)(Graph*, File*, char*);
	int n;
	char *s, *p, *path;
	ioff nnodes, nedges;
	Graph g;
	File *f;

	path = arg;
	nnodes = nedges = 0;
	DPRINT(Debugfs, "loadgfa1 %s", path);
	g = initgraph(FFgfa);
	if((f = graphopenfs(&g, path, OREAD)) == nil)
		sysfatal("loadgfa %s: %s", path, error());
	free(path);
	if(names == nil)
		names = names_init();
	while((s = readline(f, nil)) != nil){
		p = nextfield(f, s, nil, '\t');
		switch(s[0]){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; if(++nnodes % 100000 == 0) warn("loadgfa: %de5 nodes...\n", nnodes/100000); break;
		case 'L': parse = gfa1link; if(++nedges % 100000 == 0) warn("loadgfa: %de5 edges...\n", nedges/100000); break;
		case 'P': parse = gfa1path; break;
		default: DPRINT(Debugfs, "line %d: unknown record type %c", f->nr, s[0]); continue;
		}
		if(parse(&g, f, p) < 0){
			warn("loadgfa1: line %d: %s\n", f->nr, error());
			f->err++;
		}
	}
	if(f->err >= 10)
		sysfatal("loadgfa1: too many errors, last error: %s\n", error());
	if(dylen(g.nodeoff) < dylen(g.nodes))
		sysfatal("loadgfa1: invalid GFA: missing S lines, links reference non-existent segments");
	if(dylen(g.nodes) == 0 || dylen(g.edges) == 0)
		sysfatal("loadgfa1: empty graph: %s", error());
	DPRINT(Debugfs, "done loading gfa");
	pushgraph(&g);
	logmsg("loadgfa: loading tags...\n");
	if((n = collectgfameta(&g)) < 0)
		warn("loadgfa: loading metadata failed: %s\n", error());
	/* if no actual useful metadata was loaded, don't do anything */
	else if(n > 0 || !gottagofast)
		pushcmd("cmd(\"FHJ142\")");
	logmsg("loadgfa: done\n");
	pushcmd("cmd(\"FGD135\")");	/* FIXME: after only one input file? */
	cleargraphtempshit(&g);
	clearmetatempshit(&g);
	closefs(f);
}

static Filefmt ff = {
	.name = "gfa",
	.load = loadgfa1,
	.nuke = nukegraph,
};

Filefmt *
reggfa(void)
{
	return &ff;
}
