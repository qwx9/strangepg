#include "strpg.h"
#include "fs.h"
#include "em.h"
#include "index.h"

int
main(int argc, char **argv)
{
	int i;
	ssize u, v, idx, par, w, ei;
	Graph g;
	Level *l;
	File *f;
	Coarse *c;

	ARGBEGIN{
	}ARGEND
	if(*argv == nil)
		sysfatal("usage: %s BCT", argv0);
	g = initgraph(FFindex);
	if(readtree(&g, argv[0]) < 0)
		sysfatal("load: failed to read tree %s: %s", argv[0], error());
	print("%zd nodes %zd supers %zd edges\n", g.nnodes, g.nsuper, g.nedges);
	c = g.c;
	f = g.f;
	for(l=c->levels; l<c->levels+dylen(c->levels); l++){
		print("level %zd nodes[%zd] @%zd len[%zd] @%zd\n",
			l-c->levels, l->nnodes, l->noff, l->nedges, l->eoff);
		seekfs(f, l->noff);
		print("\tnodes:");
		for(i=0; i<l->nnodes; i++){
			u = eget64(f);
			idx = eget64(f);
			par = eget64(f);
			w = eget64(f);
			print(" %zx[%zx]←%zx:%zd", u, idx, par, w);
		}
		print("\n\tedges:");
		seekfs(f, l->eoff);
		for(i=0; i<l->nedges; i++){
			u = eget64(f);
			v = eget64(f);
			ei = eget64(f);
			print(" [%zx]%zx,%zx", ei, u&1?-u/2:u/2, v&1?-v/2:v/2);
		}
		print("\n");
	}
	closefs(f);
	exits(nil);
}
