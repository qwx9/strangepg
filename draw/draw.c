#include "strpg.h"
#include "graph.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "layout.h"

View view;
RNode *rnodes;
REdge *redges;
ssize ndnodes, ndedges;
Drawing drawing;
extern Channel *drawc;

static inline void
drawedge(REdge *r, RNode *u, RNode *v, int urev, int vrev)
{
	float m;
	Vertex p1, p2, du, dv;

	du.x = u->dir[0];
	du.y = u->dir[1];
	du.z = u->dir[2];
	p1.x = u->pos[0];
	p1.y = u->pos[1];
	p1.z = u->pos[2];
	dv.x = v->dir[0];
	dv.y = v->dir[1];
	dv.z = v->dir[2];
	p2.x = v->pos[0];
	p2.y = v->pos[1];
	p2.z = v->pos[2];
	m = sqrtf(du.x * du.x + du.y * du.y) + 0.000001;
	du = divv(du, m);
	du = mulv(du, u->len * Nodesz / 2.0f);
	du = urev ? addv(p1, du) : subv(p1, du);
	m = sqrtf(dv.x * dv.x + dv.y * dv.y) + 0.000001;
	dv = divv(dv, m);
	dv = mulv(dv, v->len * Nodesz / 2.0f);
	dv = vrev ? subv(p2, dv) : addv(p2, dv);
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos1[2] = du.z;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
	r->pos2[2] = dv.z;
}

static REdge *
drawedges(REdge *r, RNode *rn)
{
	ioff x, *e, *ee;
	Node *n, *ne;
	RNode *u, *v;

	for(u=rn, n=nodes, ne=n+dylen(n); n<ne; n++, u++){
		for(e=edges+n->eoff, ee=e+n->nedges-n->nin; e<ee; e++, r++){
			x = *e;
			v = rnodes + (x >> 2);
			drawedge(r, u, v, x & 1, x & 2);
		}
	}
	return r;
}

void
resizenodes(void)
{
	double l, Δ, max;
	RNode *r, *re;

	if((drawing.flags & DFstalelen) == 0)
		return;
	drawing.flags &= ~DFstalelen;
	if(drawing.length.min <= Minsz)
		drawing.length.min = Minsz;
	if(drawing.length.max <= drawing.length.min)
			drawing.length.max = Minsz;
	Δ = MAX(Minsz, drawing.length.max - drawing.length.min);
	max = Maxsz;
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		l = r->len;
		if(l == 0.0)
			l = Minsz;
		r->len = (max - (max - Minsz) * exp(-l / Δ));
	}
	drawing.length.max = (max - (max - Minsz) * exp(-drawing.length.max / Δ));
}

static inline void
faceyourfears(RNode *ru, Node *u)
{
	float x, y, Δ, Δx, Δy;
	float θ, c, s;
	ioff *i, *ie;
	u32int e;
	RNode *rv;

	x = ru->pos[0];
	y = ru->pos[1];
	c = s = 0.0;
	for(i=edges+u->eoff, ie=i+u->nedges-u->nin; i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		if((e & 1) != 0){
			Δx = rv->pos[0] - x;
			Δy = rv->pos[1] - y;
		}else{
			Δx = x - rv->pos[0];
			Δy = y - rv->pos[1];
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy);
		c += Δx / Δ;
		s += Δy / Δ;
	}
	for(ie+=u->nin; i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		if((e & 1) != 0){
			Δx = rv->pos[0] - x;
			Δy = rv->pos[1] - y;
		}else{
			Δx = x - rv->pos[0];
			Δy = y - rv->pos[1];
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy);
		c += Δx / Δ;
		s += Δy / Δ;
	}
	θ = fmodf(atan2f(s, c), 2.0f*(float)PI);
	ru->dir[0] = cosf(θ);
	ru->dir[1] = sinf(θ);
	ru->dir[2] = 0.0f;
}

static RNode *
drawnodes(RNode *r)
{
	Node *n, *e;

	for(n=nodes, e=n+dylen(n); n<e; n++, r++)
		faceyourfears(r, n);
	return r;
}

static int
drawworld(int go)
{
	int r;
	RNode *rn, *rne;
	REdge *re;
	Graph *g;

	if(drawing.flags & DFstalelen)
		resizenodes();
	r = 0;
	rn = rnodes;
	re = redges;
	for(g=graphs; g<graphs+dylen(graphs); g++){
		if(g->type <= FFdead || g->layout == nil)
			continue;
		if((g->flags & GFdrawme) != 0)
			r++;
		else if(!go){
			rn += g->nnodes;
			re += g->nedges;
			continue;
		}
		rne = drawnodes(rn);
		re = drawedges(re, rn);
		rn = rne;
	}
	ndnodes = rn - rnodes;
	ndedges = re - redges;
	return r;
}

/* FIXME: separate pipeline */
static void
drawui(void)
{
	if(ndedges < 1 || selbox[0].pos2[0] - selbox[0].pos1[0] == 0.0f)
		return;
	assert(ndedges + nelem(selbox) <= dylen(redges));	/* realloc would race */
	memcpy(redges + ndedges, selbox, sizeof selbox);
	ndedges += nelem(selbox);
}

int
redraw(int go)
{
	static Clk clk = {.lab = "redraw"};

	CLK0(clk);
	go = drawworld(go);
	drawui();
	CLK1(clk);
	return go;
}

static void
ticker(void *)
{
	for(;;){
		sleep(10);
		sendul(drawc, Reqrefresh);
	}
}

void
noloop(void)
{
	ulong u;

	newthread(ticker, nil, nil, nil, "ticker", mainstacksize); 
	while((u = recvul(drawc)) > 0 && u != Reqredraw)
		;
	if(u <= 0)
		return;
	reqlayout(Lstart);
	while((u = recvul(drawc)) > 0){
		switch(u){
		case Reqredraw:
		case Reqrefresh: pushcmd("exportlayout(\"%s\")", drawing.layfile); flushcmd(); break;
		}
	}
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	drawing.length = (Range){9999999.0f, 0.0f};
	drawing.xbound = drawing.length;
	drawing.ybound = drawing.length;
	drawing.zbound = drawing.length;
	drawing.nodesz = Nodesz;
	drawing.flags |= DFstalelen;
	drawing.fatness = Ptsz;
	settheme();
	initcol();
	initsysdraw();
}
