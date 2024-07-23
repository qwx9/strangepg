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
	ioff i, ie;
	vlong *off;
	Node *n;

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
		n = g->nodes + i;
		DPRINT(Debugmeta, "collectgfanodes node[%zd]: %d %s", i, f->trunc, s);
		if((s = nextfield(f, s, nil, '\t')) == nil	/* S */
		|| (s = nextfield(f, s, nil, '\t')) == nil)	/* id */
			continue;
		t = nextfield(f, s, &l, '\t');	/* seq */
		if(l > 1 || s[0] != '*'){
			pushcmd("LN[lnode[%d]] = %d", i, l);
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
					DPRINT(Debugmeta, "node[%zx]: ignoring redundant length field", i);
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
			pushcmd("%s[lnode[%d]] = \"%s\"", s, i, s+5);
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
		DPRINT(Debugmeta, "collectgfaedges edges[%zd]: %d %s", i, f->trunc, s);
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

static void
clearmetatempshit(Graph *g)
{
	dyfree(g->nodeoff);
	dyfree(g->edgeoff);
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
	cleargraphtempshit(&g);
	DPRINT(Debugfs, "done loading gfa");
	pushgraph(g);
	if((n = collectgfameta(&g)) < 0)
		warn("loadgfa: loading metadata failed: %s\n", error());
	/* if no actual useful metadata was loaded, don't do anything */
	else if(n > 0 && !noreset)
		pushcmd("cmd(\"OPL753\")");
	pushcmd("cmd(\"FGD135\")");
	clearmetatempshit(&g);
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
