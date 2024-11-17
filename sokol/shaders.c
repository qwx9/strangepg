#include "strpg.h"
#define	HANDMADE_MATH_IMPLEMENTATION
//#define	HANDMADE_MATH_NO_SIMD
#include "lib/HandmadeMath.h"
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
#include "sokol.h"
#define SOKOL_SHDC_IMPL
#include "glsl/edge.h"
#include "glsl/edgeidx.h"
#include "glsl/node.h"
#include "glsl/nodeidx.h"
#include "glsl/scr.h"
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

Render render;

static sg_image fb, pickfb, zfb;

#define	FNodesz	((float)Nodesz)
#define	Nodethiccc	((float)Ptsz)

ioff
mousepick(int x, int y)
{
	u32int i;

	if(x < 0 || x >= view.w || y < 0 || y >= view.h)
		return -1;
	if(render.stalepick && !render.moving){
		sg_query_image_pixels(pickfb, render.pickfb);
		render.stalepick = 0;
	}
	if((i = render.pickfb[(view.h - y - 1) * view.w + x]) == 0)
		return -1;
	return i;
}

void
setnodeshape(int arrow)
{
	float quadv[] = {
		-FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, -Nodethiccc/2.0f,
		-FNodesz/2.0f, -Nodethiccc/2.0f,
	}, arrowv[] = {
		 0.50f * FNodesz,	-0.5f * Nodethiccc,
		-0.00f * FNodesz,	-0.5f * Nodethiccc,
		-0.00f * FNodesz,	 0.5f * Nodethiccc,
		 0.50f * FNodesz,	 0.5f * Nodethiccc,
		-0.00f * FNodesz,	-1.0f * Nodethiccc,
		-0.50f * FNodesz,	-0.0f * Nodethiccc,
		-0.00f * FNodesz,	 1.0f * Nodethiccc,
	};
	u16int quadi[] = {
		0, 2, 1,	// first triangle
		0, 3, 2,	// second triangle
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
		render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.data = SG_RANGE(arrowv),
		});
		render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(arrowi),
		}),
		render.nnodev = nelem(arrowi);
	}else{
		render.nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.data = SG_RANGE(quadv),
		});
		render.nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(quadi),
		}),
		render.nnodev = nelem(quadi);
	}
}

static void
initfb(int w, int h)
{
	view.w = w;
	view.h = h;
	/* multi-target rendering targets, without multisampling */
	fb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = w,
		.height = h,
		.sample_count = 1,
	});
	pickfb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_R32UI,
		.sample_count = 1,
	});
	zfb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = w,
		.height = h,
		.pixel_format = SG_PIXELFORMAT_DEPTH,
		.sample_count = 1,
	});
	render.offscrfb = sg_make_attachments(&(sg_attachments_desc){
		.colors = {
			[0].image = fb,
			[1].image = pickfb,
		},
		.depth_stencil.image = zfb,
	});
	render.offscrbind.images[0] = fb;
	render.pickfb = emalloc(w * h * sizeof *render.pickfb);
}

void
resize(void)
{
	sg_destroy_image(fb);
	sg_destroy_image(pickfb);
	sg_destroy_image(zfb);
	free(render.pickfb);
	sg_destroy_attachments(render.offscrfb);
	initfb(sapp_width(), sapp_height());
	updateview();
}

