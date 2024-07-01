#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "cmd.h"

/* FIXME:
 * - add status messages => first cimgui stuff
 * - toggle node/edge labels on/off
 * - move gfa specific shit to fs/gfa.c → loadmeta function pointer
 */

static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int c, r, l, nerr;
	Node *n;

	for(n=g->nodes, nerr=c=0; n<g->nodes+dylen(g->nodes); n++){
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
	char *s, *t;
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

int
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

void
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
