#include "strpg.h"
#include "fs.h"
#include "graph.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "threads.h"

/* FIXME: versioning + backwards compat? */

enum{
	Recsz = sizeof(ioff) + 3 * sizeof(float),
};

int
importlayout(char *path)
{
	union { u32int u; float f; ioff i; } u;
	int x, nr, ns;
	ioff id, idx;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r;

	/* FIXME: mt-safety */
	reqlayout(Lstop);
	if((fs = openfs(path, OREAD)) == nil)
		return -1;
	for(ns=nr=0;; nr++){
		if((x = readfs(fs, buf, sizeof buf)) < sizeof buf)
			break;
		p = buf;
		u.u = GBIT32(p);
		p += sizeof u.u;
		id = u.i;
		if((idx = getnodeidx(u.i)) < 0){
			DPRINT(Debugfs, "importlayout: skipping hidden node %d", u.i);
			continue;
		}
		if((r = rnodes + idx) >= rnodes + dylen(rnodes)){
			DPRINT(Debugfs, "importlayout: skipping out of bounds node %d", idx);
			continue;
		}
		u.u = GBIT32(p);
		p += sizeof u.u;
		r->pos[0] = u.f;
		u.u = GBIT32(p);
		p += sizeof u.u;
		r->pos[1] = u.f;
		u.u = GBIT32(p);
		r->pos[2] = u.f;
		DPRINT(Debugfs, "importlayout: %d %d %f,%f,%f",
			idx, id, r->pos[0], r->pos[1], r->pos[2]);
		ns++;
	}
	DPRINT(Debugfs, "importlayout: imported %d/%d positions", ns, nr);
	pushcmd("cmd(\"FGG138\")");
	flushcmd();
	if(x > 0 && x < sizeof buf){
		werrstr("unexpected EOF: truncated input");
		x = -1;
	}
	freefs(fs);
	logmsg("importlayout: done\n");
	return x;
}

/* FIXME: coarsen: same comment as above */
/* technically rnodes are shared with all graphs */
int
exportlayout(char *path)
{
	union { u32int u; float f; ioff i; } u;
	int x;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r, *re;
	Node *n;

	if((fs = openfs(path, OWRITE)) == nil)
		return -1;
	x = 0;
	for(n=nodes, r=rnodes, re=r+dylen(r); r<re; r++, n++){
		p = buf;
		u.i = n->id;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->pos[0];
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->pos[1];
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->pos[2];
		PBIT32(p, u.u);
		DPRINT(Debugfs, "exportlayout: %zd %d %f,%f,%f",
			r-rnodes, n->id, r->pos[0], r->pos[1], r->pos[2]);
		if((x = writefs(fs, buf, sizeof buf)) < 0)
			break;
	}
	freefs(fs);
	if((drawing.flags & DFnope) == 0)
		logmsg("exportlayout: done\n");
	return x;
}
