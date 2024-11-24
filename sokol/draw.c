#include "strpg.h"

#define GL_VIEWPORT 0x0BA2
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_TYPE_ERROR 0x824C
#define SG_GL_FUNCS_EXT	\
	_SG_XMACRO(glDebugMessageCallback,	void, (GLDEBUGPROC callback, const void * userParam)) \
	_SG_XMACRO(glGetTexImage,	void, (GLenum target, GLint level, GLenum format, GLenum type, void * pixels)) \
	_SG_XMACRO(glReadPixels,	void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)) \

#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
#include "glsl/edge.h"
#include "glsl/edgeidx.h"
#include "glsl/node.h"
#include "glsl/nodeidx.h"
#include "glsl/scr.h"
#include "lib/nuklear.h"
#include "lib/sokol_nuklear.h"
#include "lib/HandmadeMath.h"
#include "sokol.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

void	drawui(struct nk_context*);
void	event(const sapp_event*);

typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;

static Channel *drawc;
static int reqs;

void
setcolor(float *col, u32int v)
{
	Color *c;

	c = color(v);
	memcpy(col, c->col, sizeof c->col);
}

Color *
newcolor(u32int v)
{
	float a;
	Color *c;

	a = (v & 0xff) / 255.f;
	c = emalloc(sizeof *c);
	c->col[0] = a * ((v >> 24 & 0xff) / 255.f);
	c->col[1] = a * ((v >> 16 & 0xff) / 255.f);
	c->col[2] = a * ((v >> 8 & 0xff) / 255.f);
	c->col[3] = a;
	return c;
}

void
endmove(void)
{
	render.moving = 0;
	reqdraw(Reqpickbuf);
}

/* FIXME: promote following functions to portable code; alias HMM types? */
void
updateview(void)
{
	HMM_Vec3 eye, center, up;
	HMM_Mat4 vw, rot, proj;

	view.Δeye = subv(view.eye, view.center);
	view.ar = (float)view.w / view.h;
	proj = HMM_Perspective_RH_NO(view.fov, view.ar, 0.01f, 100000.0f);
	rot = HMM_MulM4(
			HMM_Rotate_RH(view.θ, HMM_V3(0.0f, 1.0f, 0.0f)),
            HMM_Rotate_RH(view.φ, HMM_V3(1.0f, 0.0f, 0.0f))
            );
	proj = HMM_MulM4(proj, rot);
	eye = HMM_V3(view.eye.x, view.eye.y, view.eye.z);
	center = HMM_V3(view.center.x, view.center.y, view.center.z);
	up = HMM_V3(view.up.x, view.up.y, view.up.z);
	vw = HMM_LookAt_RH(eye, center, up);
	render.cam.mvp = HMM_MulM4(proj, vw);
	render.moving = 1;
}

/* FIXME: pan/zoom in world coordinates; standardize this, make mvp global */
void
worldview(Vertex v)
{
	view.center.x = view.eye.x = v.x;
	view.center.y = view.eye.y = v.y;
	view.eye.z = v.z;
	updateview();
}

void
worldpandraw(float x, float y)
{
	updateview();
}

void
pandraw(float Δx, float Δy)
{
	Δx /= view.w;
	Δy /= view.h;
	Δx *= 2 * view.Δeye.z * view.ar * view.tfov;
	Δy *= 2 * view.Δeye.z * view.tfov;
	view.eye.x += Δx;
	view.eye.y -= Δy;
	view.center.x += Δx;
	view.center.y -= Δy;
	updateview();
}

/* FIXME: geared towards 2D, where it doesn't make sense to zoom past
 * the canvas */
void
zoomdraw(float Δ, float Δx, float Δy)
{
	Vertex v;

	v = mulv(view.Δeye, Δ);
	view.eye = subv(view.eye, v);
	pandraw(Δx, Δy);
}

void
rotdraw(Vertex v)
{
	view.θ += v.x;
	view.φ += v.y;
	updateview();
}

static void
renderdirect(void)
{
	int n;
	double t;

	t = debug & Debugperf ? μsec() : 0;
	drawui(snk_new_frame());
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.swapchain = sglue_swapchain(),
	});
	if((n = ndedges) >= 1){
		sg_update_buffer(render.edgebind.vertex_buffers[0], &(sg_range){
			.ptr = redges,
			.size = n * sizeof *redges,
		});
		sg_apply_pipeline(render.edgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_Vparam, &SG_RANGE(render.cam));
		sg_apply_uniforms(UB_Fparam, &SG_RANGE(render.edgefs));
		sg_draw(0, render.nedgev * n, 1);
	}
	if((n = ndnodes) >= 1){
		sg_update_buffer(render.nodebind.vertex_buffers[1], &(sg_range){
			.ptr = rnodes,
			.size = n * sizeof *rnodes,
		});
		sg_apply_pipeline(render.nodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, n);
	}
	snk_render(view.w, view.h);
	sg_end_pass();
	DPRINT(Debugperf, "renderdirect: %.2f ms", (μsec() - t) / 1000);
}

