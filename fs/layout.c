#include "strpg.h"
#include "fs.h"
#include "graph.h"
#include "drw.h"
#include "layout.h"
#include "threads.h"
#include "cmd.h"

enum{
	Recsz = 3 * sizeof(float),
};

int
importlayout(Graph *g, char *path)
{
	union { u32int u; float f; } u;
	int x;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r;

	if((g->flags & GFdrawme) != 0)
		reqlayout(g, Lstop);
	if((fs = openfs(path, OREAD)) == nil)
		return -1;
	for(r=rnodes;; r++){
		if((x = readfs(fs, buf, sizeof buf)) < sizeof buf)
			break;
		if(r >= rnodes + dylen(rnodes)){
			werrstr("more records than there are rnodes");
			x = -1;
			break;
		}
		p = buf;
		u.u = GBIT32(p);
		r->pos[0] = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		r->pos[1] = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		r->pos[2] = u.f;
	}
	if(x > 0 && x < sizeof buf){
		werrstr("unexpected EOF: truncated input");
		x = -1;
	}
	freefs(fs);
	reqdraw(Reqredraw);
	logmsg("importlayout: done\n");
	return x;
}

/* technically rnodes are shared with all graphs */
int
exportlayout(Graph *, char *path)
{
	union { u32int u; float f; } u;
	int x;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r, *re;

	if((fs = openfs(path, OWRITE)) == nil)
		return -1;
	x = 0;
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		p = buf;
		u.f = r->pos[0];
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->pos[1];
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->pos[2];
		PBIT32(p, u.u);
		if((x = writefs(fs, buf, sizeof buf)) < 0)
			break;
	}
	freefs(fs);
	logmsg("exportlayout: done\n");
	return x;
}
