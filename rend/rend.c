#include "strpg.h"

static double
faceyourfears(Graph *g, Node *u)
{
	int n, sign;
	usize *ip, *ie, us, them;
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
	us = vecindexof(&g->nodes, u);
	/* face outgoing */
	if(u->out.len > 0){
		for(ip=u->out.buf, ie=ip+u->out.len; ip<ie; ip++){
			e = vecp(&g->edges, *ip);
			// FIXME: helpers
			sign = (e->from & 1) != (e->to & 1) ? 1 : -1;
			/* FIXME: see this is why we don't want global edge list */
			them = e->to >> 1;
			assert(us == e->from >> 1);
			if(them == us){
				them = e->from >> 1;
				if(them == us){
					n--;
					continue;
				}
			}
			v = vecp(&g->nodes, them);
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
	if(u->in.len > 0){
		for(ip=u->in.buf, ie=ip+u->in.len; ip<ie; ip++){
			e = vecp(&g->edges, *ip);
			sign = (e->from & 1) != (e->to & 1) ? 1 : -1;
			them = e->from >> 1;
			assert(us == e->to >> 1);
			if(them == us){
				them = e->to >> 1;
				if(them == us){
					n--;
					continue;
				}
			}
			v = vecp(&g->nodes, them);
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
	return 0;
}

static int
rendershapes(Graph *g)
{
	Node *u, *ue;
	Quad d;
	Vertex p;

	for(d=ZQ, u=g->nodes.buf, ue=u+g->nodes.len; u<ue; u++){
		dprint("render node %s\n", shitprint('q', &u->vrect));
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
render(Graph *g)
{
	if(g->nodes.len < 1){
		werrstr("empty graph");
		return -1;
	}
	return rendershapes(g);
}

void
initrend(void)
{
}
