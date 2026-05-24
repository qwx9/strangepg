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
loadct(void *arg)
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
		logerr(va("loadctab %s: coarsening table already initialized\n", path));
		return;
	}
	logmsg(va("loadct: %s\n", path));
	if(initcoarse() < 0){
		logerr(va("loadctab %s: %s\n", path, error()));
		return;
	}
	DPRINT(Debugfs, "loadctab %s", path);
	if((f = openfs(path, OREAD)) == nil){
		logerr(va("loadctab %s: %s\n", path, error()));
		return;
	}
	r = -1;
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
	r = 0;
	if(U != UE){
		werrstr("too few records in ctab, %zd < %d", U-cnodes, nnodes);
		r = -1;
	}else if(x < 0)
		r = -1;
	graph.flags |= GFctarmed;
	pushcmd("cmd(\"HGI234\")");	/* signal needed to continue, error or no */
	flushcmd();
	if(r < 0)
		logerr(va("loadct %s: %s, line %d\n", path, error(), f->nr));
	else
		logmsg("loadct: done\n");
	freefs(f);
}

int
exportct(char *path)
{
	int r, x;
	union { u32int u; s32int i; } b;
	uchar buf[Recsz], *p;
	File *f;
	CNode *U, *UE;

	logmsg(va("exportct: %s\n", path));
	if((graph.flags & GFctarmed) == 0){	/* FIXME: build it? */
		werrstr("no tree yet");
		return -1;
	}
	if((f = openfs(path, OWRITE)) == nil)
		return -1;
	r = -1;
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
	r = x < 0 ? -1 : 0;
	freefs(f);
	logmsg("exportct: done\n");
	return r;
}

int
exportdot(char *path)
{
	int r;
	ioff i;
	char buf[1024], *p;
	File *f;
	CNode *U, *UE;

	if((graph.flags & GFctarmed) == 0){	/* FIXME: build it? */
		werrstr("no tree yet");
		return -1;
	}
	if((f = openfs(path, OWRITE)) == nil)
		return -1;
	r = -1;
	p = seprint(buf, buf + sizeof buf, "digraph {\n");
	if(writefs(f, buf, p - buf) < 0)
		goto end;
	for(i=0, U=cnodes, UE=U+nnodes; U<UE; U++, i++){
		if(U->parent != -1){
			p = seprint(buf, buf + sizeof buf, "\t%d -> %d\n", U->parent, i);
			if(writefs(f, buf, p - buf) < 0)
				goto end;
		}
	}
	p = seprint(buf, buf + sizeof buf, "}\n");
	r = writefs(f, buf, p - buf);
end:
	freefs(f);
	return r;
}

static Filefmt ff = {
	.name = "ctab",
	.load = loadct,
};

Filefmt *
regctab(void)
{
	return &ff;
}
