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
#include "glsl/node.h"
#include "glsl/line.h"
#define	NK_INCLUDE_FIXED_TYPES
#define	NK_INCLUDE_STANDARD_IO
#define	NK_INCLUDE_DEFAULT_ALLOCATOR
#define	NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define	NK_INCLUDE_FONT_BAKING
#define	NK_INCLUDE_DEFAULT_FONT
#define	NK_INCLUDE_STANDARD_VARARGS
#include "lib/nuklear.h"
#include "lib/sokol_nuklear.h"
#include "lib/HandmadeMath.h"
#include "sokol.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

extern Channel *rendc, *ctlc;

void	drawui(struct nk_context*);
void	event(const sapp_event*);

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
	if(render.stalepick)
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
	proj = HMM_Perspective_RH_NO(view.fov, view.ar, 0.01f, 10000.0f);
	rot = HMM_MulM4(
		HMM_Rotate_RH(view.φ, HMM_V3(1.0f, 0.0f, 0.0f)),
		HMM_Rotate_RH(view.θ, HMM_V3(0.0f, 1.0f, 0.0f))
	),
	proj = HMM_MulM4(proj, rot);
	eye = HMM_V3(view.eye.x, view.eye.y, view.eye.z);
	center = HMM_V3(view.center.x, view.center.y, view.center.z);
	up = HMM_V3(view.up.x, view.up.y, view.up.z);
	vw = HMM_LookAt_RH(eye, center, up);
	render.cam.mvp = HMM_MulM4(proj, vw);
	render.moving = 1;
	render.stalepick = 1;
	reqdraw(Reqrefresh);
}

/* FIXME: pan/zoom in world coordinates; standardize this, make mvp global */
void
worldview(Vertex v)
{
	view.eye.x = v.x;
	view.eye.y = v.y;
	view.eye.z = v.z;
	if((drawing.flags & DF3d) == 0){
		view.center.x = v.x;
		view.center.y = v.y;
	}
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
	HMM_Vec4 vw;
	HMM_Mat4 rot;

	if((drawing.flags & DF3d) == 0){
		Δx /= view.w;
		Δy /= view.h;
		Δx *= 2 * view.Δeye.z * view.ar * view.tfov;
		Δy *= 2 * view.Δeye.z * view.tfov;
		view.center.x += Δx;
		view.center.y -= Δy;
		view.eye.x += Δx;
		view.eye.y -= Δy;
		updateview();
	}else{
		Δx *= HMM_DegToRad * 0.2f;
		Δy *= HMM_DegToRad * 0.2f;
		rot = HMM_MulM4(
			HMM_Rotate_RH(Δy, HMM_V3(view.right.x, view.right.y, view.right.z)),
			HMM_Rotate_RH(Δx, HMM_V3(view.up.x, view.up.y, view.up.z))
		);
		vw = HMM_MulM4V4(rot, HMM_V4(view.eye.x, view.eye.y, view.eye.z, 1.0f));
		view.eye = V(vw.X, vw.Y, vw.Z);
		vw = HMM_MulM4V4(rot, HMM_V4(view.up.x, view.up.y, view.up.z, 1.0f));
		view.up = V(vw.X, vw.Y, vw.Z);
		vw = HMM_MulM4V4(rot, HMM_V4(view.right.x, view.right.y, view.right.z, 1.0f));
		view.right = V(vw.X, vw.Y, vw.Z);
		updateview();
	}
}

void
zoomdraw(float Δ, float Δx, float Δy)
{
	Vertex v;

	if(drawing.flags & DF3d){
		v = mulv(view.Δeye, Δ);
		view.eye = subv(view.eye, v);
		updateview();
		return;
	}
	v = mulv(view.Δeye, Δ);
	view.eye = subv(view.eye, v);
	pandraw(Δx, Δy);
}

void
rotdraw(float Δx, float Δy)
{
	Δx *= HMM_DegToRad * 0.2f;
	Δy *= HMM_DegToRad * 0.2f;
	view.θ += Δx;
	view.φ += Δy;
	updateview();
}

static void
renderdirect(void)
{
	int n;
	static Clk clk = {.lab = "renderdirect"};

	CLK0(clk);
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.swapchain = sglue_swapchain(),
	});
	if((n = ndedges) >= 1){
		sg_apply_pipeline(render.edgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_edge_Vparam, &SG_RANGE(render.cam));
		sg_apply_uniforms(UB_edge_Fparam, &SG_RANGE(render.edgefs));
		sg_draw(0, render.nedgev * n, 1);
	}
	if((n = ndnodes) >= 1){
		sg_apply_pipeline(render.nodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_node_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, n);
	}
	if((n = ndlines) >= 1){
		sg_apply_pipeline(render.linepipe);
		sg_apply_bindings(&render.linebind);
		sg_apply_uniforms(UB_line_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nlinev * n, 1);
	}
	drawui(snk_new_frame());
	snk_render(view.w, view.h);
	sg_end_pass();
	CLK1(clk);
	render.nframes++;
}