static void
renderoffscreen(void)
{
	int n;

	double t;

	t = debug & Debugperf ? μsec() : 0;
	drawui(snk_new_frame());
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.attachments = render.offscrfb,
	});
	if((n = ndedges) >= 1){
		sg_update_buffer(render.edgebind.vertex_buffers[0], &(sg_range){
			.ptr = redges,
			.size = n * sizeof *redges,
		});
		sg_apply_pipeline(render.offscredgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_Vparam, &SG_RANGE(render.cam));
		sg_apply_uniforms(UB_Fparam, &SG_RANGE(render.edgefs));
		sg_draw(0, render.nedgev * n, 1);
	}
	if((n = ndnodes) >= 1){
		sg_update_buffer(render.nodebind.vertex_buffers[1], &(sg_range){
			.ptr = rnodes,
			.size = n * sizeof *rnodes,
		});
		sg_apply_pipeline(render.offscrnodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, n);
	}
	sg_end_pass();
	/* FIXME: only works on opengl/x86; fix it first */
	goto skip;
	sg_begin_pass(&(sg_pass){
		.action = render.nothing,
		.swapchain = sglue_swapchain(),
	});
	sg_apply_pipeline(render.offscrpipe);
	sg_apply_bindings(&render.offscrbind);
	sg_draw(0, 4, 1);
	snk_render(view.w, view.h);
	sg_end_pass();
skip:
	DPRINT(Debugperf, "renderoffscreen: %.2f ms", (μsec() - t) / 1000);
}

static void
clearscreen(void)
{
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.swapchain = sglue_swapchain(),
	});
	snk_render(view.w, view.h);
	sg_end_pass();
}

static void
flush(void)
{
	if(dylen(rnodes) <= 0)
		clearscreen();
	else if(!render.caching)
		renderdirect();
	else
		renderoffscreen();
	sg_commit();
}

void
reqdraw(int r)
{
	static ulong f;

	f |= r;
	reqs |= r;
	if(nbsendul(drawc, f) != 0)
		f = 0;
}

static void
drawproc(void *)
{
	int req, stop;

	stop = 0;
	for(;;){
		if((req = recvul(drawc)) == 0)
			break;
		if((req & Reqredraw) != 0){
			reqs &= ~Reqredraw;
			stop = 0;
			sapp_wakethefup();
		}else if((reqs & Reqshallowdraw) != 0)
			sapp_wakethefup();
		if(stop = !redraw(stop)){
			sapp_input_wait(true);
		}else
			reqs |= Reqpickbuf;
	}
}

void
frame(void)
{
	vlong t;
	static int frm;
	static vlong t0;

	if((reqs & Reqresetdraw) != 0){
		reqs &= ~Reqresetdraw;
		resize();
	}
	if((reqs & Reqresetui) != 0){
		reqs &= ~Reqresetui;
		resetui();
		updateview();
	}
	if((reqs & Reqfocus) != 0){
		reqs &= ~Reqfocus;
		focusobj();
	}
	if((reqs & Reqshape) != 0){
		reqs &= ~Reqshape;
		drawing.flags ^= DFdrawarrows;
		setnodeshape(drawing.flags & DFdrawarrows);
	}
	if(ndedges == 0 && ndnodes == 0)
		goto end;
	if((reqs & Reqpickbuf) != 0){
		reqs &= ~Reqpickbuf;
		t = μsec();
		if(t - t0 >= 1000000 && frm > 2){
			render.stalepick = 1;
			render.caching = 1;
			t0 = t;
			frm = 0;
		}else
			frm++;
	}else
		frm++;
	reqs &= ~Reqshallowdraw;	/* clear here to let us get some input events first */
	flush();
	if(render.caching)
		render.caching = 0;
end:
	reqdraw(Reqrefresh);
}

static void
init(void)
{
	sg_setup(&(sg_desc){
		.environment = sglue_environment(),
		.logger.func = slog_func,
	});
	assert(sg_isvalid());
	if(debug & Debugdraw)
		sg_enable_debug_log();
	initgl();
	initnk();
	resetui();
	updateview();
}

static void
cleanup(void)
{
	snk_shutdown();
	sg_shutdown();
}

void
initsysdraw(void)
{
	if((drawc = chancreate(sizeof(ulong), 32)) == nil)
		sysfatal("initsysdraw: chancreate");
}

void
evloop(void)
{
	/* wait until at least one file asks for a redraw */
	while(recvul(drawc) != Reqredraw)
		;
	newthread(drawproc, nil, nil, nil, "draw", mainstacksize);
	reqdraw(Reqredraw);
	sapp_run(&(sapp_desc){
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.event_cb = event,
		.width = Vdefw,
		.height = Vdefh,
		.enable_clipboard = true,
		.window_title = "strangepg",
		.ios_keyboard_resizes_canvas = true,
		.icon.sokol_default = true,
		.logger.func = slog_func,
	});
}
