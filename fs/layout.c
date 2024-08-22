#include "strpg.h"
#include "fs.h"
#include "graph.h"
#include "layout.h"
#include "threads.h"
#include "cmd.h"

int
importlayout(Graph *g, char *path)
{
	union { u32int u; float f; } u;
	int x;
	uchar buf[sizeof(ioff) + 2*3*sizeof(float)], *p;
	File *fs;
	RNode *r;

	stoplayout(g);
	x = -1;
	fs = emalloc(sizeof *fs);
	if(g == nil || path == nil){
		werrstr("invalid %s", g == nil ? "graph" : "path");
		goto end;
	}
	if((x = openfs(fs, path, OREAD)) < 0)
		goto end;
	for(r=rnodes;; r++){
		if(r >= rnodes + dylen(rnodes)){
			werrstr("more records than there are rnodes");
			x = -1;
			break;
		}
		if((x = readfs(fs, buf, sizeof buf)) < sizeof buf)
			break;
		p = buf;
		u.u = GBIT32(p);
		r->pos[0] = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		r->pos[1] = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		r->dir[0] = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		r->dir[1] = u.f;
	}
end:
	if(x > 0 && x < sizeof buf){
		werrstr("unexpected EOF: truncated input");
		x = -1;
	}
	freefs(fs);
	return x;
}

int
exportlayout(Graph *g, char *path)
{
	union { u32int u; float f; } u;
	int x;
	uchar buf[sizeof(u64int) + 2*3*sizeof(float)], *p;
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
		u.f = r->dir[0];
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = r->dir[1];
		PBIT32(p, u.u);
		if(writefs(fs, buf, sizeof buf) < 0)
			goto end;
	}
	x = 0;
end:
	freefs(fs);
	return x;
}
