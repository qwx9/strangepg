#include "strpg.h"
#include "fs.h"
#include "graph.h"
#include "layout.h"
#include "cmd.h"
#include "drw.h"

typedef struct Comp Comp;
struct Comp{
	Range x;
	Range y;
};

static int
readhdr(File *f)
{
	int n;

	if(readline(f) < 0)
		return -1;
	n = 0;
	while(nextfield(f) != nil)
		n++;
	if(n == 0){
		werrstr("invalid header");
		return -1;
	}
	return f->foff;
}

int
importodgilayout(char *path)
{
	int rc, nc;
	uint *cis, ci;
	ioff idx;
	voff id;
	char *s, *p;
	double rf, x, y, c, t;
	vlong nr, start;
	File *f;
	RNode *r;
	Comp *cs, *cp, ctop = {{0.0}, {0.0}};

	t = μsec();
	reqlayout(Lstop);
	if((f = openfs(path, OREAD)) == nil)
		return -1;
	if((start = readhdr(f)) < 0)
		return -1;
	nr = 1;
	rc = -1;
	nc = 0;
	cs = nil;
	cis = nil;
	/* doesn't assume ordering outside of pairs */
	while(readline(f) != nil){
		nr++;
		if((s = nextfield(f)) == nil){
			werrstr("line %lld: short read", nr);
			goto error;
		}
		id = strtoull(s, &p, 10);
		if(*p != 0 || id < 0){
			werrstr("line %lld: invalid index: %d", nr, id);
			goto error;
		}
		if((idx = getnodeidx(id >> 1)) < 0){
			DPRINT(Debugfs, "importodgilayout: skipping collapsed node %d", id >> 1);
			continue;
		}
		r = rnodes + idx;
		if((s = nextfield(f)) == nil){
			werrstr("line %lld: short read", nr);
			goto error;
		}
		c = strtod(s, &p);
		if(*p != 0){
			werrstr("line %lld: invalid x: %f", nr, c);
			goto error;
		}
		if((id & 1) == 0)
			x = c;
		else{
			x = r->pos[0];
			x += (c - x) * 0.5;
		}
		if((s = nextfield(f)) == nil){
			werrstr("line %lld: short read", nr);
			goto error;
		}
		c = strtod(s, &p);
		if(*p != 0){
			werrstr("line %lld: invalid y: %f", nr, c);
			goto error;
		}
		if((id & 1) == 0)
			y = -c;
		else{
			y = r->pos[1];
			y += (-c - y) * 0.5;
		}
		r->pos[0] = x;
		r->pos[1] = y;
		if((id & 1) == 0)
			continue;
		r->pos[2] = (double)(id >> 1) / nnodes;
		if((s = nextfield(f)) != nil){
			ci = strtoul(s, &p, 10);
			if(*p != 0){
				werrstr("line %lld: invalid component: %d", nr, ci);
				goto error;
			}
		}else
			ci = 0;
		dypushat(cis, idx, ci);
		if(dylen(cs) <= ci){
			dygrow(cs, ci);
			cp = cs + ci;
			cp->x.min = cp->x.max = x;
			cp->y.min = cp->y.max = y;
		}else{
			cp = cs + ci;
			if(cp->x.min > x)
				cp->x.min = x;
			if(cp->x.max < x)
				cp->x.max = x;
			if(cp->y.min > y)
				cp->y.min = y;
			if(cp->y.max < y)
				cp->y.max = y;
		}
		if(nc > 0){
			if(ctop.x.min > x)
				ctop.x.min = x;
			if(ctop.x.max < x)
				ctop.x.max = x;
			if(ctop.y.min > y)
				ctop.y.min = y;
			if(ctop.y.max < y)
				ctop.y.max = y;
		}else
			ctop = *cp;
		nc++;
	}
	DPRINT(Debugfs, "importodgilayout: imported %d/%d positions", nc, nr/2);
	/* have to scale everything by constant factor,
	 * won't work otherwise since we don't move adjacent nodes */
	rf = MAX(ctop.x.max - ctop.x.min, ctop.y.max - ctop.y.min);
	rf *= 8;
	if((rf = dylen(rnodes) / rf) > 1.0)
		rf = 1.0;
	seekfs(f, start);
	nr = 1;
	while(readline(f) != nil){	/* FIXME: could cache instead */
		nr++;
		if((s = nextfield(f)) == nil){
			werrstr("line %lld: short read", nr);
			goto error;
		}
		id = strtoull(s, &p, 10);
		if(*p != 0){
			werrstr("line %lld: invalid index", nr);
			goto error;
		}
		if(id & 1)
			continue;
		id >>= 1;
		if((idx = getnodeidx(id)) < 0)
			continue;
		r = rnodes + idx;
		cp = cs + cis[idx];
		/* FIXME: yes, but components are still bunched together,
		 * especially small ones, into lumps */
		r->pos[0] -= cp->x.min + (cp->x.max - cp->x.min) * 0.5;
		r->pos[1] -= cp->y.min + (cp->y.max - cp->y.min) * 0.5;
		r->pos[0] *= rf;
		r->pos[1] *= rf;
	}
	rc = 0;
error:
	dyfree(cs);
	dyfree(cis);
	freefs(f);
	pushcmd("cmd(\"FGG138\")");
	flushcmd();
	t = (μsec() - t) / 1000.0;
	logmsg(va("importodgilayout: done in %.2f ms\n", t));
	return rc;
}
