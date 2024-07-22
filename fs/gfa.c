#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "cmd.h"

/* assumptions:
 * - required field separator is \t
 */

static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int c, r, l, nerr;
	vlong *off;
	Node *n;

	off = g->nodeoff;
	for(n=g->nodes, nerr=c=0; n<g->nodes+dylen(g->nodes); n++, off){
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
		DPRINT(Debugmeta, "collectgfanodes node[%zd]: %d %s", n-g->nodes, f->trunc, s);
		if((s = nextfield(f, s, nil, '\t')) == nil	/* S */
		|| (s = nextfield(f, s, nil, '\t')) == nil)	/* id */
			continue;
		t = nextfield(f, s, &l, '\t');	/* seq */
		if(l > 1 || s[0] != '*'){
			pushcmd("LN[lnode[%d]] = %d", n->id, l);
			c++;
			n->length = l;
			r = 1;
		}
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, &l, '\t');
			if(l < 5 || s[2] != ':' || s[4] != ':'){
				warn("node[%zd]: invalid segment metadata field \"%s\"\n", n-g->nodes, s);
				continue;
			}
			/* FIXME: don't assume two letter tags? */
			/* ignore length if sequence was inlined */
			/* FIXME: no error checking */
			if(strncmp(s, "LN", 2) == 0){
				if(r){
					DPRINT(Debugmeta, "node[%zx]: ignoring redundant length field", n->id);
					continue;
				}
				n->length = atoi(s+5);
			}else if(strncmp(s, "fx", 2) == 0){
				n->flags |= FNfixed;
				n->pos0.x = atof(s+5);
			}else if(strncmp(s, "fy", 2) == 0)
				n->pos0.y = atof(s+5);
			else if(strncmp(s, "mv", 2) == 0 || strncmp(s, "BO", 2) == 0){
				n->flags |= FNinitpos;
				n->pos0.x = atof(s+5);
			}
			s[2] = 0;
			pushcmd("%s[lnode[%d]] = \"%s\"", s, n->id, s+5);
			c++;
		}
		nerr = 0;
	}
	warn("collectgfanodes: done\n");
	return c;
}

static int
collectgfaedges(Graph *g, File *f)
{
	int l, nerr;
	vlong *off;
	char *s, *t;
	Edge *e;

	off = g->edgeoff;
	for(e=g->edges,nerr=0; e<g->edges+dylen(g->edges); e++, off++){
		seekfs(f, *off);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		DPRINT(Debugmeta, "collectgfaedges edges[%zd]: %d %s", e-g->edges, f->trunc, s);
		if((s = nextfield(f, s, nil, '\t')) == nil	/* L */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* u */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* du */
		|| (s = nextfield(f, s, nil, '\t')) == nil	/* v */
		|| (s = nextfield(f, s, nil, '\t')) == nil)	/* dv */
			continue;
		t = nextfield(f, s, nil, '\t');
		if(l > 0)
			pushcmd("cigar[ledge[%d]] = \"%s\"", e->id, s);
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, nil, '\t');
			if(l < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("%s[ledge[%d]] = \"%s\"", s, e->id, s+5);
		}
		nerr = 0;
	}
	return 0;
}

static void
clearmeta(Graph *g)
{
	ssize id;
	char *k;

	if(g->strnmap == nil)
		return;
	kh_foreach(g->strnmap, k, id,
		{free(k); USED(id);}
	);
	kh_destroy(strmap, g->strnmap);
	g->strnmap = nil;
}

static int
collectgfameta(Graph *g)
{
	int n, m;
	File *f;

	clearmeta(g);	/* delegated to awk */
	f = g->f;
	if((n = collectgfanodes(g, f)) < 0)
		return -1;
	if((m = collectgfaedges(g, f)) < 0)
		return -1;
	g->flags |= GFarmed;
	return n + m;
}

static int
gfa1hdr(Graph *, File *, char *)
{
	return 0;
}

static int
gfa1seg(Graph *g, File *f, char *s)
{
	Node *u;

	nextfield(f, s, nil, '\t');
	DPRINT(Debugfs, "gfa pushnamednode %s", s);
	if((u = pushnamednode(g, s)) == nil)
		return -1;
	dypush(g->nodeoff, f->foff);
	DPRINT(Debugfs, "seg %zx %s off %lld\n", u->id, s, f->foff);
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
	Edge *e;

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
	DPRINT(Debugfs, "gfa pushnamededge %c%s,%c%s", d1?'-':'+', fld[0], d2?'-':'+', fld[2]);
	if((e = pushnamededge(g, fld[0], fld[2], d1, d2)) == nil)
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
	ssize nnodes, nedges;
	Graph g;
	File *f;

	path = arg;
	nnodes = nedges = 0;
	DPRINT(Debugfs, "loadgfa1 %s", path);
	g = initgraph(FFgfa);
	if((f = graphopenfs(&g, path, OREAD)) == nil)
		sysfatal("loadgfa %s: %s", path, error());
	free(path);
	while((s = readline(f, nil)) != nil){
		p = nextfield(f, s, nil, '\t');
		switch(s[0]){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; if(++nnodes % 10000 == 0) warn("loadgfa: %zd nodes...\n", nnodes); break;
		case 'L': parse = gfa1link; if(++nedges % 10000 == 0) warn("loadgfa: %zd edges...\n", nedges); break;
		case 'P': parse = gfa1path; break;
		default: DPRINT(Debugfs, "line %d: unknown record type %c", f->nr, s[0]); continue;
		}
		if(parse(&g, f, p) < 0){
			warn("loadgfa1: line %d: %s\n", f->nr, error());
			f->err++;
		}
	}
	if(f->err >= 10){
		warn("loadgfa1: too many errors\n");
		nukegraph(&g);
		closefs(f);
		sysfatal("%s", error());
	}
	DPRINT(Debugfs, "done loading gfa");
	g.nnodes = dylen(g.nodes);
	g.nedges = dylen(g.edges);
	pushgraph(g);
	if((n = collectgfameta(&g)) < 0)
		warn("loadgfa: loading metadata failed: %s\n", error());
	/* if no actual useful metadata was loaded, don't do anything */
	else if(n > 0 && !noreset)
		pushcmd("cmd(\"OPL753\")");
	pushcmd("cmd(\"FGD135\")");
	dyfree(g.nodeoff);
	dyfree(g.edgeoff);
	closefs(f);
}

static int
save(Graph *)
{
	return 0;
}

static Filefmt ff = {
	.name = "gfa",
	.load = loadgfa1,
	.save = save,
	.nuke = nukegraph,
};

Filefmt *
reggfa(void)
{
	return &ff;
}
