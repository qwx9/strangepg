#include "strpg.h"
#include "fs.h"
#include "em.h"
#include <bio.h>

/* FIXME: indirection + looping through everything
 * somewhat inefficient, but  degree would usually be low;
 * alternative: adjacency matrix, still EM-backed → benchmark! */
static int
exists(usize s, usize t, EM *fs2j, EM *fjump, EM *fedge, usize nedges)
{
	usize i, p;

	warn("exists %zx,%zx\n", s, t);
	if((i = emp64(fs2j, s)) == EMbupkis)
		return 0;
	do{
		if((p = emp64(fjump, i)) == EMbupkis)
			return 0;
		warn("exists %zx,%zx: check %zx: %zx,%zx\n", s, t, p, emp64(fedge, 2+2*p), emp64(fedge, 2+2*p+1));
		if(emp64(fedge, 2+2*p) != s)
			break;
		if(emp64(fedge, 2+2*p+1) == t)
			return 1;
	}while(++i < nedges);
	return 0;
}

static void
insert(usize s, EM *fs2j, EM *fjump, EM *fedge, usize nedges)
{
	usize i, p;

	if((i = emp64(fs2j, s)) != EMbupkis){
		p = nedges;
		while(i < nedges){
			if((p = emp64(fjump, i)) == EMbupkis
			|| emp64(fedge, p) != s)
				break;
			i++;
		}
		if(p != nedges){
			emw64(fjump, i, nedges);
			emw64(fjump, nedges, p);
			warn("insert %zx ← %zx\n", nedges, p);
			warn("insert %zx ← %zx\n", i, nedges);
		}else{
			emw64(fjump, nedges, nedges);
			warn("insert %zx ← %zx\n", nedges, nedges);
		}
	}else{
		emw64(fs2j, s, nedges);
		emw64(fjump, nedges, nedges);
		warn("insert %zx ← %zx\n", nedges, nedges);
	}
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
		if((s = emp64(fseen, u)) == EMbupkis){
			emw64(fseen, u, S);
			warn("S %zx ← %zx\n", u, S);
			s = S++;
			topdog = u;
		}
		if((t = emp64(fseen, v)) == EMbupkis){
			if(u == topdog){
				warn("M %zx ← %zx\n", v, s);
				emw64(fseen, v, s);
			}else
				warn("_ %zx : %zx not %zx\n", v, u, s);
		}else if(u == v)
			warn("≡ %zx %zx\n", u, v);
		else if(t == s)
			warn("↔ %zx %zx\n", u, v);
		else if(exists(s, t, fs2j, fjump, fedge, M)
		|| exists(t, s, fs2j, fjump, fedge, M))
			warn("R %zx %zx\n", s, t);
		else{
			emw64(fedge, 2+2*M, s);
			emw64(fedge, 2+2*M+1, t);
			insert(s, fs2j, fjump, fedge, M);
			warn("%zx,%zx\n", s, t);
			M++;
		}
		free(fs);
	}
	for(i=0; i<M; i++){
		s = emp64(fedge, 2+2*i);
		t = emp64(fedge, 2+2*i+1);
		warn("E %zx %zx\n", s, t);
	}
	emclose(fedge);
	emclose(fjump);
	emclose(fs2j);
	emclose(fseen);
	sysquit();
}
