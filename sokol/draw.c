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
#include "view.h"
#include "ui.h"
#include "threads.h"

extern Channel *rendc;

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
	drawstate &= ~DSmoving;
	if(drawstate & DSstalepick)
		reqdraw(Reqpickbuf);
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
	snk_render(sapp_width(), sapp_height());
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

	DPRINT(Debugrender, "updatebuffers l=%d e=%d n=%d", ndlines, ndedges, ndnodes);
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

	n = MAX(1, dylen(redges));
	d = sg_query_buffer_desc(render.edgebind.vertex_buffers[0]);
	if(d.size / sizeof *redges != n){
		DPRINT(Debugrender, "redge bindings: resize %d → %zd", d.size/sizeof *redges, n);
		sg_destroy_buffer(render.edgebind.vertex_buffers[0]);
		render.edgebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.size = dylen(redges) * sizeof *redges,
			.usage = SG_USAGE_STREAM,
		});
	}
	n = MAX(1, dylen(rnodes));
	d = sg_query_buffer_desc(render.nodebind.vertex_buffers[1]);
	if(d.size / sizeof *rnodes != n){
		DPRINT(Debugrender, "rnode bindings: resize %d → %zd", d.size/sizeof *rnodes, n);
		sg_destroy_buffer(render.nodebind.vertex_buffers[1]);
		render.nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *rnodes,
			.usage = SG_USAGE_STREAM,
		});
	}
}

/* FIXME: individual nodes */
void
updatenode(ioff id)
{
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
	snk_render(sapp_width(), sapp_height());
	sg_end_pass();
}

static void
renderscene(void)
{
	if(dylen(rnodes) <= 0){
		clearscreen();
		return;
	}
	render.cam.mvp = view.mvp;
	if(render.caching)
		renderoffscreen();
	renderdirect();
	sg_commit();
}

void
wakedrawup(void)
{
	DPRINT(Debugdraw, "wake up the fup");
    sapp_input_wait(false);
}

/* FIXME: promote to global code */
void
frame(void)
{
	int snooze, r, f;
	vlong t;
	static vlong t0;

	r = snooze = 0;
	while((f = nbrecvul(rendc)) != 0)
		r |= f;
	DPRINT(Debugdraw, "frame: %#x", r);
	if(r != 0){
		if(r & Reqsleep)
			snooze = 1;
		if(r & Reqresetdraw){
			resize();
			updateview();
		}
		if(r & Reqresetview){
			resetview();
			updateview();
		}
		if(r & Reqfocus)
			focusobj();
		if(r & Reqshape){
			drawing.flags ^= DFdrawarrows;
			setnodeshape(drawing.flags & DFdrawarrows);
		}
	}else
		snooze = 1;
	switch(drawing.flags & (DFfreeze | DFnorend)){
	case DFfreeze:
		drawing.flags |= DFnorend;
		/* wet floor */
	case DFfreeze | DFnorend:
		/* FIXME: maybe have an alt render function for eg. resizes? */
		return;
	case DFnorend:
		resizebuf();
		drawing.flags &= ~DFnorend;
		break;
	}
	/* FIXME: set time here? or in offscreen render */
	if((t = μsec()) - t0 >= (1000000 / 30)){	/* FIXME: tune */
		drawstate |= DSstalepick;
		render.caching = 1;
		t0 = t;
	}
	updatebuffers(r);
	renderscene();
	render.caching = 0;
	if(graph.flags & GFdrawme)
		reqdraw(Reqrefresh);
	if(snooze){
		drawing.flags |= DFnorend;	/* for coarsening */
		sapp_input_wait(true);
	}
}

static void
init(void)
{
	sg_setup(&(sg_desc){
		.environment = sglue_environment(),
		.logger.func = slog_func,
	});
	assert(sg_isvalid());
	if(debug & Debugrender)
		sgx_enable_debug_log();
	initgl();
	initnk();
	resetview();
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
