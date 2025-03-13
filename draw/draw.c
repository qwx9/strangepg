#include "strpg.h"
#include "fs.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"
#include "layout.h"

View view;
RNode *rnodes;
REdge *redges;
ioff *vedges;	/* FIXME: check if we could store id's in robj with SG_VERTEXFORMAT_UBYTE4N instead */
ssize ndnodes, ndedges;
Drawing drawing;
Channel *rendc;

static Channel *drawc;

static inline void
drawedge(REdge *r, RNode *u, RNode *v, int urev, int vrev)
{
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
	du = mulv(du, u->len * Nodesz / 2.0f);
	du = urev ? addv(p1, du) : subv(p1, du);
	dv = mulv(dv, v->len * Nodesz / 2.0f);
	dv = vrev ? subv(p2, dv) : addv(p2, dv);
	r->pos1[0] = du.x;
	r->pos1[1] = du.y;
	r->pos1[2] = du.z;
	r->pos2[0] = dv.x;
	r->pos2[1] = dv.y;
	r->pos2[2] = dv.z;
}

static intptr
drawedges(void)
{
	ioff id, eid, aid, x, *e, *ee;
	Node *n, *ne;
	RNode *u, *v;
	REdge *r;

	r = redges;
	x = dylen(redges);
	if(dylen(vedges) < x)
		dyresize(vedges, x);
	else if(dylen(vedges) > x)
		dyshrink(vedges, x);
	for(id=eid=0, u=rnodes, n=nodes, ne=n+dylen(n); n<ne; n++, u++, id++){
		for(e=edges+n->eoff, ee=e+n->nedges; e<ee; e++){
			x = *e;
			aid = x >> 2;
			if(id > aid || id == aid && (x & 1) == 1)
				continue;
			v = rnodes + (x >> 2);
			drawedge(r, u, v, x & 1, x & 2);
			r++;
			vedges[eid++] = e - edges;
		}
	}
	return r - redges;
}

void
resizenodes(void)
{
	double l, k, Δ, min, max;
	RNode *r, *re;

	if((drawing.flags & DFstalelen) == 0)
		return;
	min = Minsz;
	max = Maxsz;
	drawing.flags &= ~DFstalelen;
	if(drawing.length.min < 1.0)
		drawing.length.min = 1.0;
	if(drawing.length.max < drawing.length.min)
		drawing.length.max = drawing.length.min;
	if((Δ = drawing.length.max - drawing.length.min) < 1.0){
		Δ = 1.0;
		min = max = Nodesz;
	}
	k = Minsz * log(2) / Δ;
	for(r=rnodes, re=r+dylen(r); r<re; r++){
		l = r->len - drawing.length.min;
		if(l < 1.0)
			l = 1.0;
		r->len = min + (max - min) * ((exp(k * l) - 1) / (exp(k * Δ) - 1));
	}
}

static inline void
faceyourfears(RNode *ru, Node *u)
{
	float x, y, z, Δ, Δx, Δy, Δz;
	float θ, c, s, t;
	ioff *i, *ie;
	u32int e;
	RNode *rv;

	x = ru->pos[0];
	y = ru->pos[1];
	z = ru->pos[2];
	c = s = t = 0.0f;
	for(i=edges+u->eoff, ie=i+u->nedges; i<ie; i++){
		e = *i;
		rv = rnodes + (e >> 2);
		if(rv == ru)
			continue;
		if((e & 1) != 0){
			Δx = rv->pos[0] - x;
			Δy = rv->pos[1] - y;
			Δz = rv->pos[2] - z;
		}else{
			Δx = x - rv->pos[0];
			Δy = y - rv->pos[1];
			Δz = z - rv->pos[2];
		}
		Δ = sqrtf(Δx * Δx + Δy * Δy + Δz * Δz);
		c += Δx / Δ;
		//s += (Δy + Δz) / Δ;
		s += Δy / Δ;
		t += Δz / Δ;
	}
	/* FIXME: 3d edge rotation is wrong as well, we need two angles */
	θ = fmodf(atan2f(s, c), 2.0f*(float)PI);
	ru->dir[0] = cosf(θ);
	ru->dir[1] = sinf(θ);
	θ = sqrtf(c * c + s * s);
	θ = fmodf(atan2f(t, θ), 2.0f * (float)PI);
	//ru->dir[2] = cosf(θ);
	ru->dir[2] = 0.0f;
}

static intptr
drawnodes(void)
{
	Node *n, *e;
	RNode *r;

	for(r=rnodes, n=nodes, e=n+dylen(n); n<e; n++, r++)
		faceyourfears(r, n);
	return r - rnodes;
}

static int
drawworld(int go)
{
	int r;

	if(graph.layout == nil)
		return 0;
	if((r = graph.flags & GFdrawme) == 0 && !go){
		ndnodes = dylen(rnodes);
		ndedges = dylen(redges);
	}else{
		ndnodes = drawnodes();
		ndedges = drawedges();
	}
	return r;
}

/* FIXME: separate pipeline */
static void
drawui(void)
{
	return;
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
drawproc(void *)
{
	int r, go;

	go = 1;
	for(;;){
		if((r = recvul(drawc)) == 0)
			break;
		if(r & Reqstop){
			go = 0;
			continue;
		}else if(r & Reqredraw)
			go = 1;
		if(!(go = redraw(go)))
			reqdraw(Reqsleep);
		else
			reqdraw(Reqpickbuf);
	}
}

void
waitforit(void)
{
	/* wait until at least one file asks for a redraw */
	while((recvul(drawc) & Reqredraw) == 0)
		;
	newthread(drawproc, nil, nil, nil, "draw", mainstacksize);
	reqdraw(Reqredraw);
}

void
reqdraw(int r)
{
	static ulong df, rf;

	DPRINT(Debugdraw, "reqdraw %#x", r);
	switch(r){
	case Reqstop:
	case Reqshallowdraw:
	case Reqrefresh:
	case Reqredraw:
		wakedrawup();
		df |= r;
		if(nbsendul(drawc, df) != 0)
			df = 0;
		/* wet floor */
	case Reqresetdraw:
	case Reqresetui:
	case Reqfocus:
	case Reqpickbuf:
	case Reqshape:
	case Reqsleep:
		rf |= r;
		if(nbsendul(rendc, rf) != 0)
			rf = 0;
		break;
	default:
		warn("reqdraw: unknown request %#x\n", r);
	}
}

/* note: view screen dimensions are *not* necessarily set by the
 * end of all initialization */
void
initdrw(void)
{
	drawing.length = (Range){0.0f, 0.0f};
	drawing.xbound = drawing.length;
	drawing.ybound = drawing.length;
	drawing.zbound = drawing.length;
	drawing.nodesz = Nodesz;
	drawing.flags |= DFstalelen;
	drawing.fatness = Ptsz;
	settheme();
	initcol();
	/* FIXME: this chan implementation SUCKS */
	if((drawc = chancreate(sizeof(ulong), 1)) == nil
	|| (rendc = chancreate(sizeof(ulong), 1)) == nil)
		sysfatal("initdrw: chancreate");
}
