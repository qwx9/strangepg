#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"
#include "graph.h"
#include "coarse.h"

/*
static char *
edge(char *p, char *s, char *e, char *u, char *v)
{
	p = seprint(p, e, "\t\"%d@%s\" -> \"%d@%s\"\n",
		U->parent, getname(U->parent), i, getname(i));
	if(writefs(f, s, p - s) < 0)
		return nil;
	p = 
}
*/

static int
maxdepth(CNode *U)
{
	int n, m, k;
	ioff i;

	n = (i = U->child) != -1;
	for(m=0; i!=-1; i=U->sibling){
		U = cnodes + i;
		if((k = maxdepth(U)) > m)
			m = k;
	}
	return n + m;
}

/* FIXMEp per component? */
static void
save(void *arg)
{
	int r, d;
	char *path, *l;
	ioff i, j;
	char buf[2048], *p;
	File *f;
	CNode *U, *UE;

	while((graph.flags & GFctarmed) == 0){
		DPRINT(Debugfs, "exportdot: waiting");
		sleep(1);
	}
	path = arg;
	if((f = openfs(path, OWRITE)) == nil){
		logerr(va("exportdot %s: %s\n", path, error()));
		return;
	}
	logmsg(va("exporting coarsening tree to %s...\n", path));
	r = -1;
	p = seprint(buf, buf + sizeof buf, "digraph {\n");
	if(writefs(f, buf, p - buf) < 0)
		goto end;
	for(i=0, U=cnodes, UE=U+nnodes; U<UE; U++, i++){
		p = buf;
		l = getname(i);
		if(U->idx == -1){	/* leaf node (needed only if collapsed) */
			p = seprint(p, buf + sizeof buf,
				"\t\"%d@%s\" [style=\"filled\"]\n",
				i, l);
		}
		d = maxdepth(U);
		for(j=d-1; j>=0; j--){	/* level edges */
			if(U->idx == -1)
				p = seprint(p, buf + sizeof buf,
					"\t\"%d@%s_%d\" [style=\"filled\"]\n",
					i, l, j+1);
			if(j > 0)
				p = seprint(p, buf + sizeof buf,
					"\t\"%d@%s_%d\" -> \"%d@%s_%d\"\n",
					i, l, j+1, i, l, j);
			else
				p = seprint(p, buf + sizeof buf,
					"\t\"%d@%s_%d\" -> \"%d@%s\"\n",
					i, l, j+1, i, l);
			if(p - buf >= sizeof buf / 2){
				if(writefs(f, buf, p - buf) < 0)
					goto end;
				p = buf;
			}
		}
		if(p > buf){
			if(writefs(f, buf, p - buf) < 0)
				goto end;
			p = buf;
		}
		if(U->parent == -1)
			continue;
		j = U->parent;
		if(d == 0)	/* leaf node, at max depth */
			p = seprint(p, buf + sizeof buf,
				"\t\"%d@%s_1\" -> \"%d@%s\"\n",
				j, getname(j), i, l);
		else	/* inner node at level below parent's */
			p = seprint(p, buf + sizeof buf,
				"\t\"%d@%s_%d\" -> \"%d@%s_%d\"\n",
				j, getname(j), d+1, i, l, d);
		/*
		p = seprint(p, buf + sizeof buf, "\t\"%d@%s\" -> \"%d@%s\"\n",
			U->parent, getname(U->parent), i, getname(i));
		*/
		if(writefs(f, buf, p - buf) < 0)
			goto end;
	}
	p = seprint(buf, buf + sizeof buf, "}\n");
	r = writefs(f, buf, p - buf);
end:
	if(r < 0)
		logerr(va("exportdot: %s\n", error()));
	else
		logmsg("done exporting coarsening tree.\n");
	freefs(f);
}


static Filefmt ff = {
	.name = "dot",
	.save = save,
};

Filefmt *
regdot(void)
{
	return &ff;
}
