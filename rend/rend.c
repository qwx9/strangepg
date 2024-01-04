#include "strpg.h"

static double
faceyourfears(Graph *g, Node *u)
{
	int n, sign;
	ssize *ip, *ie;
	double θ, dθ;
	Edge *e;
	Node *v;
	Vector Δ;

	/* FIXME:
	 * - fix perpendicular orientations
	 *	=> look at outgoing: then dignore things "behind" us
	 * - take into account orientation?
	 * - out/in
	 * - optionally don't show nodes at all, just have points
	 */

	θ = 0.;
	n = 0;
	/* face outgoing */
	if(dylen(u->out) > 0){
		for(ip=u->out, ie=ip+dylen(u->out); ip<ie; ip++){
			// FIXME: review this code with the diff; there's some
			//	redundancy and it might be unintentional
			//	it's buggy anyway
			if((e = getedge(g, *ip)) == nil)
				continue;
			v = getnode(g, e->v >> 1);	// FIXME: genericity: idx2n or sth
			assert(v != nil);
			if(u == v)
				continue;
			sign = (e->u & 1) != (e->v & 1) ? 1 : -1;
			Δ = subpt2(v->vrect.o, u->vrect.o);
			dθ = sign * atan2(Δ.y, Δ.x);
			// FIXME: needs to be scaled down
			/* weight inversely proportional to distance: prefer
			 * facing closer nodes
			 * also node weights */
			//d = sqrt(Δ.x * Δ.x + Δ.y * Δ.y);
			θ -= dθ;
			n++;
		}
	}
	/* face away from incoming */
	if(dylen(u->in) > 0){
		for(ip=u->in, ie=ip+dylen(u->in); ip<ie; ip++){
			if((e = getedge(g, *ip)) == nil)
				continue;
			v = getnode(g, e->v >> 1);
			assert(v != nil);
			if(u == v)
				continue;
			sign = (e->u & 1) != (e->v & 1) ? 1 : -1;
			Δ = subpt2(u->vrect.o, v->vrect.o);
			dθ = sign * atan2(Δ.y, Δ.x);
			/* FIXME: see comment above */
			//d = sqrt(Δ.x * Δ.x + Δ.y * Δ.y);
			θ -= dθ;
			n++;
		}
	}
	if(n > 0)
		θ /= n;
	θ -= PI / 4;	/* upper left to bottom right corner */
	while(θ >= 2*PI)
		θ -= 2*PI;
	while(θ < 0)
		θ += 2*PI;
	return θ;
}

static int
rendernode(Graph *g, Node *u)
{
	double θ, vx, vy, rx, ry, sx, sy;

	// FIXME: scale length to sequence length (relative)
	// FIXME: fix the definition of Vertex if this shit doesn't help us at
	//	all; adding o and v happens way too often, it's stupid and so is
	//	this code
	// ^- just have upper left corner, width, height, angle
	u->θ = faceyourfears(g, u);
	vx = Nodesz * (cos(u->θ) - sin(u->θ));	/* x´ = x cosβ - y sinβ */
	vy = Nodesz * (sin(u->θ) + cos(u->θ));	/* y´ = x sinβ + y cosβ */
	u->vrect.v = Vec2(vx, vy);
	u->q1 = u->vrect;
	u->q2 = u->vrect;
	θ = u->θ;
	if(fabs(u->θ) >= PI/2)
		θ += PI/2;
	else
		θ -= PI/2;
	rx = Nodesz/8 * (cos(θ) - sin(θ));
	ry = Nodesz/8 * (sin(θ) + cos(θ));
	if(fabs(u->θ) >= PI/2)
		θ -= PI;
	else
		θ += PI;
	sx = Nodesz/4 * (cos(θ) - sin(θ));
	sy = Nodesz/4 * (sin(θ) + cos(θ));
	u->q1.o.x += rx;
	u->q1.o.y += ry;
	u->q1.v.x = u->q1.v.x + sx;
	u->q1.v.y = u->q1.v.y + sy;
	u->q2.o.x -= rx;
	u->q2.o.y -= ry;
	u->q2.v.x = u->q2.v.x - sx;
	u->q2.v.y = u->q2.v.y - sy;
	u->shape.o = u->vrect.o;
	// FIXME: both dimensions are erroneous
	u->shape.v = Vec2(Nodesz+PI, Nodesz/4);	// FIXME: systematic error above
	return 0;
}

static int
rendershapes(Graph *g)
{
	ssize i;
	Node *u;
	Quad d;
	Vertex p;

	for(i=g->node0.next, d=ZQ; i>=0; i=u->next){
		yield();
		u = g->nodes + i;
		DPRINT(Debugrender, "render node %.1f,%.1f:%.1f,%.1f", u->vrect.o.x, u->vrect.o.y, u->vrect.v.x, u->vrect.v.y);
		u->vrect.v = ZV;
		rendernode(g, u);
		p = addpt2(u->vrect.o, u->vrect.v);
		if(p.x < d.o.x)
			d.o.x = p.x;
		else if(p.x > d.v.x)
			d.v.x = p.x;
		if(p.y < d.o.y)
			d.o.y = p.y;
		else if(p.y > d.v.y)
			d.v.y = p.y;
	}
	g->dim = d;
	return 0;
}

int
renderlayout(Graph *g)
{
	DPRINT(Debugrender, "renderlayout %#p", g);
	if(g->layout.ll == nil){
		werrstr("renderlayout: no layout");
		return -1;
	}
	if(dylen(g->nodes) < 1){
		werrstr("empty graph");
		return -1;
	}
	return rendershapes(g);
}

int
rerender(int force)
{
	int r;
	Graph *g;

	r = 0;
	for(g=graphs; g<graphs+dylen(graphs); g++)
		if(force || g->layout.tid >= 0){
			renderlayout(g);
			r = 1;
		}
	return r;
}

void
initrend(void)
{
}
