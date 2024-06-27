#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "threads.h"
#include "cmd.h"

/* assumptions:
 * - required field separator is \t
 */

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
	u->metaoff = f->foff;
	DPRINT(Debugfs, "seg %zx %s off %lld\n", u->id, s, u->metaoff);
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
	e->metaoff = f->foff;
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
		default: werrstr("line %d: unknown record type %c", f->nr, s[0]); continue;
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
