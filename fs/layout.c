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
	x = -1;
	fs = emalloc(sizeof *fs);
	if(g == nil || path == nil){
		werrstr("invalid %s", g == nil ? "graph" : "path");
		goto end;
	}
	if((x = openfs(fs, path, OREAD)) < 0)
		goto end;
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
end:
	if(x > 0 && x < sizeof buf){
		werrstr("unexpected EOF: truncated input");
		x = -1;
	}
	freefs(fs);
	reqdraw(Reqrefresh);
	logmsg("importlayout: done\n");
	return x;
}

int
exportlayout(Graph *g, char *path)
{
	union { u32int u; float f; } u;
	int x;
	uchar buf[Recsz], *p;
	File *fs;
	RNode *r, *re;

	x = -1;
	fs = emalloc(sizeof *fs);
	if(g == nil || path == nil){
		werrstr("invalid %s", g == nil ? "graph" : "path");
		goto end;
	}
	if((x = openfs(fs, path, OWRITE)) < 0)
		goto end;
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
		if(writefs(fs, buf, sizeof buf) < 0)
			goto end;
	}
	x = 0;
end:
	freefs(fs);
	logmsg("exportlayout: done\n");
	return x;
}
