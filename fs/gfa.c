#include "strpg.h"
#include "fs.h"

/* assumptions:
 * - required field separator is \t
 */

static int
gfa1hdr(Graph *, char *)
{
	return 0;
}

static int
gfa1seg(Graph *g, char *s)
{
	getfield(s);
	DPRINT(Debugfs, "gfa pushnamednode %s", s);
	return pushnamednode(g, s) != nil ? 0 : -1;
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
gfa1link(Graph *g, char *s)
{
	int d1, d2;
	char *t, *fld[4], **fp;

	for(fp=fld; fp<fld+nelem(fld); fp++, s=t){
		if(s == nil){
			werrstr("malformed link");
			return -1;
		}
		t = getfield(s);
		*fp = s;
	}
	if((d1 = todir(fld[1])) < 0 || (d2 = todir(fld[3])) < 0){
		werrstr("malformed link orientation");
		return -1;
	}
	DPRINT(Debugfs, "gfa pushnamededge %c%s,%c%s", d1?'-':'+', fld[0], d2?'-':'+', fld[2]);
	return pushnamededge(g, fld[0], fld[2], d1, d2) != nil ? 0 : -1;
}

static int
gfa1path(Graph *, char *)
{
	return 0;
}

// FIXME: actually handle bad input
static void
loadgfa1(void *path)
{
	int n, (*parse)(Graph*, char*);
	char *s, *t;
	ssize nnodes, nedges;
	Graph g;
	File *f;

	nnodes = nedges = 0;
	DPRINT(Debugfs, "loadgfa1 %s", (char *)path);
	g = initgraph(FFgfa);
	memset(&f, 0, sizeof f);
	if((f = graphopenfs(&g, (char *)path, OREAD)) == nil)
		threadexits(error());
	while((s = readline(f, &n)) != nil){
		t = getfield(s);
		switch(s[0]){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; if(++nnodes % 10000 == 0) warn("loadgfa: %zd nodes...\n", nnodes); break;
		case 'L': parse = gfa1link; if(++nedges % 10000 == 0) warn("loadgfa: %zd edges...\n", nedges); break;
		case 'P': parse = gfa1path; break;
		default: werrstr("line %d: unknown record type %c", f->nr, s[0]); continue;
		}
		if(parse(&g, t) < 0){
			warn("loadgfa1: line %d: %s\n", f->nr, error());
			f->err++;
		}
	}
	closefs(f);
	clearmeta(&g);
	if(f->err == 10){
		warn("loadgfa1: too many errors\n");
		nukegraph(&g);
		threadexits(error());
	}
	DPRINT(Debugfs, "done loading gfa");
	g.nedges = nedges;
	g.nnodes = nnodes;
	pushgraph(g);
	threadexits(nil);
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
