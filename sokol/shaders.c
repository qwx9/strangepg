#include "strpg.h"
#include "lib/HandmadeMath.h"
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
#include "sokol.h"
#include "glsl/edge.h"
#include "glsl/node.h"
#include "glsl/line.h"
#include "drw.h"
#include "view.h"
#include "ui.h"
#include "threads.h"

Render render;

static sg_image pickfb, zfb;

u32int
mousepick(int x, int y)
{
	u32int i;

	if(x < 0 || x >= sapp_width() || y < 0 || y >= sapp_height()){
		DPRINT(Debugui, "mousepick %d,%d: out of bounds", x, y);
		return -1;
	}
	if(render.noframes < 1){
		DPRINT(Debugui, "mousepick: no offscreen render yet");
		return -1;
	}
	DPRINT(Debugui, "mousepick %d,%d stale %d move %d",
		x, y, drawstate & DSstalepick, drawstate & DSmoving);
	if((drawstate & (DSstalepick | DSmoving)) == DSstalepick){
		sgx_query_image_pixels(pickfb, render.pickfb);
		drawstate &= ~DSstalepick;
	}
	if(render.pickflip)
		y = sapp_height() - y;
	i = render.pickfb[(sapp_height() - y - 1) * sapp_width() + x];
	DPRINT(Debugui, "mousepick: %x", i);
	if(i == 0)
		return -1;
	return i - 1;
}

void
setnodeshape(int arrow)
{
	float quadv3d[] = {
		-0.5f * drawing.nodesz,	+0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
	}, arrowv3d[] = {
		-0.1f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, +0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, -0.5f * drawing.fatness,
		+0.5f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.1f * drawing.nodesz, -0.5f * drawing.fatness, +0.5f * drawing.fatness,
		-0.5f * drawing.nodesz, -0.0f * drawing.fatness, +0.0f * drawing.fatness, /* tip */
	}, quadv[] = {
		-0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		-0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
	}, arrowv[] = {
		+0.5f * drawing.nodesz,	-0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	-0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	+0.5f * drawing.fatness,
		+0.5f * drawing.nodesz,	+0.5f * drawing.fatness,
		-0.0f * drawing.nodesz,	-1.5f * drawing.fatness,
		-0.5f * drawing.nodesz,	-0.0f * drawing.fatness,
		-0.0f * drawing.nodesz,	+1.5f * drawing.fatness,
	};
	u16int quadi3d[] = {
		2, 0, 3,
		2, 1, 0,
		5, 2, 6,
		5, 1, 2,
		6, 3, 7,
		6, 2, 3,
		7, 0, 4,
		7, 3, 0,
		4, 1, 5,
		4, 0, 1,
		5, 7, 4,
		5, 6, 7,
	}, arrowi3d[] = {
		2, 0, 3,	/* shaft */
		2, 1, 0,
		5, 2, 6,
		5, 1, 2,
		6, 3, 7,
		6, 2, 3,
		4, 1, 5,
		4, 0, 1,
		5, 7, 4,
		5, 6, 7,
		8, 3, 0,	/* tip */
		8, 7, 3,
		8, 4, 7,
		8, 0, 4,
	}, quadi[] = {
		2, 0, 3,
		2, 1, 0,
	}, arrowi[] = {
		2, 0, 3,	// shaft
		2, 1, 0,
		6, 5, 4,	// tip
	};

	if(render.nnodev != 0){
		sg_destroy_buffer(render.nodebind.vertex_buffers[0]);
		sg_destroy_buffer(render.nodebind.index_buffer);
	}
	if(arrow){
		if(drawing.flags & DF3d){
			render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(arrowv3d),
				.usage = {
					.vertex_buffer = true,
					.immutable = true,
				},
			});
			render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(arrowi3d),
				.usage = {
					.index_buffer = true,
					.immutable = true,
				},
			}),
			render.nnodev = nelem(arrowi3d);
		}else{
			render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(arrowv),
				.usage = {
					.vertex_buffer = true,
					.immutable = true,
				},
			});
			render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(arrowi),
				.usage = {
					.index_buffer = true,
					.immutable = true,
				},
			}),
			render.nnodev = nelem(arrowi);
		}
	}else{
		if(drawing.flags & DF3d){
			render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(quadv3d),
				.usage = {
					.vertex_buffer = true,
					.immutable = true,
				},
			});
			render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(quadi3d),
				.usage = {
					.index_buffer = true,
					.immutable = true,
				},
			}),
			render.nnodev = nelem(quadi3d);
		}else{
			render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(quadv),
				.usage = {
					.vertex_buffer = true,
					.immutable = true,
				},
			});
			render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(quadi),
				.usage = {
					.index_buffer = true,
					.immutable = true,
				},
			}),
			render.nnodev = nelem(quadi);
		}
	}
}

static void
initfb(int w, int h)
{
	view.w = sapp_width();
	view.h = sapp_height();
	pickfb = sg_make_image(&(sg_image_desc){
		.usage = {
			.color_attachment = true,
			.immutable = true,
		},
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_R32UI,
		.sample_count = 1,
	});
	zfb = sg_make_image(&(sg_image_desc){
		.usage = {
			.depth_stencil_attachment = true,
			.immutable = true,
		},
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_DEPTH,
		.sample_count = 1,
	});
	render.pickvw = sg_make_view(&(sg_view_desc){
		.color_attachment.image = pickfb,
	});
	render.depthvw = sg_make_view(&(sg_view_desc){
		.depth_stencil_attachment.image = zfb,
	});
	render.pickfb = emalloc(w * h * sizeof *render.pickfb);
}

