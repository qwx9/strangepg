#include "strpg.h"
#include "fs.h"
#include "cmd.h"

// FIXME: this symbolically should really be just a bunch of hash tables
//	key == variable + index -> var:i --h--> NA, cached, or loadreq
//	functions the exact same way as EM in a way...
//	make it so!
//	lighter alternative for now: pass it to awk and make it parse and store
//	all metadata

// so design (fawk? 0wk?):
// hashmap of keys maps to index in registry, linking corresponding table
//	either flat full EM-based table, or hash table, doesn't matter which
//	(make queries transparent)

// add status messages => first cimgui stuff

// toggle node/edge labels on/off

// compile new script with existing variables, or just hardcode known ones
// alt: reading the script is fast, reading the data is slow -> just rerun?
//	plays right into fawk's hands

// we disallow setting/creating new variables
//	blue would be a variable so we don't have to type "blue"
// goal: color[NC == 3] = blue
//	-> ?u
// also: load from csv/...


// FIXME: more abstractions, fp -> gfa.c
static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int l, nerr;
	Node *n;

	for(n=g->nodes, nerr=0; n<g->nodes+dylen(g->nodes); n++){
		seekfs(f, n->metaoff);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("too many errors");
				return -1;
			}
			continue;
		}
		if((s = getfield(s)) == nil		/* S */
		|| (s = getfield(s)) == nil)	/* id */
			continue;
		t = getfield(s);
		// FIXME: truncated lines
		l = t != nil ? t - s : strlen(s);
		if(l > 1 || l == 1 && s[0] != '*')
			pushcmd("s NLN %d %d", n->id, l);
		// FIXME: don't overwrite LN from here
		for(s=t; s!=nil; s=t){
			t = getfield(s);
			l = t != nil ? t - s : strlen(s);
			if(l < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("s N%s %d %s", s, n->id, s+5);
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
		if((s = getfield(s)) == nil		/* L */
		|| (s = getfield(s)) == nil		/* u */
		|| (s = getfield(s)) == nil		/* du */
		|| (s = getfield(s)) == nil		/* v */
		|| (s = getfield(s)) == nil)	/* dv */
			continue;
		t = getfield(s);
		// FIXME: truncated lines
		l = t != nil ? t - s : strlen(s);
		if(l > 1 || l == 1 && s[0] != '*')
			pushcmd("s cig %d %s", e->id, s);
		for(s=t; s!=nil; s=t){
			t = getfield(s);
			l = t != nil ? t - s : strlen(s);
			if(l < 6 || s[2] != ':' || s[4] != ':')
				continue;
			s[2] = 0;
			pushcmd("s L%s %d %s", s, e->id, s+5);
		}
		nerr = 0;
	}
	return 0;
}

// FIXME: more abstractions, fp -> gfa.c (fs.c load structs)
void
collectgfameta(Graph *g)
{
	File *f;

	clearmeta(g);	/* delegated to awk */
	g->loading = 1;
	f = g->f;
	// FIXME: wait until meta is loaded to launch meta commands
	if(collectgfanodes(g, f) < 0)
		warn("collectmeta: %s\n", error());
	if(collectgfaedges(g, f) < 0)
		warn("collectmeta: %s\n", error());
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
