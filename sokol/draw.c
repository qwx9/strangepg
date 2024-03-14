#include "strpg.h"
#include <signal.h>
#include "drw.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_DEBUG
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_gp.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

static chan_t *drawc;
static GLFWwindow *glw;

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

int
drawline(Quad q, double w, int emph, int)
{
	Color *c;

	sgp_push_transform();
	sgp_translate(view.center.x - view.pan.x, view.center.y - view.pan.y);
	sgp_scale(view.zoom, view.zoom);
	c = &(haxx0rz ? theme2 : theme1)[Cedge];
	sgp_set_color(c->r / 255.f, c->g / 255.f, c->b / 255.f, 0.8f);
	sgp_draw_line(q.o.x, q.o.y, q.o.x+q.v.x, q.o.y+q.v.y);
	sgp_reset_color();
	sgp_pop_transform();
	sgp_reset_color();
	return 0;
}

int
drawbezier(Quad q, double w, int)
{
	// FIXME
	//q = centerscalequad(q);
	return drawline(q, w, 0, -1);
}

// FIXME: update or remove
int
drawquad(Quad q, double θ, int, int)
{
	q = centerscalequad(q);	// FIXME: part of transform?
	sgp_push_transform();
	//sgp_scale(view.zoom, view.zoom);
	sgp_translate(view.pan.x, view.pan.y);
	sgp_set_color(0.1f, 0.7f, 0.7f, 0.7f);
	sgp_draw_filled_rect(q.o.x, q.o.y, q.v.x - q.o.x, q.v.y - q.o.y);
	sgp_reset_color();
	sgp_pop_transform();
	return 0;
}

int
drawlabel(Quad, Quad, Quad, vlong)
{
	return 0;
}

/* FIXME: we need untransformed shapes, where plan9 needs the opposite; fix this */
/* FIXME: q1 and q2 are meant for a series of lines and are invalid rectangles */
int
drawquad2(Quad q1, Quad q2, Quad q, double θ, int sh, int nc, int)
{
	Color *c;

	if(sh)
		return 0;
	sgp_push_transform();
	sgp_translate(view.center.x - view.pan.x, view.center.y - view.pan.y);
	sgp_scale(view.zoom, view.zoom);

	c = palette + nc % nelem(palette);
	sgp_set_color(c->r / 255.f, c->g / 255.f, c->b / 255.f, 0.8f);
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

void
cleardraw(void)
{
	Color *c;

	// FIXME: more stuff in plan9 version
	c = &(haxx0rz ? theme2 : theme1)[Cbg];
	sgp_set_color(c->r / 255.f, c->g / 255.f, c->b / 255.f, 1.0f);
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
reqdraw(int r)
{
	if(chan_send_int32(drawc, r) < 0)
		sysfatal("chan_send");	// FIXME: errno/errstr
	glfwPostEmptyEvent();
}

typedef struct Mouse Mouse;
struct Mouse{
	double x;
	double y;
	double ox;
	double oy;
	int b;
};
static Mouse ms;

static void
mouseposev(GLFWwindow *, double x, double y)
{
	double Δx, Δy;

	if(ms.b != 0){
		Δx = x - ms.ox;
		Δy = y - ms.oy;
		if(mouseevent(Vec2(x, y), Vec2(Δx, Δy), ms.b) < 0)
			warn("mouseev\n");
		ms.ox = x;
		ms.oy = y;
	}
	ms.x = x;
	ms.x = y;
}

static void
mousebutev(GLFWwindow *w, int b, int action, int mod)
{
	int m;
	double x, y;

	switch(b){
	case GLFW_MOUSE_BUTTON_LEFT: m = Mlmb; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: m = Mmmb; break;
	case GLFW_MOUSE_BUTTON_RIGHT: m = Mrmb; break;
	default: warn("mouseev: unhandled mouse button %d\n", b); return;
	}
	if(action == GLFW_RELEASE)
		ms.b = ms.b & ~m;
	else
		ms.b = ms.b | m;
	glfwGetCursorPos(w, &x, &y);
	ms.ox = x;
	ms.oy = y;
	mouseposev(w, x, y);
}

/* could be split this way as well as per usual, but not sure it has
 * any advantage here besides not dicking with modifiers */
/*
static void
ukeyev(GLFWwindow *, unsigned int u)
{
	Rune r;

	warn("ukeyev %d\n", u);
	switch(u){
	case 'q': glfwSetWindowShouldClose(glw, GLFW_TRUE); return;
	case '-': r = '-'; break;
	case '+': r = '+'; break;
	case 'R': r = 'R'; break;
	case 'a': r = 'a'; break;
	default: return;
	}
	if(keyevent(r) < 0)
		warn("invalid input key %d\n", r);
}
*/

static void
keyev(GLFWwindow *, int k, int, int action, int mod)
{
	Rune r;

	if(action == GLFW_RELEASE)
		return;
	warn("keyev %d\n", k);
	r = 0;
	switch(k){
	case GLFW_KEY_DELETE: /* wet floor */
	case GLFW_KEY_Q: glfwSetWindowShouldClose(glw, GLFW_TRUE); return;
	case GLFW_KEY_UP: r = KBup; break;
	case GLFW_KEY_DOWN: r = KBdown; break;
	case GLFW_KEY_LEFT: r = KBleft; break;
	case GLFW_KEY_RIGHT: r = KBright; break;
	case GLFW_KEY_ESCAPE: r = KBescape; break;
	case GLFW_KEY_MINUS: r = '-'; break;
	case GLFW_KEY_EQUAL: if(mod & GLFW_MOD_SHIFT) r = '+'; break;
	case GLFW_KEY_R: r = 'R'; break;
	case GLFW_KEY_A: if(mod & GLFW_MOD_SHIFT) r ='a'; break;
	default: return;
	}
	if(keyevent(r) < 0)
		warn("keyev: invalid input key %d\n", k);
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

/* FIXME: error handling + error strings */
int
initsysdraw(void)
{
	view.dim.o = ZV;	/* FIXME: horrible */
	view.dim.v = Vec2(Vdefw, Vdefh);
	initgl();
	//glfwSetCharCallback(glw, ukeyev);
	glfwSetKeyCallback(glw, keyev);
	glfwSetMouseButtonCallback(glw, mousebutev);
	glfwSetCursorPosCallback(glw, mouseposev);
	glfwSetFramebufferSizeCallback(glw, resizeev);
	if((drawc = chan_init(166*sizeof(int))) == nil)
		sysfatal("initsysdraw: chancreate");
	//atexit(quitdraw);
	return 0;
}

void
initsysui(void)
{
}
