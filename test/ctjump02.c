#include "strpg.h"
#include "fs.h"
#include "em.h"
#include <bio.h>

/* FIXME: indirection + looping through everything
 * somewhat inefficient, but  degree would usually be low;
 * alternative: adjacency matrix, still EM-backed → benchmark!
 * alternative 2: hash table, in-between solution: hash s*NS+t or sth
 *	only hashing external edges → potentially very small
 *	edge list is current external edges as well: couldn't we use that info?
 * see if we couldn't just use EM as is for it
 * ... all of this just to detect redundancies? other way? see above
 * + reordering
 * + reduce number of tables by combining them */
static int
exists(usize s, usize t, EM *fs2j, EM *fjump, EM *fedge, usize nedges)
{
	usize i, p;

	if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "exists %zx,%zx", s, t);}
	if((i = emr64(fs2j, s)) == EMbupkis)
		return 0;
	do{
		if((p = emr64(fjump, i)) == EMbupkis)
			return 0;
		if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "exists %zx,%zx: check %zx: %zx,%zx", s, t, p, emr64(fedge, 2+2*p), emr64(fedge, 2+2*p+1));}
		if(emr64(fedge, 2+2*p) != s)
			break;
		if(emr64(fedge, 2+2*p+1) == t)
			return 1;
	}while(++i < nedges);
	return 0;
}

static void
insert(usize s, EM *fs2j, EM *fjump, EM *fedge, usize nedges)
{
	usize i, p;

	if((i = emr64(fs2j, s)) != EMbupkis){
		p = nedges;
		while(i < nedges){
			if((p = emr64(fjump, i)) == EMbupkis
			|| emr64(fedge, p) != s)
				break;
			i++;
		}
		if(p != nedges){
			emw64(fjump, i, nedges);
			emw64(fjump, nedges, p);
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "insert %zx ← %zx", nedges, p);}
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "insert %zx ← %zx", i, nedges);}
		}else{
			emw64(fjump, nedges, nedges);
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "insert %zx ← %zx", nedges, nedges);}
		}
	}else{
		emw64(fs2j, s, nedges);
		emw64(fjump, nedges, nedges);
		if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "insert %zx ← %zx", nedges, nedges);}
	}
}

static void
usage(void)
{
	sysfatal("usage: %s [-D what] INDEX\n", argv0);
}

void
threadmain(int argc, char **argv)
{
	int i, topdog, M;
	char *fs, *fld[2];
	usize s, t, S, u, v;
	EM *fedge, *fjump, *fs2j, *fseen;
	Biobuf *bf;

	ARGBEGIN{
	case 'D':
		fs = EARGF(usage());
		if(strcmp(fs, "draw") == 0)
			debug |= Debugdraw;
		else if(strcmp(fs, "render") == 0)
			debug |= Debugrender;
		else if(strcmp(fs, "layout") == 0)
			debug |= Debuglayout;
		else if(strcmp(fs, "fs") == 0)
			debug |= Debugfs;
		else if(strcmp(fs, "coarse") == 0)
			debug |= Debugcoarse;
		else if(strcmp(fs, "extmem") == 0)
			debug |= Debugextmem;
		else if(strcmp(fs, "all") == 0)
			debug |= Debugtheworld;
		else{
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "unknown debug component %s", fs);}
			usage();
		}
		break;
	}ARGEND
	if((bf = Bfdopen(0, OREAD)) == nil)
		sysfatal("bopen: %s", error());
	fedge = emnew(0);
	fseen = emnew(0);
	fs2j = emnew(0);
	fjump = emnew(0);
	emw64(fedge, 0, 0);
	emw64(fedge, 1, 0);
	S = 0;
	M = 0;
	topdog = 0;
	while((fs = Brdstr(bf, '\n', 0)) != nil){
		if(getfields(fs, fld, nelem(fld), 1, " \t") != nelem(fld))
			sysfatal("getfields: short read: %s", error());
		u = strtoull(fld[0], nil, 10) - 1;	/* awk indexing */
		v = strtoull(fld[1], nil, 10) - 1;
		if((s = emr64(fseen, u)) == EMbupkis){
			emw64(fseen, u, S);
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "S %zx ← %zx", u, S);}
			s = S++;
			topdog = u;
		}
		if((t = emr64(fseen, v)) == EMbupkis){
			if(u == topdog){
				if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "M %zx ← %zx", v, s);}
				emw64(fseen, v, s);
			}else
				if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "_ %zx : %zx not %zx", v, u, s);}
		}else if(u == v)
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "≡ %zx %zx", u, v);}
		else if(t == s)
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "↔ %zx %zx", u, v);}
		else if(exists(s, t, fs2j, fjump, fedge, M)
		|| exists(t, s, fs2j, fjump, fedge, M))
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "R %zx %zx", s, t);}
		else{
			emw64(fedge, 2+2*M, s);
			emw64(fedge, 2+2*M+1, t);
			insert(s, fs2j, fjump, fedge, M);
			if((debug & Debugcoarse) != 0){dprint(Debugcoarse, "%zx,%zx", s, t);}
			M++;
		}
		free(fs);
	}
	for(i=0; i<M; i++){
		s = emr64(fedge, 2+2*i);
		t = emr64(fedge, 2+2*i+1);
		print("%zx %zx", s, t);
	}
	emclose(fedge);
	emclose(fjump);
	emclose(fs2j);
	emclose(fseen);
	sysquit();
}
