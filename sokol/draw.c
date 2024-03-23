#include "strpg.h"
#include <signal.h>
#include "drw.h"
#include "threads.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_DEBUG
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_gp.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

GLFWwindow *glw;

struct Color{
	u32int col;
	float r;
	float g;
	float b;
};

static Channel *drawc;

Color *
newcolor(u32int v)
{
	Color *c;

	c = emalloc(sizeof *c);
	c->col = v;
	c->r = (v >> 16) / 255.f;
	c->g = (v >> 8) / 255.f;
	c->b = v / 255.f;
	return c;
}

u32int
col2int(Color *c)
{
	return c->col;
}

int
scrobj(Vertex)
{
	return -1;
}

void
showobj(Obj *)
{
}

void
flushdraw(void)
{
}

void
startdrawclock(void)
{
}

int
drawlabel(Node *, Quad, Quad, Quad, vlong, Color *)
{
	return 0;
}

int
drawline(Quad q, double w, int emph, int idx, Color *c)
{
	USED(idx);
	sgp_push_transform();
	sgp_translate(view.center.x - view.pan.x, view.center.y - view.pan.y);
	sgp_scale(view.zoom, view.zoom);
	sgp_set_color(c->r, c->g, c->b, 0.8f);
	sgp_draw_line(q.o.x, q.o.y, q.o.x+q.v.x, q.o.y+q.v.y);
	sgp_reset_color();
	sgp_pop_transform();
	sgp_reset_color();
	return 0;
}

int
drawbezier(Quad q, double w, int idx, Color *c)
{
	USED(idx);
	// FIXME
	//q = centerscalequad(q);
	return drawline(q, w, 0, idx, c);
}

/* FIXME: we need untransformed shapes, where plan9 needs the opposite; fix this */
/* FIXME: q1 and q2 are meant for a series of lines and are invalid rectangles */
int
drawquad(Quad q1, Quad q2, Quad q, double θ, int idx, Color *c)
{
	sgp_push_transform();
	sgp_translate(view.center.x - view.pan.x, view.center.y - view.pan.y);
	sgp_scale(view.zoom, view.zoom);
	sgp_set_color(c->r, c->g, c->b, 0.8f);
	// FIXME: systematic "error"s in rend.c
	sgp_rotate_at(θ+PI/4, q.o.x, q.o.y);
	q.o = subpt2(q.o, Vec2(0, Nodesz/8));	// FIXME: layer violation
	sgp_draw_filled_rect(q.o.x, q.o.y, q.v.x, q.v.y);
	sgp_reset_color();
	sgp_pop_transform();
	if(debug){
		sgp_set_color(0.0f, 1.0f, 0.0f, 0.8f);
		sgp_draw_filled_rect(q1.o.x, q1.o.y, 1, 1);
		sgp_set_color(1.0f, 1.0f, 0.0f, 0.8f);
		sgp_draw_filled_rect(q1.o.x+q1.v.x, q1.o.y+q1.v.y, 1, 1);
		q1 = centerscalequad(q1);
		q2 = centerscalequad(q2);
		sgp_set_color(1.0f, 1.0f, 1.0f, 1.0f);
		sgp_draw_line(q1.o.x, q1.o.y, q1.v.x, q1.v.y);
		sgp_set_color(0.8f, 0.8f, 0.8f, 1.0f);
		sgp_draw_line(q2.o.x, q2.o.y, q2.v.x, q2.v.y);
		sgp_reset_color();
	}
	return 0;
}

// FIXME: missing stuff in plan9/draw which should be generic anyway
void
cleardraw(void)
{
	Color *c;

	c = color(theme[Cbg]);
	sgp_set_color(c->r, c->g, c->b, 1.0f);
	sgp_clear();
	sgp_reset_color();
}

static void
quitdraw(void)
{
	sgp_shutdown();
	sg_shutdown();
	glfwTerminate();
}

void
evloop(void)
{
	int r;
	sg_pass_action pass_action = {0};

	glfwMakeContextCurrent(glw);
	glfwSwapInterval(1);
	while(!glfwWindowShouldClose(glw)){
		sgp_begin(view.dim.v.x, view.dim.v.y);
		sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
		redraw();
		sgp_set_blend_mode(SGP_BLENDMODE_NONE);
		sg_begin_default_pass(&pass_action, view.dim.v.x, view.dim.v.y);
		sgp_flush();
		sgp_end();
		sg_end_pass();
		sg_commit();
		glfwSwapBuffers(glw);
		glfwWaitEvents();
		while(chan_size(drawc) > 0){
			r = -1;
			if(chan_recv_int32(drawc, &r) < 0)
				sysfatal("chan_recv");	// FIXME: errno/errstr
			switch(r){
			case Reqresetdraw: /* wet floor */
			case Reqresetui: resetui(1); /* wet floor */
			case Reqredraw: /* wet floor */
			case Reqshallowdraw: break;
			case Reqrefresh: rerender(1); break;
			default: warn("reqdraw: unknown req %d", r); return;
			}
		}
		if(drawstep && rerender(0))
			glfwPostEmptyEvent();
	}
	quitdraw();
}

void
reqdraw(int32_t r)
{
	if(sendul(drawc, r) < 0)
		sysfatal("sendul: %s", error());
	glfwPostEmptyEvent();
}

static void
resizeev(GLFWwindow *glw, int w, int h)
{
	view.dim.v = Vec2(w, h);
	resetui(1);
}

static void
glerr(int err, const char *desc)
{
	warn("glerr %d: %s\n", err, desc);
}

/* FIXME: error handling + error strings */
static void
initgl(void)
{
	int w, h;
	sg_desc desc = {
		.logger.func = slog_func,
	};
	sgp_desc sgpdesc = { 0 };

	if(!glfwInit())
		sysfatal("glfwInit");
	glfwSetErrorCallback(glerr);
	if((glw = glfwCreateWindow(view.dim.v.x, view.dim.v.y, "strpg", NULL, NULL)) == nil)
		sysfatal("glfwCreateWindow");
	glfwMakeContextCurrent(glw);
	sg_setup(&desc);
	assert(sg_isvalid());
	sgp_setup(&sgpdesc);
	assert(sgp_is_valid());
}

// FIXME: use Channel and similar typedefs
// FIXME: use error strings in sokol/linux code

/* FIXME: error handling */
void
initsysdraw(void)
{
	view.dim.o = ZV;	/* FIXME: horrible */
	view.dim.v = Vec2(Vdefw, Vdefh);
	initgl();
	glfwSetFramebufferSizeCallback(glw, resizeev);
	if((drawc = chancreate(sizeof(int), 166)) == nil)	// FIXME: 166??
		sysfatal("initsysdraw: chancreate");
	//atexit(quitdraw);
}
