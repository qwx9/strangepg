#include "strpg.h"
#include "fs.h"

/* assumptions:
 * - all segments are defined before any links
 */

static int
gfa1hdr(Graph *, File *f)
{
	if(f->nf < 2){
		werrstr("line %d: malformed header", f->nr);
		return -1;
	}
	return 0;
}

static int
gfa1seg(Graph *g, File *f)
{
	if(f->nf < 3){
		werrstr("line %d: malformed segment", f->nr);
		return -1;
	}
	return addnode(g, f->fld[1], f->fld[2]);
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
gfa1link(Graph *g, File *f)
{
	int d1, d2;
	char *s, *t;

	if(f->nf < 6){
		s = f->fld[1] + strlen(f->fld[1]) - 1;
		t = f->fld[2] + strlen(f->fld[2]) - 1;
		if((d1 = todir(s)) < 0 || (d2 = todir(t)) < 0){
			werrstr("line %d: malformed link", f->nr);
			return -1;
		}
		*s = *t = 0;
		return addedge(g, f->fld[1], f->fld[2], d1, d2, f->fld[3], 1.);
	}else if((d1 = todir(f->fld[2])) < 0 || (d2 = todir(f->fld[4])) < 0){
		werrstr("line %d: malformed link orientation", f->nr);
		return -1;
	}
	return addedge(g, f->fld[1], f->fld[3], d1, d2, f->fld[5], 1.);
}

static int
gfa1path(Graph *, File *)
{
	return 0;
}

// FIXME: actually handle bad input
static Graph*
loadgfa1(char *path)
{
	int nerr;
	int (*parse)(Graph*, File*);
	Graph *g;
	File *f;

	dprint("loadgfa1 %s\n", path);
	if((g = initgraph()) == nil)
		sysfatal("loadgfa1: %r");
	memset(&f, 0, sizeof f);
	if((f = graphopenfs(path, OREAD, g)) == nil)
		return nil;
	while(readrecord(f) != nil && f->err < 10){
		switch(f->fld[0][0]){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; break;
		case 'L': parse = gfa1link; break;
		case 'P': parse = gfa1path; break;
		default: werrstr("line %d: unknown record type %c", f->nr, *f->fld[0]); continue;
		}
		if(parse(g, f) < 0){
			warn("loadgfa1: %s\n", error());
			f->err++;
		}
	}
	dprint("done loading gfa\n");
	nerr = f->err;
	closefs(f);
	free(g->infile);
	g->infile = nil;
	idnuke(g->id2n);
	g->id2n = nil;
	if(nerr == 10){
		warn("loadgfa1: too many errors\n");
		nukegraph(g);
		return nil;
	}
	return g;
}

static int
save(Graph *)
{
	return 0;
}

// FIXME: more handles? readnode/readedge, etc?
static Filefmt ff = {
	.name = "gfa",
	.load = loadgfa1,
	.save = save,
};

Filefmt *
reggfa(void)
{
	return &ff;
}
