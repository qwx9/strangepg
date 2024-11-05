#include "strpg.h"
#include <signal.h>
#include "lib/flextgl/flextGL.h"
#define	HANDMADE_MATH_IMPLEMENTATION
//#define	HANDMADE_MATH_NO_SIMD
#include "lib/HandmadeMath.h"
#define	SOKOL_IMPL
#define	SOKOL_GLCORE
#define	SOKOL_NO_ENTRY
#define SOKOL_GL_FUNCS_EXT	\
	_SG_XMACRO(glDebugMessageCallback,	void, (DEBUGPROC callback, const void * userParam))
//#define	NDEBUG
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
#include "drw.h"
#include "ui.h"
#include "threads.h"
#include "cmd.h"

extern char *node_vertsh, *node_fragsh;
extern char *edge_vertsh, *edge_fragsh;
extern char *scr_vertsh, *scr_fragsh;

void	event(const sapp_event*);
void	drawui(struct nk_context*);
void	initnk(void);

struct Color{
	float col[4];
};

#define	FNodesz	((float)Nodesz)
#define	Nodethiccc	((float)Ptsz)

typedef struct Params Params;
typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;
typedef struct Shader Shader;

/* FIXME: provide aligned data as per std140? */
struct Params{
	HMM_Mat4 mvp;
};
static HMM_Mat4 mvp;

struct Shader{
	char *name;
	GLenum type;
	char **src;
};

static sg_pass_action offscreen_pass_action, offscreen_pass_action2;
static sg_pass_action default_pass_action;
static sg_attachments offscreen_attachments;
static sg_pipeline nodepip;
static sg_bindings nodebind;
static sg_pipeline edgepip;
static sg_bindings edgebind;
static sg_pipeline fsq_pip;
static sg_bindings fsq_bind;
static sg_image fb, pickfb, zfb;
static int nnodev, nedgev;

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

ioff
mousepick(int x, int y)
{
	ioff i;

	if((i = sg_query_image_pixel(x, y, pickfb)) == 0)
		return -1;
	return i;
}

/* FIXME: promote following functions to portable code; alias HMM types? */
static void
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

	if((n = ndedges) < 1)
		return;
	sg_update_buffer(edgebind.vertex_buffers[1], &(sg_range){
		.ptr = redges,
		.size = n * sizeof *redges,
	});
	sg_begin_pass(&(sg_pass){
		.action = offscreen_pass_action,
		.attachments = offscreen_attachments,
	});
	sg_apply_pipeline(edgepip);
	sg_apply_bindings(&edgebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, nedgev, n);
	sg_end_pass();
}

static inline void
rendernodes(Params p)
{
	ioff n;

	if((n = ndnodes) < 1)
		return;
	sg_update_buffer(nodebind.vertex_buffers[1], &(sg_range){
		.ptr = rnodes,
		.size = n * sizeof *rnodes,
	});
	sg_begin_pass(&(sg_pass){
		.action = offscreen_pass_action2,
		.attachments = offscreen_attachments,
	});
	sg_apply_pipeline(nodepip);
	sg_apply_bindings(&nodebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, nnodev, n);
	sg_end_pass();
}

static inline void
renderscreen(Params p)
{
	sg_begin_pass(&(sg_pass){
		.action = default_pass_action,
		.swapchain = sglue_swapchain(),
	});
	sg_apply_pipeline(fsq_pip);
	sg_apply_bindings(&fsq_bind);
	sg_draw(0, 4, 1);
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
	if(dylen(rnodes) > 0){
		renderedges(p);
		rendernodes(p);
	}
	renderscreen(p);
	sg_commit();
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
	offscreen_attachments = sg_make_attachments(&(sg_attachments_desc){
		.colors = {
			[0].image = fb,
			[1].image = pickfb,
		},
		.depth_stencil.image = zfb,
	});
	fsq_bind.fs.images[0] = fb;
}

static void
resize(void)
{
	sg_destroy_image(fb);
	sg_destroy_image(pickfb);
	sg_destroy_image(zfb);
	sg_destroy_attachments(offscreen_attachments);
	initfb(sapp_width(), sapp_height());
	updateview();
}

static void
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

	if(nnodev != 0){
		sg_destroy_buffer(nodebind.vertex_buffers[0]);
		sg_destroy_buffer(nodebind.index_buffer);
	}
	if(arrow){
		nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.data = SG_RANGE(arrowv),
		});
		nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(arrowi),
		}),
		nnodev = nelem(arrowi);
	}else{
		nodebind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
			.data = SG_RANGE(quadv),
		});
		nodebind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(quadi),
		}),
		nnodev = nelem(quadi);
	}
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
		}else if((reqs & Reqshallowdraw) == 0)
			sapp_input_wait(true);
	}
}

void
frame(void)
{
	struct nk_context *ctx;
	Clk clk = {.lab = "flush"}, fclk = {.lab = "frame"};

	CLK0(fclk);
	ctx = snk_new_frame();
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
	reqs &= ~Reqshallowdraw;
	drawui(ctx);
	CLK0(clk);
	flush();
	CLK1(clk);
	CLK1(fclk);
	reqdraw(Reqrefresh);
}

static void
initgl(void)
{
	Color *c;

	/* bindings for instancing + offscreen rendering: vertex buffer slot 1
	 * is used for instance data */
	nodebind = (sg_bindings){
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
	nedgev = nelem(edgeidx);
	if(debug & Debugdraw)
		sg_enable_debug_log();
	edgebind = (sg_bindings){
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

	c = color(theme[Cbg]);
	/* clear colors on the first pass, don't do anything on the second */
	offscreen_pass_action = (sg_pass_action){
		.colors = {
			[0] = {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { c->col[0], c->col[1], c->col[2], 0.0f },
			},
			[1] = {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { 0 },
			},
		},
	};
	offscreen_pass_action2 = (sg_pass_action){
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
	nodepip = sg_make_pipeline(&(sg_pipeline_desc){
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
					.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
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
	edgepip = sg_make_pipeline(&(sg_pipeline_desc){
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
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 2,
		.colors = {
			[0] = {
				.blend = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
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
	fsq_bind = (sg_bindings){
		.vertex_buffers[0] = quad_vbuf,
		.fs.samplers[0] = smp,
	};
	sg_shader fsq_shd = sg_make_shader(&(sg_shader_desc){
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
	fsq_pip = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs[0].format = SG_VERTEXFORMAT_FLOAT2
		},
		.shader = fsq_shd,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP
	});
	/* no clear needed, since whole screen is overwritten */
	default_pass_action = (sg_pass_action){
		.colors[0].load_action = SG_LOADACTION_DONTCARE,
		.depth.load_action = SG_LOADACTION_DONTCARE,
		.stencil.load_action = SG_LOADACTION_DONTCARE
	};
	initfb(sapp_width(), sapp_height());
}

static void
init(void)
{
	if(!flextInit())
		sysfatal("flextGL: failed to initialize: %s", error());
	sg_setup(&(sg_desc){
		.environment = sglue_environment(),
		.logger.func = slog_func,
	});
	assert(sg_isvalid());
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
	/* FIXME: the reason why this can't be 0 is because of the fucking
	 * input handling being synchronous */
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
