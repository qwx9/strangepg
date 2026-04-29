#include "strpg.h"

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
endmove(void)
{
	drawstate &= ~DSmoving;
	if(drawstate & DSstalepick)
		reqdraw(Reqpickbuf);
}

static void
renderdirect(int nn, int ne, int nl)
{
	static Clk clk = {.lab = "renderdirect"};

	CLK0(clk);
	sg_begin_pass(&(sg_pass){
		.action = render.clearscreen,
		.swapchain = sglue_swapchain(),
	});
	if(nl >= 1){
		sg_apply_pipeline(render.linepipe);
		sg_apply_bindings(&render.linebind);
		sg_apply_uniforms(UB_line_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nlinev * nl, 1);
	}
	if(ne >= 1){
		sg_apply_pipeline(render.edgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_edge_Vparam, &SG_RANGE(render.cam));
		sg_apply_uniforms(UB_edge_Fparam, &SG_RANGE(render.edgefs));
		sg_draw(0, render.nedgev * ne, 1);
	}
	if(nn >= 1){
		sg_apply_pipeline(render.nodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_node_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, nn);
	}
	drawui(snk_new_frame());
	snk_render(sapp_width(), sapp_height());
	sg_end_pass();
	CLK1(clk);
	render.nframes++;
}

static void
renderoffscreen(int nn, int ne)
{
	static Clk clk = {.lab = "renderoffscreen"};

	CLK0(clk);
	sg_begin_pass(&(sg_pass){
		.action = render.clearpick,
		.attachments = {
			.colors[0] = render.pickvw,
			.depth_stencil = render.depthvw,
		},
	});
	if(ne >= 1){
		sg_apply_pipeline(render.offscredgepipe);
		sg_apply_bindings(&render.edgebind);
		sg_apply_uniforms(UB_edge_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nedgev * ne, 1);
	}
	if(nn >= 1){
		sg_apply_pipeline(render.offscrnodepipe);
		sg_apply_bindings(&render.nodebind);
		sg_apply_uniforms(UB_node_Vparam, &SG_RANGE(render.cam));
		sg_draw(0, render.nnodev, nn);
	}
	sg_end_pass();
	CLK1(clk);
	render.noframes++;
}

static void
staticbufs(int *nn, int *ne)
{
	int n;
	sg_buffer_desc d;

	if((n = dylen(redges)) > 0){
		d = sg_query_buffer_desc(render.edgebind.vertex_buffers[0]);
		if(d.size / sizeof *redges < n){
			DPRINT(Debugrender, "redge bindings: resize %zd → %d",
				d.size/sizeof *redges, n);
			sg_destroy_buffer(render.edgebind.vertex_buffers[0]);
			render.edgebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.size = n * sizeof *redges,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			});
			if(sg_query_buffer_state(render.edgebind.vertex_buffers[0]) != SG_RESOURCESTATE_VALID)
				n = 0;
		}
	}
	if(n > 0){
		sg_update_buffer(render.edgebind.vertex_buffers[0], &(sg_range){
			.ptr = redges,
			.size = n * sizeof *redges,
		});
	}
	*ne = n;
	if((n = dylen(vnodes)) > 0){
		d = sg_query_buffer_desc(render.nodebind.vertex_buffers[1]);
		if(d.size / sizeof *vnodes != n){
			DPRINT(Debugrender, "rnode bindings: resize %zd → %d",
				d.size/sizeof *vnodes, n);
			sg_destroy_buffer(render.nodebind.vertex_buffers[1]);
			render.nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = n * sizeof *vnodes,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			});
			if(sg_query_buffer_state(render.nodebind.vertex_buffers[1]) != SG_RESOURCESTATE_VALID)
				n = 0;
		}
	}
	if(n > 0){
		sg_update_buffer(render.nodebind.vertex_buffers[1], &(sg_range){
			.ptr = vnodes,
			.size = n * sizeof *vnodes,
		});
	}
	*nn = n;
}

