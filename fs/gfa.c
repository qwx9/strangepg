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
 * - length of inlined sequence trumps LN tag
 */

KHASHL_MAP_INIT(KH_LOCAL, namemap, names, char*, ioff, kh_hash_str, kh_eq_str)

typedef struct Aux Aux;
struct Aux{
	Graph g;
	namemap *names;
	vlong *nodeoff;
	vlong *edgeoff;
};

static void
cleanup(Aux *a)
{
	khint_t k;
	namemap *h;

	if((h = a->names) != nil){
		kh_foreach(h, k)
			free(kh_key(h, k));
		names_destroy(h);
	}
	dyfree(a->nodeoff);
	dyfree(a->edgeoff);
	closefs(a->g.f);	/* closed but reopenable */
}

static ioff
getid(namemap *h, char *s)
{
	ioff id;
	khint_t k;

	k = names_get(h, s);
	if(k == kh_end(h))
		return -1;
	id = kh_val(h, k);
	return id;
}
static ioff
pushid(namemap *h, char *s, ioff id)
{
	int abs;
	khint_t k;

	k = names_put(h, s, &abs);
	assert(abs);
	kh_val(h, k) = id;
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

static inline void
setlength(Node *n, ioff id, int len)
{
	n->length = len;
	pushcmd("LN[label[%d]] = %d", id, len);
	rnodes[id].len = len;
}

static int
collectgfanodes(File *f, vlong *offs, Node *nodes, namemap *h)
{
	char *s;
	int c, m, l, nerr, minl, maxl;
	ioff id, i, ie;
	vlong *off;
	Node *n;

	minl = 0x7fffffff;
	maxl = 1;
	for(off=offs,i=0,ie=dylen(offs),nerr=c=0; i<ie; i++, off++){
		seekfs(f, *off);
		if(readline(f) == nil)
			goto err;
		if(nextfield(f) == nil	/* S */
		|| (s = nextfield(f)) == nil)	/* id */
			goto err;
		if((id = getid(h, s)) < 0)
			sysfatal("collectgfanodes: bug: %s not found", s);
		n = nodes + id;
		DPRINT(Debugmeta, "collectgfanodes node[%d]: %s %d", id, s, f->trunc);
		if((s = nextfield(f)) != nil){
			if((l = f->toksz) > 0){
				if(l > 1 || *s != '*'){
					setlength(n, id, l);
					c++;
				}
			/* do not tolerate empty field here */
			}else
				goto err;
		}else if((l = f->toksz) > 0){	/* field just too long */
			setlength(n, id, l);
			c++;
		}else
			goto err;
		while((s = nextfield(f)) != nil){
			DPRINT(Debugmeta, "f: \"%s\"%s", s, f->trunc?" (trunc)":"");
			if(f->toksz < 5 || s[2] != ':' || s[4] != ':'){
				warn("segment id=%d: invalid tag \"%s\"\n", id, s);
				continue;
			}
			s[2] = 0;
			/* ignore length if sequence was inlined */
			if(strcmp(s, "LN") == 0 && l > 0){
				m = atoi(s + 5);
				if(m != l)
					warn("segment[%d]: conflicting sequence length %d with LN=%d\n", id, l, m);

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
		continue;
err:
		warn("collectgfanodes: invalid S record %d\n", i);
		if(++nerr > 10){
			werrstr("too many errors");
			return -1;
		}
	}
	if(maxl > 1)
		fixlengths(minl, maxl);
	return c;
}

static int
collectgfaedges(File *f, vlong *offs)
{
	int nerr;
	ioff i, ie;
	vlong *off;
	char *s;

	for(off=offs,i=0,ie=dylen(offs),nerr=0; i<ie; i++, off++){
		seekfs(f, *off);
		if((s = readline(f)) == nil)
			goto err;
		DPRINT(Debugmeta, "collectgfaedges edges[%d]: %d %s", i, f->trunc, s);
		if(nextfield(f) == nil	/* L */
		|| nextfield(f) == nil	/* u */
		|| nextfield(f) == nil	/* du */
		|| nextfield(f) == nil	/* v */
		|| nextfield(f) == nil	/* dv */
		|| (s = nextfield(f)) == nil)	/* cigar */
			goto err;
		pushcmd("cigar[ledge[%d]] = \"%s\"", i, s);
		while((s = nextfield(f)) != nil){
			if(f->toksz < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("%s[ledge[%d]] = \"%s\"", s, i, s+5);
		}
		nerr = 0;
		continue;
err:
		warn("collectgfaedges: invalid L record %d\n", i);
		if(++nerr > 10){
			werrstr("too many errors");
			return -1;
		}
	}
	return 0;
}

static int
collectgfameta(Aux *a)
{
	int n, m;

	if((n = collectgfanodes(a->g.f, a->nodeoff, a->g.nodes, a->names)) < 0)
		return -1;
	if((m = collectgfaedges(a->g.f, a->edgeoff)) < 0)
		return -1;
	a->g.flags |= GFarmed;
	return n + m;
}

static inline ioff
pushnode(Graph *g, namemap *h, char *s)
{
	ioff id;

	/* nodes may be defined after an edge references them, should
	 * not be considered as an error */
	if((id = getid(h, s)) >= 0){
		DPRINT(Debugfs, "duplicate node[%d] %s", id, s);
		return id;
	}
	s = estrdup(s);
	if((id = newnode(g, s)) < 0 || pushid(h, s, id) < 0){
		free(s);
		return -1;
	}
	return id;
}

static inline ioff
pushedge(Graph *g, namemap *h, char *eu, char *ev, int d1, int d2)
{
	char *s, *sf;
	ioff id, f, n, u, v;

	if((u = pushnode(g, h, eu)) < 0 || (v = pushnode(g, h, ev)) < 0)
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
	if((id = getid(h, s)) >= 0){
		DPRINT(Debugfs, "duplicate edge[%d] %s%s", id, s, f ? " (flipped)" : "");
		free(s);
		free(sf);
		return id;
	}
	u = u << 1 | d1;
	v = v << 1 | d2;
	/* always push only what was actually in the input */
	if((id = newedge(g, u, v, sf != nil ? sf : s)) < 0 || pushid(h, s, id) < 0){
		free(s);
		id = -1;
	}
	free(sf);
	return id;
}

static inline int
gfa1seg(Aux *a)
{
	char *s;
	File *f;

	f = a->g.f;
	s = nextfield(f);
	DPRINT(Debugfs, "gfa pushnode %s", s);
	if(pushnode(&a->g, a->names, s) < 0)
		return -1;
	dypush(a->nodeoff, f->foff);
	return 0;
}

static inline int
todir(char *s)
{
	if(strncmp(s, "+", 2) == 0)
		return Vforward;
	else if(strncmp(s, "-", 2) == 0)
		return Vreverse;
	return -1;
}
static inline int
gfa1link(Aux *a)
{
	int d1, d2;
	char *s, *fld[4], **fp;
	File *f;

	f = a->g.f;
	for(fp=fld; fp<fld+nelem(fld); fp++){
		if((s = nextfield(f)) == nil){
			werrstr("malformed link");
			return -1;
		}
		*fp = s;
	}
	if((d1 = todir(fld[1])) < 0 || (d2 = todir(fld[3])) < 0){
		werrstr("malformed link orientation");
		return -1;
	}
	DPRINT(Debugfs, "gfa pushedge %c%s,%c%s", *fld[1], fld[0], *fld[3], fld[2]);
	if(pushedge(&a->g, a->names, fld[0], fld[2], d1, d2) < 0)
		return -1;
	dypush(a->edgeoff, f->foff);
	return 0;
}

static File *
opengfa(Aux *a, char *path)
{
	File *f;

	a->g = initgraph(FFgfa);
	if((f = openfs(path, OREAD)) == nil)
		sysfatal("loadgfa %s: %s", path, error());
	free(path);
	a->g.f = f;
	a->names = names_init();
	return f;
}

static void
loadgfa1(void *arg)
{
	int n;
	char *s;
	ioff nerr, nn, ne;
	File *f;
	Aux a = {0};

	f = opengfa(&a, arg);
	nn = ne = nerr = 0;
	while(readline(f) != nil){
		if(nerr >= 10)
			sysfatal("loadgfa1: too many errors, last error: %s\n", error());
		if((s = nextfield(f)) == nil){
			warn("null pointer\n");
			break;
		}
		switch(s[0]){
		err:
			warn("loadgfa1: line %d: %s\n", f->nr, error());
			nerr++;
			break;
		case 'H':
		case 'P': continue;
		case 'S':
			if(gfa1seg(&a) < 0)
				goto err;
			if(++nn % 100000 == 0)
				warn("loadgfa: %de5 nodes...\n", nn / 100000);
			break;
		case 'L':
			if(gfa1link(&a) < 0)
				goto err;
			if(++ne % 100000 == 0)
				warn("loadgfa: %de5 edges...\n", ne/ 100000);
			break;
		default: DPRINT(Debugfs, "line %d: unknown record type %c", f->nr, s[0]);
		}
	}
	if(nn == 0)
		sysfatal("loadgfa1: empty graph: %s", error());
	if(dylen(a.nodeoff) < nn)
		sysfatal("loadgfa1: invalid GFA: missing S lines, links reference non-existent segments");
	if(ne == 0)
		logmsg("loadgfa: no edges\n");
	pushgraph(&a.g);
	logmsg("loadgfa: loading tags...\n");
	if((n = collectgfameta(&a)) < 0)
		warn("loadgfa: loading metadata failed: %s\n", error());
	/* if no actual useful metadata was loaded, don't do anything */
	else if(n > 0 || !gottagofast)
		pushcmd("cmd(\"FHJ142\")");
	logmsg("loadgfa: done\n");
	pushcmd("cmd(\"FGD135\")");	/* FIXME: after only one input file? */
	cleanup(&a);
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
