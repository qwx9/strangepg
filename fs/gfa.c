#include "strpg.h"
#include "fsprv.h"

static int
gfa1hdr(Graph *g, File *f)
{
	if(f->nf != 2){
		werrstr("line %d: malformed header", f->nr);
		return -1;
	}
	USED(g);
	return 0;
}

static int
gfa1seg(Graph *g, File *f)
{
	if(f->nf != 3){
		werrstr("line %d: malformed segment", f->nr);
		return -1;
	}
	addnode(g, strtoull(f->fld[1], nil, 0), f->fld[2]);
	return 0;
}

static int
ugh(char *s, usize *u)
{
	assert(u != nil);
	if(s[0] == '+' && s[1] == 0){
		*u |= Vforward;
		return 0;
	}else if(s[0] == '-' && s[1] == 0){
		*u |= Vreverse;
		return 0;
	}
	return -1;
}

static int
gfa1link(Graph *g, File *f)
{
	usize u, v;

	if(f->nf != 6){
		werrstr("line %d: malformed link", f->nr);
		return -1;
	}
	u = strtoull(f->fld[1], nil, 10) << 1;
	v = strtoull(f->fld[3], nil, 10) << 1;
	if(ugh(f->fld[2], &u) < 0 || ugh(f->fld[4], &v) < 0){
		werrstr("line %d: malformed link", f->nr);
		return -1;
	}
	addedge(g, u, v, f->fld[5], 0.);
	return 0;
}

static int
gfa1path(Graph *g, File *f)
{
	USED(g, f);
	return 0;
}

// FIXME: actually handle bad input + handle no graph loaded in main
static Graph*
loadgfa1(char *path)
{
	int (*parse)(Graph*, File*);
	Graph *g;
	File f;

	warn("loadgfa1 %s\n", path);
	if((g = initgraph()) == nil)	
		sysfatal("loadgfa1: ");
	memset(&f, 0, sizeof f);
	f.path = path;
	while(readrecord(&f) != nil){
		switch(f.fld[0][0]){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; break;
		case 'L': parse = gfa1link; break;
		case 'P': parse = gfa1path; break;
		default: werrstr("line %d: unknown record type %c", f.nr, *f.fld[0]); continue;
		}
		if(parse(g, &f) < 0){
			f.err++;
			break;
		}
	}
	warn("shit %r\n");
	if(f.err){
		nukegraph(g);
		return nil;
	}
	warn("done loading gfa\n");
	return g;
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
};

Filefmt *
reggfa(void)
{
	return &ff;
}
