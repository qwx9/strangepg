#include "strpg.h"
#include <signal.h>
#define	HANDMADE_MATH_IMPLEMENTATION
//#define	HANDMADE_MATH_NO_SIMD
#include "lib/HandmadeMath.h"

#define GL_VIEWPORT 0x0BA2
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_TYPE_ERROR 0x824C
#define SG_GL_FUNCS_EXT	\
	_SG_XMACRO(glDebugMessageCallback,	void, (GLDEBUGPROC callback, const void * userParam)) \
	_SG_XMACRO(glGetTexImage,	void, (GLenum target, GLint level, GLenum format, GLenum type, void * pixels)) \
	_SG_XMACRO(glReadPixels,	void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)) \

//#define	NDEBUG
#define	SOKOL_IMPL
#define	SOKOL_GLCORE
#define	SOKOL_NO_ENTRY
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
/* include nuklear.h before the sokol_nuklear.h implementation */
#define	NK_INCLUDE_FIXED_TYPES
#define	NK_INCLUDE_STANDARD_IO
#define	NK_INCLUDE_DEFAULT_ALLOCATOR
#define	NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define	NK_INCLUDE_FONT_BAKING
#define	NK_INCLUDE_DEFAULT_FONT
#define	NK_INCLUDE_STANDARD_VARARGS
//#define	NK_INCLUDE_ZERO_COMMAND_MEMORY
#include "lib/nuklear.h"
#include "lib/sokol_nuklear.h"
#include "sokol.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

void	drawui(struct nk_context*);
void	event(const sapp_event*);

typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;

static HMM_Mat4 mvp;

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
	Color *c;

	c = emalloc(sizeof *c);
	c->col[0] = (v >> 24 & 0xff) / 255.f;
	c->col[1] = (v >> 16 & 0xff) / 255.f;
	c->col[2] = (v >> 8 & 0xff) / 255.f;
	c->col[3] = (v & 0xff) / 255.f;
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
	mvp = HMM_MulM4(proj, vw);
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

static inline void
renderedges(Params p)
{
	ioff n;
	double t;

	if((n = ndedges) < 1)
		return;
	if((debug & Debugperf) != 0)
		t = μsec();
	sg_update_buffer(render.edgebind.vertex_buffers[1], &(sg_range){
		.ptr = redges,
		.size = n * sizeof *redges,
	});
	if(render.caching){
		sg_begin_pass(&(sg_pass){
			.action = render.clearscreen,
			.attachments = render.offscrfb,
		});
		sg_apply_pipeline(render.offscredgepipe);
	}else{
		sg_begin_pass(&(sg_pass){
			.action = render.clearscreen,
			.swapchain = sglue_swapchain(),
		});
		sg_apply_pipeline(render.edgepipe);
	}
	sg_apply_bindings(&render.edgebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, render.nedgev, n);
	sg_end_pass();
	DPRINT(Debugperf, "renderedges: %.2f ms", (μsec() - t) / 1000);
}

static inline void
rendernodes(Params p)
{
	ioff n;
	double t;

	if((n = ndnodes) < 1)
		return;
	if((debug & Debugperf) != 0)
		t = μsec();
	sg_update_buffer(render.nodebind.vertex_buffers[1], &(sg_range){
		.ptr = rnodes,
		.size = n * sizeof *rnodes,
	});
	if(render.caching){
		sg_begin_pass(&(sg_pass){
			.action = render.overdraw,
			.attachments = render.offscrfb,
		});
		sg_apply_pipeline(render.offscrnodepipe);
	}else{
		sg_begin_pass(&(sg_pass){
			.action = render.overdraw,
			.swapchain = sglue_swapchain(),
		});
		sg_apply_pipeline(render.nodepipe);
	}
	sg_apply_bindings(&render.nodebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, render.nnodev, n);
	if(!render.caching)
		snk_render(view.w, view.h);
	sg_end_pass();
	DPRINT(Debugperf, "rendernodes: %.2f ms", (μsec() - t) / 1000);
}

static inline void
renderscreen(Params p)
{
	double t;

	if(!render.caching)
		return;
	if((debug & Debugperf) != 0)
		t = μsec();
	sg_begin_pass(&(sg_pass){
		.action = render.nothing,
		.swapchain = sglue_swapchain(),
	});
	sg_apply_pipeline(render.offscrpipe);
	sg_apply_bindings(&render.offscrbind);
	sg_draw(0, 4, 1);
	snk_render(view.w, view.h);
	sg_end_pass();
	DPRINT(Debugperf, "renderscreen: %.2f ms", (μsec() - t) / 1000);
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
	Params p;

	p = (Params){
		.mvp = mvp,
	};
	drawui(snk_new_frame());	/* rendered in the last pass, whichever it is */
	if(dylen(rnodes) > 0){
		renderedges(p);
		rendernodes(p);
		renderscreen(p);
	}else
		clearscreen();
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
		if(!stop){
			if((req & Reqrefresh) == 0)
				continue;
			reqs &= ~Reqrefresh;
			if(!redraw()){
				stop = 1;
				sapp_input_wait(true);
			}
			reqs |= Reqpickbuf;
		}else if((reqs & Reqshallowdraw) == 0)
			sapp_input_wait(true);
	}
}

void
frame(void)
{
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
		view.flags ^= VFdrawarrows;
		setnodeshape(view.flags & VFdrawarrows);
	}
	if(ndedges == 0 && ndnodes == 0)
		goto end;
	if((reqs & Reqpickbuf) != 0){
		reqs &= ~Reqpickbuf;
		render.caching = 1;
	}
	reqs &= ~Reqshallowdraw;
	flush();
	if(render.caching){
		render.stalepick = 1;
		render.caching = 0;
	}
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
