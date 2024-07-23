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
	int r;
	uchar buf[sizeof(ioff) + 2*3*sizeof(float)], *p;
	ioff id;
	Vertex v;
	File *fs;
	Node *n;

	stoplayout(g);
	r = -1;
	fs = emalloc(sizeof *fs);
	if(g == nil || path == nil){
		werrstr("invalid %s", g == nil ? "graph" : "path");
		goto end;
	}
	if((r = openfs(fs, path, OREAD)) < 0)
		goto end;
	for(;;){
		if((r = readfs(fs, buf, sizeof buf)) < sizeof buf)
			break;
		p = buf;
		id = GBIT64(p);
		p += sizeof id;
		if(id < 0 || id >= dylen(g->nodes)){
			werrstr("invalid id");
			r = -1;
			break;
		}
		n = g->nodes + id;
		u.u = GBIT32(p);
		v.x = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		v.y = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		v.z = u.f;
		p += sizeof u.u;
		n->pos = v;
		u.u = GBIT32(p);
		v.x = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		v.y = u.f;
		p += sizeof u.u;
		u.u = GBIT32(p);
		v.z = u.f;
		n->dir = v;
	}
end:
	if(r > 0 && r < sizeof buf){
		werrstr("unexpected EOF: truncated input");
		r = -1;
	}
	freefs(fs);
	return r;
}

int
exportlayout(Graph *g, char *path)
{
	union { u32int u; float f; } u;
	int r;
	ioff i, ie;
	uchar buf[sizeof(u64int) + 2*3*sizeof(float)], *p;
	File *fs;
	Node *n;

	r = -1;
	fs = emalloc(sizeof *fs);
	if(g == nil || path == nil){
		werrstr("invalid %s", g == nil ? "graph" : "path");
		goto end;
	}
	if((r = openfs(fs, path, OWRITE)) < 0)
		goto end;
	for(i=0, ie=dylen(g->nodes); i<ie; i++){
		p = buf;
		n = g->nodes + i;
		PBIT64(p, i);
		p += sizeof i;
		u.f = n->pos.x;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = n->pos.y;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = n->pos.z;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = n->dir.x;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = n->dir.y;
		PBIT32(p, u.u);
		p += sizeof u.u;
		u.f = n->dir.z;
		PBIT32(p, u.u);
		if(writefs(fs, buf, sizeof buf) < 0){
			r = -1;
			break;
		}
	}
end:
	freefs(fs);
	return r;
}