void
resize(void)
{
	sg_destroy_image(pickfb);
	sg_destroy_image(zfb);
	free(render.pickfb);
	sg_destroy_view(render.pickvw);
	sg_destroy_view(render.depthvw);
	initfb(sapp_width(), sapp_height());
}

static void
setupnodes(int threedee)
{
	/* bindings for instancing + offscreen rendering: vertex buffer slot 1
	 * is used for instance data */
	render.nodebind = (sg_bindings){
		.vertex_buffers = {
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = dylen(rnodes) * sizeof *rnodes,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			}),
		},
	};
	setnodeshape(0);
	sg_shader sh = sg_make_shader((threedee ? node_s3d_shader_desc : node_s_shader_desc)(sg_query_backend()));
	render.nodepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = (threedee ? 3 : 2) * sizeof(float),
				},
				[1] = {
					.stride = sizeof(RNode),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[ATTR_node_s_geom] = {
					.format = threedee ? SG_VERTEXFORMAT_FLOAT3 : SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[ATTR_node_s_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_node_s_dir] = {
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[ATTR_node_s_col] = {
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[ATTR_node_s_len] = {
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true
		},
		.color_count = 1,
		/* NOTE: per-attachment blend state may not be supported */
		.colors = {
			[0] = {
				.blend = {
					.enabled = (drawing.flags & DFnoalpha) == 0,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
		},
		.sample_count = drawing.flags & DFmsaa ? 4 : 1,
	});
	sh = sg_make_shader((threedee ? node_idx3d_shader_desc : node_idx_shader_desc)(sg_query_backend()));
	render.offscrnodepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = (threedee ? 3 : 2) * sizeof(float),
				},
				[1] = {
					.stride = sizeof(RNode),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[ATTR_node_idx_geom] = {
					.format = threedee ? SG_VERTEXFORMAT_FLOAT3 : SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[ATTR_node_idx_pos] = {
					.offset = offsetof(RNode, pos),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_node_idx_dir] = {
					.offset = offsetof(RNode, dir),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[ATTR_node_idx_len] = {
					.offset = offsetof(RNode, len),
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true
		},
		.color_count = 1,
		.colors = {
			[0] = {
				.pixel_format = SG_PIXELFORMAT_R32UI,
			},
		},
		.sample_count = 1,
	});
}

static void
setupedges(void)
{
	sg_shader sh;

	render.edgebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.size = dylen(redges) * sizeof *redges,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			}),
		},
	};
	sh = sg_make_shader(edge_s_shader_desc(sg_query_backend()));
	render.edgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs = {
				[ATTR_edge_s_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_LINES, 
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.depth = {
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 1,
		.colors = {
			[0] = {
				.blend = {
					.enabled = (drawing.flags & DFnoalpha) == 0,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
		},
		.sample_count = drawing.flags & DFmsaa ? 4 : 1,
	});
	sh = sg_make_shader(edge_idx_shader_desc(sg_query_backend()));
	render.offscredgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs = {
				[ATTR_edge_idx_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.depth = {
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 1,
		.colors = {
			[0] = {
				.pixel_format = SG_PIXELFORMAT_R32UI,
			},
		},
		.sample_count = 1,
	});
	render.nedgev = 2;
}

static void
setuplines(void)
{
	int n;
	sg_shader sh;

	n = dylen(rlines) + 4 + 7 & ~7;
	dygrow(rlines, n);
	render.linebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.size = n * sizeof *rlines,
				.usage = {
					.vertex_buffer = true,
					.stream_update = true,
				},
			}),
		},
	};
	sh = sg_make_shader(line_s_shader_desc(sg_query_backend()));
	render.linepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs = {
				[ATTR_line_s_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT4,
				},
				[ATTR_line_s_color] = {
					.format = SG_VERTEXFORMAT_FLOAT4,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_LINES, 
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.depth = {
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 1,
		.colors = {
			[0] = {
				.blend = {
					.enabled = (drawing.flags & DFnoalpha) == 0,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
		},
		.sample_count = drawing.flags & DFmsaa ? 4 : 1,
	});
	render.nlinev = 2;
}

static void
setuppasses(void)
{
	float c[4];

	setcolor(c, theme[Cbg]);
	render.clearscreen = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_CLEAR,
				.clear_value = { c[0], c[1], c[2], c[3] },
			},
		},
	};
	render.clearpick = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_CLEAR,
				.clear_value = { 0.0f, 0.0f, 0.0f, 1.0f },
			},
		},
	};
}

void
initgl(void)
{
	sg_backend b;

	setupnodes(drawing.flags & DF3d);
	setupedges();
	setuplines();
	setuppasses();
	setcolor(render.edgefs.color, theme[Cedge]);
	initfb(sapp_width(), sapp_height());
	b = sg_query_backend();
	render.pickflip = b != SG_BACKEND_GLCORE && b != SG_BACKEND_GLES3;
	drawstate |= DSstalepick;
	onscreen = 1;
}