static void
renderoffscreen(void)
{
	int n;
	static Clk clk = {.lab = "renderoffscreen"};

	CLK0(clk);
	sg_begin_pass(&(sg_pass){
		.action = render.clearpick,
		.attachments = render.pickimg,
	});
	if((n = ndedges) >= 1){
		sg_apply_pipeline(render.offscredgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_edge_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nedgev * n, 1);
	}
	if((n = ndnodes) >= 1){
		sg_apply_pipeline(render.offscrnodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_node_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, n);
	}
	sg_end_pass();
	CLK1(clk);
	render.noframes++;
}

static void
updatebuffers(int reqs)
{
	int n;

	if(dylen(rnodes) <= 0)
		return;
	if((n = ndlines) >= 1){
		sg_update_buffer(render.linebind.vertex_buffers[0], &(sg_range){
			.ptr = rlines,
			.size = n * sizeof *rlines,
		});
	}
	if((reqs & (Reqrefresh|Reqredraw|Reqpickbuf)) == 0)
		return;
	if((n = ndedges) >= 1){
		sg_update_buffer(render.edgebind.vertex_buffers[0], &(sg_range){
			.ptr = redges,
			.size = n * sizeof *redges,
		});
	}
	if((n = ndnodes) >= 1){
		sg_update_buffer(render.nodebind.vertex_buffers[1], &(sg_range){
			.ptr = rnodes,
			.size = n * sizeof *rnodes,
		});
	}
}

static void
resizebuf(void)
{
	ssize n;
	sg_buffer_desc d;

	n = dylen(redges);
	d = sg_query_buffer_desc(render.edgebind.vertex_buffers[0]);
	if(d.size / sizeof *redges != n){
		warn("redges: %d → %zd\n", d.size/sizeof *redges, n);
		sg_destroy_buffer(render.edgebind.vertex_buffers[0]);
		render.edgebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.size = dylen(redges) * sizeof *redges,
			.usage = SG_USAGE_STREAM,
		});
	}
	n = dylen(rnodes);
	d = sg_query_buffer_desc(render.nodebind.vertex_buffers[1]);
	if(d.size / sizeof *rnodes != n){
		warn("rnodes: %d → %zd\n", d.size/sizeof *rnodes, n);
		sg_destroy_buffer(render.nodebind.vertex_buffers[1]);
		render.nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *rnodes,
			.usage = SG_USAGE_STREAM,
		});
	}
}

/* FIXME: individual nodes */
void
updatenode(ioff id)	/* FIXME: single update */
{
	render.stalepick = 1;
	reqdraw(Reqredraw);
}

static void
clearscreen(void)
{
	drawui(snk_new_frame());
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.swapchain = sglue_swapchain(),
	});
	snk_render(view.w, view.h);
	sg_end_pass();
}

static void
renderscene(void)
{
	if(dylen(rnodes) <= 0){
		clearscreen();
		return;
	}
	if(render.caching)
		renderoffscreen();
	renderdirect();
	sg_commit();
}

void
wakedrawup(void)
{
	/* FIXME: yes, but there are no events pending */
	sapp_wakethefup();
}

/* FIXME: promote to global code */
void
frame(void)
{
	int f, r;
	vlong t;
	static vlong t0;

	r = 0;
	while((f = nbrecvul(rendc)) != 0)
		r |= f;
	if(r != 0){
		if(r & Reqthaw){
			resizebuf();
			drawing.flags &= ~DFnorend;
		}else if(r & Reqfreeze){	/* FIXME: race */
			sendul(ctlc, DFnorend);
			drawing.flags |= DFnorend;
		}
		if(r & Reqresetdraw){
			resize();
			updateview();
		}
		if(r & Reqresetui){
			resetui();
			updateview();
		}
		if(r & Reqfocus)
			focusobj();
		if(r & Reqshape){
			drawing.flags ^= DFdrawarrows;
			setnodeshape(drawing.flags & DFdrawarrows);
		}
		if(r & Reqpickbuf && (t = μsec()) - t0 >= (1000000 / 30)){	/* FIXME: tune */
			render.stalepick = 1;
			render.caching = 1;
			t0 = t;
		}
		if(r & (Reqstop|Reqsleep))
			sapp_input_wait(true);
	}
	if(drawing.flags & DFnorend)
		return;
	updatebuffers(r);
	renderscene();
	render.caching = 0;
	if(graph.flags & GFdrawme)
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
		sgx_enable_debug_log();
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
evloop(void)
{
	sapp_run(&(sapp_desc){
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.event_cb = event,
		.sample_count = drawing.flags & DFmsaa ? 4 : 1,
		.enable_clipboard = true,
		.window_title = "strangepg",
		.icon.sokol_default = true,
		.logger.func = slog_func,
		.high_dpi = drawing.flags & DFhidpi,
	});
}
