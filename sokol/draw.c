#include "strpg.h"
#include <signal.h>
#include <pthread.h>
#include "lib/chan.h"
#include "drw.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_gp.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

static pthread_t wth;
static chan_t *drawc;
static GLFWwindow *win;
static sg_pass_action pass_action; // FIXME

int
drawline(Quad q, double w, int emph)
{
	sgp_push_transform();
	sgp_translate(q.o.x, q.o.y);
	sgp_set_color(1.0f, 0.0f, 0.0f, 1.0f);
	sgp_draw_filled_rect(0, 0, q.v.x - q.o.x, q.v.y - q.o.y);
	sgp_reset_color();
	sgp_pop_transform();
	return 0;
}

int
drawbezier(Quad q, double w)
{
	return 0;
}

int
drawquad(Quad q, double θ, int)
{
	// FIXME: weight?
	sgp_push_transform();
	sgp_translate(q.o.x, q.o.y);
	sgp_set_color(1.0f, 0.0f, 0.0f, 1.0f);
	sgp_rotate(θ);
	sgp_draw_filled_rect(0, 0, q.v.x - q.o.x, q.v.y - q.o.y);
	sgp_reset_color();
	sgp_pop_transform();
	return 0;
}

int
drawquad2(Quad q1, Quad q2, double θ, int, int)
{
	// FIXME
	sgp_push_transform();
	sgp_translate(q1.o.x, q1.o.y);
	sgp_set_color(1.0f, 1.0f, 0.0f, 1.0f);
	sgp_rotate(θ);
	sgp_draw_filled_rect(0, 0, q2.v.x - q1.v.x, q2.v.y - q1.v.y);
	sgp_reset_color();
	sgp_pop_transform();
	return 0;
}

void
flushdraw(void)
{
	int w, h;

	glfwGetFramebufferSize(win, &w, &h);	// just use win?
	sg_begin_default_pass(&pass_action, w, h);
	sgp_flush();
	sgp_end();
	sg_end_pass();
	sg_commit();
	glfwSwapBuffers(win);
	glfwPollEvents();	// FIXME: blocking?
}

void
cleardraw(void)
{
	// FIXME: more stuff in plan9 version
	// draw background
	sgp_set_color(.0f, .0f, .0f, 1.0f);
	sgp_clear();
	sgp_reset_color();
}

void
reqdraw(int r)
{
	dprint("reqdraw %d\n", r);	/* FIXME: non-blocking */
	if(chan_send(drawc, &r) < 0)
		sysfatal("reqdraw: chan_send");
}

static void
mkwin(void)
{
	if((win = glfwCreateWindow(view.dim.v.x, view.dim.v.y, "strpg", NULL, NULL)) == nil)
		sysfatal("glfwCreateWindow");
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);	// FIXME: what is this?
}

static void
initgl(void)
{
	int w, h;

	if(!glfwInit())
		sysfatal("glfwInit");
	mkwin();
	sg_desc desc = { .logger.func = slog_func };
	sg_setup(&desc);
	assert(sg_isvalid());
	sgp_desc sgpdesc = {0};
	sgp_setup(&sgpdesc);
	assert(sgp_is_valid());
}

static void
resetdraw(void)
{
	// FIXME: ?
}

static void*
drawproc(void *ch)
{
	int w, h, c;
	float g;
	chan_t *chans[1];
	int *chanv[nelem(chans)];

	initgl();
	chans[0] = ch;
	chanv[0] = &c;	/* FIXME: what the fuck this api */

	/* default pass action, clear to red */
/*
	pass_action = (sg_pass_action){
	    .colors[0] = {
	    	.load_action = SG_LOADACTION_CLEAR,
	    	.clear_value = { 1.0f, 0.0f, 0.0f, 1.0f },
	    },
	};
*/

	while(!glfwWindowShouldClose(win)){
		glfwGetFramebufferSize(win, &w, &h);
		sgp_begin(w, h);
		// FIXME: blocking recv
		switch(chan_select(chans, nelem(chans), chanv, NULL, 0, NULL)){
		case 0:
			switch(c){
			case Reqresetdraw: resetdraw(); resetui(1); redraw(); break;
			case Reqredraw: redraw(); break;
			case Reqshallowdraw: shallowdraw(); break;
			}
			break;
		case -1:
			redraw();	// FIXME
			break;
		}
		flushdraw();
	}
	sgp_shutdown();
	sg_shutdown();
	glfwTerminate();
	pthread_exit(NULL);
}

static void
quitdraw(void)
{
	pthread_kill(wth, SIGTERM);
}

int
initsysdraw(void)
{
	int r;

	view.dim.o = ZV;
	view.dim.v = Vec2(Vdefw, Vdefh);
	atexit(quitdraw);
	if((drawc = chan_init(sizeof(int))) == nil)
		sysfatal("initsysdraw: chancreate");
	if((r = pthread_create(&wth, NULL, drawproc, drawc)) != 0)
		sysfatal("initsysdraw: pthread_create: %d", r);
	return 0;
}
