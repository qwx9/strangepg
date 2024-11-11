#include "strpg.h"
#define	HANDMADE_MATH_IMPLEMENTATION
//#define	HANDMADE_MATH_NO_SIMD
#include "lib/HandmadeMath.h"
#define	SOKOL_GLCORE
//#define	NDEBUG
#include "lib/sokol_app.h"
#include "lib/sokol_gfx.h"
//#include "lib/sokol_log.h"
#include "lib/sokol_glue.h"
#include "sokol_gfx_ext.h"
#include "sokol.h"
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
	render.offscrbind.fs.images[0] = fb;
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

void
initgl(void)
{
	Color *c;

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

	float edgevert[] = { -1, 1 };
	u16int edgeidx[] = { 0, 1 };
	render.nedgev = nelem(edgeidx);
	render.edgebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(edgevert),
			}),
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = (dylen(redges) + nelem(selbox)) * sizeof *redges,
				.usage = SG_USAGE_STREAM,
			}),
		},
		.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(edgeidx),
		}),
	};

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
	sg_shader nodesh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
			},
		},
		.vs.source = node_vertsh,
		.fs.source = node_fragsh,
	});
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
				[0] = {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[1] = {
					.offset = offsetof(RNode, pos),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[2] = {
					.offset = offsetof(RNode, dir),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[3] = {
					.offset = offsetof(RNode, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[4] = {
					.offset = offsetof(RNode, len),
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
			}
		},
		.shader = nodesh,
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
	sg_shader nodeidxsh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
			},
		},
		.vs.source = nodeidx_vertsh,
		.fs.source = nodeidx_fragsh,
	});
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
				[0] = {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[1] = {
					.offset = offsetof(RNode, pos),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[2] = {
					.offset = offsetof(RNode, dir),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[3] = {
					.offset = offsetof(RNode, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
				[4] = {
					.offset = offsetof(RNode, len),
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
			}
		},
		.shader = nodeidxsh,
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
	sg_shader edgesh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
			},
		},
		.vs.source = edge_vertsh,
		.fs.source = edge_fragsh,
	});
	render.edgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = sizeof(float),
				},
				[1] = {
					.stride = sizeof(REdge),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[0] = {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 0,
				},
				[1] = {
					.offset = offsetof(REdge, pos1),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[2] = {
					.offset = offsetof(REdge, pos2),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[3] = {
					.offset = offsetof(REdge, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
			}
		},
		.shader = edgesh,
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.index_type = SG_INDEXTYPE_UINT16,
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
	sg_shader edgeidxsh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
			},
		},
		.vs.source = edgeidx_vertsh,
		.fs.source = edgeidx_fragsh,
	});
	render.offscredgepipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] = {
					.stride = sizeof(float),
				},
				[1] = {
					.stride = sizeof(REdge),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[0] = {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 0,
				},
				[1] = {
					.offset = offsetof(REdge, pos1),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[2] = {
					.offset = offsetof(REdge, pos2),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[3] = {
					.offset = offsetof(REdge, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
					.buffer_index = 1,
				},
			}
		},
		.shader = edgeidxsh,
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.face_winding = SG_FACEWINDING_CCW,
		.cull_mode = SG_CULLMODE_BACK,
		.index_type = SG_INDEXTYPE_UINT16,
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
		.fs.samplers[0] = smp,
	};
	sg_shader scrsh = sg_make_shader(&(sg_shader_desc){
		.vs = {
			.source = scr_vertsh,
		},
		.fs = {
			.images = {
				[0].used = true,
			},
			.samplers[0].used = true,
			.image_sampler_pairs = {
				[0] = {
					.used = true,
					.glsl_name = "tex0",
					.image_slot = 0,
					.sampler_slot = 0
				},
			},
			.source = scr_fragsh,
		}
	});
	render.offscrpipe = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs[0].format = SG_VERTEXFORMAT_FLOAT2
		},
		.shader = scrsh,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP
	});
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
	initfb(sapp_width(), sapp_height());
}
