#include "strpg.h"
#include "fs.h"
#include "threads.h"
#include "drw.h"
#include "cmd.h"
#include "graph.h"
#include "coarse.h"

/* FIXME: errors here cause shit to hang after gfa is loaded, ie no draw */

static inline int
getval(char *s, ioff *ip)
{
	char *p;
	ioff i;

	i = strtoll(s, &p, 10);	/* FIXME: use hex format? */
	if(p == s || i < -1 || i >= nnodes)
		return -1;
	*ip = i;
	return 0;
}

enum{
	Recsz = sizeof(ioff) * 3,
};

/* cedges don't need to be allocated initially */
static void
load(void *arg)
{
	int x, r;
	ioff idx;
	union { u32int u; s32int i; } b;
	uchar buf[Recsz], *p;
	char *path;
	File *f;
	CNode *U, *UE;
	Node *u;

	path = arg;
	if(graph.flags & GFctarmed){
		logerr(va("importctab %s: ctab already initialized\n", path));
		return;
	}
	logmsg(va("importing coarsening table from %s...\n", path));
	r = -1;
	if((f = openfs(path, OREAD)) == nil)
		goto end;
	if(initcoarse() < 0)
		goto end;
	for(idx=x=0, u=nodes, U=cnodes, UE=U+nnodes; U<UE; U++, u++, idx++){
		if((x = readfs(f, buf, sizeof buf)) < sizeof buf)
			break;
		p = buf;
		b.u = GBIT32(p);
		p += sizeof b.u;
		U->parent = b.i;
		b.u = GBIT32(p);
		p += sizeof b.u;
		U->child = b.i;
		b.u = GBIT32(p);
		p += sizeof b.u;
		U->sibling = b.i;
		U->idx = idx;
		U->eoff = u->eoff;
		U->nedges = u->nedges;
	}
	if(U != UE){
		werrstr("too few records in ctab, %zd < %d", U-cnodes, nnodes);
		r = -1;
	/* FIXME: detect too large files by an extra read and x == 0 */
	}else
		r = x < 0 ? -1 : 0;
end:
	if(r < 0)	/* ctab corrupted, can't continue */
		sysfatal("importctab %s line %d: aborting: %s", path, f->nr, error());
	graph.flags |= GFctarmed;
	pushcmd("cmd(\"HGI234\")");	/* signal needed to continue, error or no */
	flushcmd();
	logmsg("done importing coarsening table.\n");
	freefs(f);
}

static void
save(void *arg)
{
	int x;
	char *path;
	union { u32int u; s32int i; } b;
	uchar buf[Recsz], *p;
	File *f;
	CNode *U, *UE;

	path = arg;
	if((f = openfs(path, OWRITE)) == nil){
		logerr(va("exportctab %s: %s\n", path, error()));
		return;
	}
	while((graph.flags & GFctarmed) == 0){
		DPRINT(Debugfs, "exportctab: waiting");
		sleep(1);
	}
	logmsg(va("exporting coarsening table to %s...\n", path));
	for(x=0, U=cnodes, UE=U+nnodes; U<UE; U++){
		p = buf;
		b.i = U->parent;
		PBIT32(p, b.u);
		p += sizeof b.u;
		b.i = U->child;
		PBIT32(p, b.u);
		p += sizeof b.u;
		b.i = U->sibling;
		PBIT32(p, b.u);
		p += sizeof b.u;
		if((x = writefs(f, buf, sizeof buf)) < 0)
			break;
	}
	if(x < 0)
		logerr(va("exportctab: %s\n", error()));
	else
		logmsg("done exporting coarsening table.\n");
	freefs(f);
}

static Filefmt ff = {
	.name = "ctab",
	.load = load,
	.save = save,
};

Filefmt *
regctab(void)
{
	return &ff;
}
