#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

void
threadmain(int argc, char **argv)
{
	int i;
	ssize u, v, idx, par, w, ei;
	Graph *g;
	Level *l;
	File *f;
	Coarse *c;

	ARGBEGIN{
	}ARGEND
	if(*argv == nil)
		sysfatal("usage: %s BCT", argv0);
	if((g = initgraph()) == nil)
		sysfatal("initgraph: %r");
	if(readtree(g, argv[0]) < 0)
		sysfatal("load: failed to read tree %s: %s", argv[0], error());
	print("%zd nodes %zd supers %zd edges\n", g->nnodes, g->nsuper, g->nedges);
	c = g->c;
	f = g->f;
	for(l=c->levels; l<c->levels+dylen(c->levels); l++){
		print("level %zd nodes[%zd] @%zd len[%zd] @%zd\n",
			l-c->levels, l->nnodes, l->noff, l->nedges, l->eoff);
		seekfs(f, l->noff);
		print("\tnodes:");
		for(i=0; i<l->nnodes; i++){
			u = get64(f);
			idx = get64(f);
			par = get64(f);
			w = get64(f);
			print(" %zx[%zx]←%zx:%zd", u, idx, par, w);
		}
		print("\n\tedges:");
		seekfs(f, l->eoff);
		for(i=0; i<l->nedges; i++){
			u = get64(f);
			v = get64(f);
			ei = get64(f);
			print(" [%zx]%zx,%zx", ei, u&1?-u/2:u/2, v&1?-v/2:v/2);
		}
		print("\n");
	}
	closefs(f);
	exits(nil);
}
