#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "fs.h"
#include "cmd.h"
#include "drw.h"
#include "layout.h"
#include "threads.h"

int
exportsvg(char *path)
{
	char buf[1024], tail[256], *p;
	float c, s, l, w, θ, dim[4] = {9999.0f, 9999.0f, 0.0f, 0.0f};
	u32int col;
	RNode *r, *re;
	REdge *e, *ee;
	File *fs;

	/* FIXME: at least one frame must've been drawn and rendered, maybe
	 * read a counter or sth; should this also be on its own thread?
	 * through cmd maybe? */
	/* FIXME: add sleep() to awk (previously system) and remove this counter */
	while(drawing.frames == 0)
		lsleep(1000);
	if((fs = openfs(path, OWRITE)) == nil)
		return -1;
	/* FIXME: doesn't take into account transformations */
	w = MAX(drawing.nodesz, drawing.fatness);
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		//l = r->len + w;
		if(r->pos[0] - w < dim[0])
			dim[0] = r->pos[0] - w;
		else if(r->pos[0] + w > dim[2])
			dim[2] = r->pos[0] + w;
		if(r->pos[1] - w < dim[1])
			dim[1] = r->pos[1] - w;
		else if(r->pos[1] + w > dim[3])
			dim[3] = r->pos[1] + w;
	}
	p = seprint(buf, buf+sizeof buf,
		"<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
		"width=\"%d\" height=\"%d\" viewBox=\"%f %f %f %f\">\n",
		1920, 1080, dim[0], dim[1], dim[2]-dim[0], dim[3]-dim[1]);
	if(writefs(fs, buf, p - buf) < 0){
		logerr(va("exportsvg: %s\n", error()));
		goto end;
	}
	seprint(tail, tail+sizeof tail,
		"x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\"/>\n",
		-0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		drawing.nodesz, drawing.fatness);
	for(r=rnodes, re=r+dylen(r); r<re; r++){
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
		if(writefs(fs, buf, p - buf) < 0){
			logerr(va("exportsvg: %s\n", error()));
			goto end;
		}
	}
	for(e=redges, ee=e+dylen(e); e<ee; e++){
		p = seprint(buf, buf+sizeof buf,
			"<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" "
			"style=\"stroke:#cccccc;stroke-opacity:0.3;stroke-width:0.1\"/>\n",
			e->pos1[0], -e->pos1[1], e->pos2[0], -e->pos2[1]);
		if(writefs(fs, buf, p - buf) < 0){
			logerr(va("exportsvg: %s\n", error()));
			goto end;
		}
	}
	p = seprint(buf, buf+sizeof buf, "</svg>\n");
	if(writefs(fs, buf, p - buf) < 0){
		logerr(va("exportsvg: %s\n", error()));
		goto end;
	}
	logmsg("exportsvg: done\n");
end:
	freefs(fs);
	return 0;
}
