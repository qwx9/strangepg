#include "strpg.h"
#include "fs.h"
#include "cmd.h"

// FIXME: don't clearmeta(g) in loadgfa

// FIXME: this symbolically should really be just a bunch of hash tables
//	key == variable + index -> var:i --h--> NA, cached, or loadreq
//	functions the exact same way as EM in a way...
//	make it so!
//	lighter alternative for now: pass it to awk and make it parse and store
//	all metadata

// so design:
// hashmap of keys maps to index in registry, linking corresponding table
//	either flat full EM-based table, or hash table, doesn't matter which
//	(make queries transparent)

// but, in practice, until we get our new not-fawk:
// parse meta in separate worker and feed to awk

// awk side: execute request and show result

// add status messages => first cimgui stuff

// toggle node/edge labels on/off


// FIXME: more abstractions, fp -> gfa.c
static int
collectgfanodes(Graph *g, File *f)
{
	char *s, *t;
	int l, nerr;
	Node *n;

	// FIXME: wrong id??
	for(n=g->nodes, nerr=0; n<g->nodes+dylen(g->nodes); n++){
		seekfs(f, n->metaoff);
		warn("node %zx off %lld\n", n->id, n->metaoff);
		if((s = readline(f, &l)) == nil){
			warn("collectmeta: %s\n", error());
			if(++nerr > 10){
				werrstr("collectmeta: too many errors");
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
			pushcmd("s NLN %d %d", n->id, n);
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

// FIXME: more abstractions, fp -> gfa.c
void
collectgfameta(Graph *g)
{
	File *f;

	g->loading = 1;
	f = g->f;
	//clearmeta(g);	/* delegated to awk */
	if(collectgfanodes(g, f) < 0)
		warn("collectmeta: %s\n", error());


// FIXME: compile awk script with existing variables
// do simple trial
// goal: color nodes
// alt: reading the script is fast, reading the data is slow -> just rerun?
//	plays right into fawk's hands

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
