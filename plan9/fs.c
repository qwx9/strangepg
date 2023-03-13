#include "strpg.h"
#include <bio.h>

/*
klib etc is hyperoptimizing at this point
instead do the usual game shit,
just a predefined struct with a graph,
and test with that
delegate any kind of loading or shit for later
use the same approach for everything else
and get this shit up

// FIXME: don't write our own parse; there already is gfatools
//	just shim it for now with some simple graph... then maybe fork gfatools?

// FIXME: must be fully up to spec (v1.1)
int
loadfsgfa1(char *path)
{
	int nf, nr;
	char *s, *fld[16];
	Biobuf *bf;
	... graph g ...

	if((bf = Bopen(path, OREAD)) == nil)
		return -1;
	nr = 0;
	while((s = Brdstr(bf, '\n', 0)) != nil){
		if((nf = getfields(s+2, fld, nelem(fld), 1, "\t ")) < 2){
			werrstr("line %d: invalid record", nr+1);
			goto err;
		}
		switch(*s){
		case 'H': parse = gfa1hdr; break;
		case 'S': parse = gfa1seg; break;
		case 'L': parse = gfa1link; break;
		case 'P': parse = gfa1path; break;
		default: werrstr("line %d: unknown record type %c", nr+1, *s); goto err;
		}
		if(parse(g, fld, nf) < 0)
			goto err;




		free(s);
		nr++;
	}
	Bterm(bf);
	return 0;
err:
	free(s);
	return -1;
}
*/