static void
dynbufs(int *nn, int *ne)
{
	int n;
	sg_buffer_desc d;

	/* FIXME: maybe don't resize on every frame? only grow? */
	sg_uninit_buffer(render.edgebind.vertex_buffers[0]);
	sg_uninit_buffer(render.nodebind.vertex_buffers[1]);
	if((n = dylen(redges)) > 0){
		d = sg_query_buffer_desc(render.edgebind.vertex_buffers[0]);
		if(d.size / sizeof *redges != n){
			DPRINT(Debugrender, "redge bindings: resize %zd → %d", d.size/sizeof *redges, n);
			sg_destroy_buffer(render.edgebind.vertex_buffers[0]);
			render.edgebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.size = 0,
				.data = {
					.ptr = redges,
					.size = n * sizeof *redges,
				},
			});
			if(sg_query_buffer_state(render.edgebind.vertex_buffers[0]) != SG_RESOURCESTATE_VALID)
				n = 0;
		}
	}
	*ne = n;
	if((n = dylen(vnodes)) > 0){
		d = sg_query_buffer_desc(render.nodebind.vertex_buffers[1]);
		if(d.size / sizeof *vnodes != n){
			DPRINT(Debugrender, "rnode bindings: resize %zd → %d", d.size/sizeof *vnodes, n);
			sg_destroy_buffer(render.nodebind.vertex_buffers[1]);
			render.nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = 0,
				.data = {
					.ptr = vnodes,
					.size = n * sizeof *vnodes,
				},
			});
			if(sg_query_buffer_state(render.nodebind.vertex_buffers[1]) != SG_RESOURCESTATE_VALID)
				n = 0;
		}
	}
	*nn = n;
}

static void
updatebuffers(int *nn, int *ne, int *nl)
{
	int n;
	sg_buffer_desc d;

	if(drawing.flags & DFnoray)
		staticbufs(nn, ne);
	else
		dynbufs(nn, ne);
	if((n = dylen(rlines)) > 0){
		d = sg_query_buffer_desc(render.linebind.vertex_buffers[0]);
		if(d.size / sizeof *rlines < n){
			DPRINT(Debugrender, "rline bindings: resize %zd → %d",
				d.size/sizeof *rlines, n);
			sg_destroy_buffer(render.linebind.vertex_buffers[0]);
			render.linebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.size = n * sizeof *rlines,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			});
			if(sg_query_buffer_state(render.linebind.vertex_buffers[0]) != SG_RESOURCESTATE_VALID)
				n = 0;
		}
	}
	if(n > 0){
		sg_update_buffer(render.linebind.vertex_buffers[0], &(sg_range){
			.ptr = rlines,
			.size = n * sizeof *rlines,
		});
	}
	*nl = n;
}

static void
renderscene(int nn, int ne, int nl, int go)
{
	DPRINT(Debugrender, "renderscene %d %d %d", nn, ne, nl);
	render.cam.mvp = view.mvp;
	if(nn > 0 && render.caching && go)
		renderoffscreen(nn, ne);
	renderdirect(nn, ne, nl);
	sg_commit();
}

void
wakedrawup(void)
{
	DPRINT(Debugdraw, "wake the fup");
    sapp_input_wait(false);
}

void
renderframe(ulong req, int snooze)
{
	int go;
	vlong t;
	static int nn, ne, nl;
	static vlong t0;

	DPRINT(Debugrender, "renderframe %lx snooze %d", req, snooze);
	/* FIXME: make this part of frame() in portable code? */
	/* FIXME: set time here? or in offscreen render */
	if((t = μsec()) - t0 >= (1000000 / 30)){	/* FIXME: tune */
		drawstate |= DSstalepick;
		render.caching = 1;
		t0 = t;
	}
	TIME("render", "wait", t);
	if(go = drawing.flags & DFfiring){
		lockdraw();
		if(canlockrend()){
			updatebuffers(&nn, &ne, &nl);
			unlockrend();
		}
		unlockdraw();
	}
	TIME("render", "resize", t);
	renderscene(nn, ne, nl, go);
	TIME("render", "push", t);
	if(go)
		render.caching = 0;
	if(graph.flags & GFdrawme)
		reqdraw(Reqrefresh);
	if(snooze)
		sapp_input_wait(true);
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
	initscreen(sapp_width(), sapp_height());
	initgl();
	initnk();
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
		.sample_count = drawing.flags & DFnomsaa ? 1 : 4,
		.enable_clipboard = true,
		.window_title = "strangepg",
		.icon.sokol_default = true,
		.logger.func = slog_func,
		.high_dpi = drawing.flags & DFhidpi,
	});
}
