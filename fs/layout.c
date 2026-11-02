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

/* FIXME: if we want to importlayout then exportsvg, exportsvg needs to wait
 * until this is done, either here or in awk: maybe just defer exportsvg,
 * which means it shouldn't be an awk function */

static void
load(void *arg)
{
	union { u32int u; float f; ioff i; } b;
	int x, nr, ns;
	char *path;
	ioff id, idx;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r;

	path = arg;
	logmsg(va("importing layout from %s...\n", path));
	if((fs = openfs(path, OREAD)) == nil){
		logerr(va("importlayout %s: %s\n", path, error()));
		return;
	}
	for(ns=nr=0;; nr++){
		if((x = readfs(fs, buf, sizeof buf)) < sizeof buf)
			break;
		p = buf;
		b.u = GBIT32(p);
		p += sizeof b.u;
		id = b.i;
		if((idx = getactiveidx(id)) < 0){
			DPRINT(Debugfs, "importlayout: skipping hidden node %d", id);
			continue;
		}
		if(idx >= dylen(nodes)){
			DPRINT(Debugfs, "importlayout: skipping out of bounds node %d", idx);
			continue;
		}
		lockdraw();
		r = rnodes + idx;
		b.u = GBIT32(p);
		p += sizeof b.u;
		r->pos[0] = b.f;
		b.u = GBIT32(p);
		p += sizeof b.u;
		r->pos[1] = b.f;
		b.u = GBIT32(p);
		r->pos[2] = b.f;
		DPRINT(Debugfs, "importlayout: %d %d %f,%f,%f",
			idx, id, r->pos[0], r->pos[1], r->pos[2]);
		unlockdraw();
		ns++;
	}
	DPRINT(Debugfs, "importlayout: imported %d/%d positions", ns, nr);
	/* FIXME: push cmd anyway to avoid interrupting awk? */
	if(x < 0)
		logerr(va("importlayout: %s\n", error()));
	else if(x > 0 && x < sizeof buf)
		logerr(va("importlayout: unexpected EOF: truncated input\n"));
	else
		logmsg("done importing layout.\n");
	pushcmd("cmd(\"FGG138\")");
	flushcmd();
	freefs(fs);
}

/* technically nodes[] is shared with all graphs */
static void
save(void *arg)
{
	union { u32int u; float f; ioff i; } b;
	int x;
	char *path;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r, *re;
	Node *u;

	path = arg;
	if((fs = openfs(path, OWRITE)) == nil){
		logerr(va("exportlayout %s: %s\n", path, error()));
		return;
	}
	logmsg(va("exporting layout to %s...\n", path));
	x = 0;
	lockdraw();
	for(u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		p = buf;
		b.i = u->id;
		PBIT32(p, b.u);
		p += sizeof b.u;
		b.f = r->pos[0];
		PBIT32(p, b.u);
		p += sizeof b.u;
		b.f = r->pos[1];
		PBIT32(p, b.u);
		p += sizeof b.u;
		b.f = r->pos[2];
		PBIT32(p, b.u);
		DPRINT(Debugfs, "exportlayout: %zd %d %f,%f,%f",
			r-rnodes, u->id, r->pos[0], r->pos[1], r->pos[2]);
		if((x = writefs(fs, buf, sizeof buf)) < 0)
			break;
	}
	unlockdraw();
	if(x < 0)
		logerr(va("exportlayout: %s\n", error()));
	else if((drawing.flags & DFnope) == 0)
		logmsg("done exporting layout.\n");
	freefs(fs);
}

static Filefmt ff = {
	.name = "layout",
	.load = load,
	.save = save,
};

Filefmt *
reglayout(void)
{
	return &ff;
}
