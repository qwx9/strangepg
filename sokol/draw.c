#include "strpg.h"
#include <signal.h>
#include "drw.h"
#include "threads.h"
#include "flextgl/flextGL.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_DEBUG
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_gp.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

GLFWwindow *glw;

enum{
	GLsamples = 4,
	GLdepth = 1,
};

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
	c->r = (v >> 16 & 0xff) / 255.f;
	c->g = (v >> 8 & 0xff) / 255.f;
	c->b = (v & 0xff) / 255.f;
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
startdrawclock(void)
{
}

void
stopdrawclock(void)
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
	int req, w, h;
	sg_pass sgp = {
		.swapchain = {
			.sample_count = GLsamples,
			.color_format = SG_PIXELFORMAT_RGBA8,
			.depth_format = GLdepth ? SG_PIXELFORMAT_DEPTH_STENCIL : SG_PIXELFORMAT_NONE,
			.gl = {
				/* assumption, GL framebuffer always 0 */
				.framebuffer = 0,
			},
		},
	};
	static Clk clk = {.lab = "flush"};

	/* FIXME: theming */
	/* FIXME: unimplemented functions above */
	/* FIXME: if we want the plan9 lilu dallas mooltithreading, this needs
	 *	a different design; on plan9 input and drawing are decoupled,
	 *	whereas here the same thread processes both; either figure out a
	 *	way to decouple them again (ticker, multithread glfw, locks) or
	 *	do it the old fashion way (poll for events, process, sleep); the
	 *	latter sucks because slow redraw throttles input, and we don't want
	 *	that */
	req = 0;
	while (!glfwWindowShouldClose(glw)) {
		if((req = nbrecvul(drawc)) != 0){
			/* Reqresetdraw for resizing: handled by callback */
			if((req & Reqresetui) != 0)
				resetui(1);
			w = view.dim.v.x;
			h = view.dim.v.y;
			if((req & Reqrefresh) != 0 && !norefresh || (req & Reqrender) != 0){
				if(!rerender(req & Reqrender))
					stopdrawclock();
			}
			if(req != Reqshallowdraw){
				sgp_begin(w, h);
				sgp_viewport(0, 0, w, h);
				sgp_set_blend_mode(SGP_BLENDMODE_BLEND);
				redraw();
				CLK1(clk);
				//sgp_set_blend_mode(SGP_BLENDMODE_NONE);
				sgp.swapchain.width = w;
				sgp.swapchain.height = h;
				sg_begin_pass(&sgp);
				sgp_flush();
				sgp_end();
				sg_end_pass();
				sg_commit();
				glfwSwapBuffers(glw);
				CLK1(clk);
		}
		reqdraw(Reqrefresh);	/* FIXME */
		glfwWaitEvents();		/* FIXME: kind of redundant with nbrecv */
	}
	quitdraw();
}

void
reqdraw(int r)
{
	static ulong f;

	f |= r;
	if(nbsendul(drawc, f) != 0){
		glfwPostEmptyEvent();
		f = 0;
	}
}

static void
glresize(GLFWwindow *, int w, int h)
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
	if(!glfwInit())
		sysfatal("glfwInit");
	glfwSetErrorCallback(glerr);
	/* swiped from glfw_glue in sokol-samples */
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, 0);
	glfwWindowHint(GLFW_SAMPLES, GLsamples == 1 ? 0 : GLsamples);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if((glw = glfwCreateWindow(view.dim.v.x, view.dim.v.y, "strpg", NULL, NULL)) == nil)
		sysfatal("glfwCreateWindow failed: %s", error());
	glfwMakeContextCurrent(glw);
	glfwSwapInterval(1);
	if(!flextInit(glw))
		sysfatal("flextGL: failed to initialize: %s", error());
	sg_setup(&(sg_desc){
		.environment = {
			.defaults = {
				.color_format = SG_PIXELFORMAT_RGBA8,
				.depth_format = GLdepth ? SG_PIXELFORMAT_DEPTH_STENCIL : SG_PIXELFORMAT_NONE,
				.sample_count = GLsamples,
			},
		},
		.logger.func = slog_func,
	});
	assert(sg_isvalid());
	sgp_setup(&(sgp_desc){0});
	assert(sgp_is_valid());
}

/* FIXME: error handling */
void
initsysdraw(void)
{
	view.dim.o = ZV;	/* FIXME: horrible */
	view.dim.v = Vec2(Vdefw, Vdefh);
	initgl();
	glfwSetFramebufferSizeCallback(glw, glresize);
	/* FIXME: the reason why this can't be 0 is because of the fucking
	 * input handling being synchronous */
	if((drawc = chancreate(sizeof(ulong), 64)) == nil)
		sysfatal("initsysdraw: chancreate");
	//atexit(quitdraw);
}
