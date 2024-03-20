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
			/* ignore length if sequence was inlined */
			if(l < 6 || s[2] != ':' || s[4] != ':'){
				warn("invalid metadata field %s\n", s);
				continue;
			}
			if(strncmp(s, "LN", 2) == 0 && r){
				warn("node[%zx]: ignoring redundant length field\n", n->id);
				continue;
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
	g->loading = 1;
	f = g->f;
	if(collectgfanodes(g, f) < 0)
		warn("collectmeta: %s\n", error());
	if(collectgfaedges(g, f) < 0)
		warn("collectmeta: %s\n", error());
	pushcmd("FGD135");
	g->loading = 0;
}

void
clearmeta(Graph *g)
{
	if(g->strnmap == nil)
		return;
	kh_destroy(strmap, g->strnmap);
	g->strnmap = nil;
}
