#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "fs.h"
#include "cmd.h"
#include "drw.h"
#include "view.h"
#include "layout.h"
#include "threads.h"

/* FIXME: quit() in a script will interrupt any waiting here */
/* ... and our current fix doesn't work because there's TWO different
 * defer lists... maybe we should just stipulate WHEN individual commands
 * may be executed, ie. in what state the main program has to be, or maybe
 * we just can spawn a thread and wait until the condition is complete,
 * even if that sucks */

static void
save(void *arg)
{
	int rc;
	char *path, buf[1024], tail[256], *p;
	float c, s, θ, dim[4] = {9999.0f, 9999.0f, 0.0f, 0.0f};
	u32int col;
	RNode *r, *re;
	REdge *e, *ee;
	Node *u;
	File *fs;

	while((graph.flags & (GFarmed | GFdrawme | GFlayme)) != GFarmed){
		DPRINT(Debugfs, "exportsvg: waiting");
		sleep(1);
	}
	path = arg;
	if((fs = openfs(path, OWRITE)) == nil){
		logerr(va("exportsvg %s: %s\n", path, error()));
		return;
	}
	logmsg(va("exporting svg render to %s...\n", path));
	rc = -1;
	lockdraw();
	/* FIXME: rotate according to view */
	dim[0] = view.bb.tl.x;
	dim[1] = -view.bb.br.y;
	dim[2] = view.bb.br.x;
	dim[3] = -view.bb.tl.y;
	p = seprint(buf, buf+sizeof buf,
		"<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
		"width=\"%d\" height=\"%d\" viewBox=\"%f %f %f %f\">\n",
		view.w, view.h, dim[0], dim[1], dim[2]-dim[0], dim[3]-dim[1]);
	if(writefs(fs, buf, p - buf) < 0)
		goto end;
	seprint(tail, tail+sizeof tail,
		"x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\"/>\n",
		-0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		drawing.nodesz, drawing.fatness);
	for(e=redges, ee=e+dylen(e); e<ee; e++){
		p = seprint(buf, buf+sizeof buf,
			"<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" "
			"style=\"stroke:#cccccc;stroke-opacity:0.3;stroke-width:0.05\"/>\n",
			e->pos1[0], -e->pos1[1], e->pos2[0], -e->pos2[1]);
		if(writefs(fs, buf, p - buf) < 0)
			goto end;
	}
	for(u=nodes, r=rnodes, re=r+dylen(r); r<re; r++, u++){
		if(u->nedges == 0)
			continue;
		s = 2.0f * (r->dir[3] * r->dir[2] + r->dir[0] * r->dir[1]);
		c = 1.0f - 2.0f * (r->dir[1] * r->dir[1] + r->dir[2] * r->dir[2]);
		θ = HMM_ToDeg(atan2f(s, c));
		col = ((int)(r->col[0] / r->col[3] * 255.f) & 0xff) << 16;
		col |= ((int)(r->col[1] / r->col[3] * 255.f) & 0xff) << 8;
		col |= ((int)(r->col[2] / r->col[3] * 255.f) & 0xff);
		p = seprint(buf, buf+sizeof buf,
			"<rect transform=\"scale(1 -1) translate(%f %f) rotate(%f) scale(%f 1)\" "
			"fill=\"#%06x\" fill-opacity=\"%f\" ",
			r->pos[0], r->pos[1], θ, r->len, col, r->col[3]);
		p = strecpy(p, buf+sizeof buf, tail);
		if(writefs(fs, buf, p - buf) < 0)
			goto end;
	}
	unlockdraw();
	p = seprint(buf, buf+sizeof buf, "</svg>\n");
	if(writefs(fs, buf, p - buf) < 0)
		goto end;
	rc = 0;
end:
	if(rc < 0)
		logerr(va("exportsvg: %s\n", error()));
	else
		logmsg("done exporting svg.\n");
	freefs(fs);
}

static Filefmt ff = {
	.name = "svg",
	.save = save,
};

Filefmt *
regsvg(void)
{
	return &ff;
}