static void
setupnodes(void)
{
	/* bindings for instancing + offscreen rendering: vertex buffer slot 1
	 * is used for instance data */
	render.nodebind = (sg_bindings){
		.vertex_buffers = {
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = dylen(rnodes) * sizeof *rnodes,
				.usage = SG_USAGE_STREAM,
			}),
		},
	};
	setnodeshape(0);
	sg_shader sh = sg_make_shader(n_node_shader_desc(sg_query_backend()));
	render.nodepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = 2 * sizeof(float),
				},
				[1] = {
					.stride = sizeof(RNode),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[ATTR_node_geom] = {
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[ATTR_node_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_node_dir] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_node_col0] = {
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[ATTR_node_len] = {
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
		.colors = {
			[0] = {
				.blend = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
		},
		.sample_count = 1,
	});
	sh = sg_make_shader(ni_nodeidx_shader_desc(sg_query_backend()));
	render.offscrnodepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = 2 * sizeof(float),
				},
				[1] = {
					.stride = sizeof(RNode),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[ATTR_nodeidx_geom] = {
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[ATTR_nodeidx_pos] = {
					.offset = offsetof(RNode, pos),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_nodeidx_dir] = {
					.offset = offsetof(RNode, dir),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[ATTR_nodeidx_col0] = {
					.offset = offsetof(RNode, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[ATTR_nodeidx_len] = {
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
		.color_count = 2,
		/* NOTE: per-attachment blend state may not be supported */
		.colors = {
			[0] = {
				.blend = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
			[1] = {
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
				.usage = SG_USAGE_STREAM,
			}),
		},
	};
	sh = sg_make_shader(e_edge_shader_desc(sg_query_backend()));
	render.edgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs = {
				[ATTR_edge_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_LINES, 
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		//.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 1,
		.colors = {
			[0] = {
				.blend = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
		},
		.sample_count = 1,
	});
	sh = sg_make_shader(ei_edgeidx_shader_desc(sg_query_backend()));
	render.offscredgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs = {
				[ATTR_edgeidx_pos] = {
					.format = SG_VERTEXFORMAT_FLOAT3,
				},
			}
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		//.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 2,
		.colors = {
			[0] = {
				.blend = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_ONE,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_alpha = SG_BLENDOP_ADD,
				},
			},
			[1] = {
				.pixel_format = SG_PIXELFORMAT_R32UI,
			},
		},
		.sample_count = 1,
	});
	render.nedgev = 2;
}

static void
setupscreen(void)
{
	sg_shader sh;

	/* drawing to display */
	float quad_vertices[] = {
		0.0f, 0.0f, 
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};
	sg_buffer quad_vbuf = sg_make_buffer(&(sg_buffer_desc){
		.data = SG_RANGE(quad_vertices)
	});
	sg_sampler smp = sg_make_sampler(&(sg_sampler_desc){
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR,
		.wrap_u = SG_WRAP_CLAMP_TO_EDGE,
		.wrap_v = SG_WRAP_CLAMP_TO_EDGE,
	});
	render.offscrbind = (sg_bindings){
		.vertex_buffers[0] = quad_vbuf,
		.samplers[SMP_smp] = smp,
	};
	sh = sg_make_shader(s_scr_shader_desc(sg_query_backend()));
	render.offscrpipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs[ATTR_scr_pos].format = SG_VERTEXFORMAT_FLOAT2
		},
		.shader = sh,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP
	});
}

static void
setuppasses(void)
{
	Color *c;

	c = color(theme[Cbg]);
	/* clear colors on the first pass, don't do anything on the second */
	render.clearscreen = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { c->col[0], c->col[1], c->col[2], c->col[3] },
			},
			[1] = {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { 0.0f },
			},
		},
	};
	render.overdraw = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_DONTCARE,
				.store_action = SG_STOREACTION_DONTCARE,
			},
			[1] = {
				.load_action = SG_LOADACTION_DONTCARE,
				.store_action = SG_STOREACTION_DONTCARE,
			},
		},
		.depth.load_action = SG_LOADACTION_DONTCARE,
		.stencil.load_action = SG_LOADACTION_DONTCARE,
	};
	/* screen pipeline: no clear needed, whole screen is overwritten */
	render.nothing = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_DONTCARE,
				/* image set in initfb */
			},
		},
		.depth.load_action = SG_LOADACTION_DONTCARE,
		.stencil.load_action = SG_LOADACTION_DONTCARE,
	};
}

static void
setuplines(void)
{
	/* FIXME */
}

void
initgl(void)
{
	setupnodes();
	setupedges();
	setupscreen();
	setuplines();
	setuppasses();
	setcolor(render.edgefs.color, theme[Cedge]);
	initfb(sapp_width(), sapp_height());
	onscreen = 1;
}
