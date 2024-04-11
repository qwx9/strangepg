#include "strpg.h"
#include "fs.h"
#include "cmd.h"

/* FIXME:
 * - add status messages => first cimgui stuff
 * - toggle node/edge labels on/off
 * - load metadata from csv/tsv
 * - move gfa specific shit to fs/gfa.c → loadmeta function pointer
 * - buffer commands until meta is loaded to launch meta commands
 *	awk: save to batch buffer, then send it back
 *	then strpg sends it back again?
 */

static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int r, l, nerr;
	Node *n;

	for(n=g->nodes, nerr=0; n<g->nodes+dylen(g->nodes); n++){
		r = 0;
		seekfs(f, n->metaoff);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		DPRINT(Debugmeta, "collectgfanodes node[%zd]: %d %s", n-g->nodes, f->trunc, s);
		if((s = nextfield(f, s, nil)) == nil	/* S */
		|| (s = nextfield(f, s, nil)) == nil)	/* id */
			continue;
		t = nextfield(f, s, &l);	/* seq */
		if(l > 1 || s[0] != '*'){
			pushcmd("LN[\"n\",%d] = %d", n->id, l);
			r = 1;
		}
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, &l);
			if(l < 5 || s[2] != ':' || s[4] != ':'){
				warn("node[%zd]: invalid segment metadata field \"%s\"\n", n-g->nodes, s);
				continue;
			}
			/* FIXME: don't assume two letter tags? */
			/* ignore length if sequence was inlined */
			if(strncmp(s, "LN", 2) == 0 && r){
				DPRINT(Debugmeta, "node[%zx]: ignoring redundant length field", n->id);
				continue;
			/* FIXME: no error checking */
			}else if(strncmp(s, "fx", 2) == 0){
				n->flags |= FNfixed;
				n->fixed.x = atof(s+5);
			}else if(strncmp(s, "fy", 2) == 0){
				n->flags |= FNfixed;
				n->fixed.y = atof(s+5);
			}else if(strncmp(s, "mv", 2) == 0){
				n->flags |= FNinitpos;	/* FIXME */
				n->fixed.x = atof(s+5);
			}
			s[2] = 0;
			pushcmd("%s[\"n\",%d] = \"%s\"", s, n->id, s+5);
		}
		nerr = 0;
	}
	return 0;
}

static int
collectgfaedges(Graph *g, File *f)
{
	char *s, *t;
	int l, nerr;
	Edge *e;

	for(e=g->edges, nerr=0; e<g->edges+dylen(g->edges); e++){
		seekfs(f, e->metaoff);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		DPRINT(Debugmeta, "collectgfaedges edges[%zd]: %d %s", e-g->edges, f->trunc, s);
		if((s = nextfield(f, s, nil)) == nil	/* L */
		|| (s = nextfield(f, s, nil)) == nil	/* u */
		|| (s = nextfield(f, s, nil)) == nil	/* du */
		|| (s = nextfield(f, s, nil)) == nil	/* v */
		|| (s = nextfield(f, s, nil)) == nil)	/* dv */
			continue;
		t = nextfield(f, s, nil);
		if(l > 0)
			pushcmd("cigar[%d] = \"%s\"", e->id, s);
		for(s=t; s!=nil; s=t){
			t = nextfield(f, s, nil);
			if(l < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("%s[\"e\",%d] = \"%s\"", s, e->id, s+5);
		}
		nerr = 0;
	}
	return 0;
}

void
collectgfameta(Graph *g)
{
	File *f;

	clearmeta(g);	/* delegated to awk */
	f = g->f;
	if(collectgfanodes(g, f) < 0)
		warn("collectmeta: %s\n", error());
	if(collectgfaedges(g, f) < 0)
		warn("collectmeta: %s\n", error());
	g->flags |= FGarmed;
	pushcmd("FGD135");
}

void
clearmeta(Graph *g)
{
	if(g->strnmap == nil)
		return;
	kh_destroy(strmap, g->strnmap);
	g->strnmap = nil;
}
