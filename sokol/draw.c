#include "strpg.h"
#include <signal.h>
#include "lib/flextgl/flextGL.h"
#define	HANDMADE_MATH_IMPLEMENTATION
//#define	HANDMADE_MATH_NO_SIMD
#include "lib/HandmadeMath.h"
#define	SOKOL_IMPL
#define	SOKOL_GLCORE
#define	SOKOL_NO_ENTRY
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
void	_drawui(struct nk_context*);
void	setnktheme(void);

struct Color{
	u32int col;
	float r;
	float g;
	float b;
};

/* FIXME:
 * - resize: compensate for different viewsize == different gl coordinate system
 *	by moving eye (zoom + pan)
 * - debug node angle and scale, offset edge control points
 */
/* FIXME: blackjack and hookers:
 * - switch to opengl4, add tesselation, generalize nodes as thick lines, shape
 *	nodes and edges during tesselation stage, store shape params, implement beziers
 *		.. see discussion on tesselation: mesh shaders instead?
 * - start/stopdrawclock (larger draw buffer than screen + redraw when necessary?)
 *	redraw only, not re-render etc; or rather toggle continuous redraw?
 *	does performance/load suffer?
 * - draw labels, arrows
 * - beziers, thickness: https://thebookofshaders.com/05/
 * - extend for 3d vis and navigation, later port to plan9
 * - 3d layout algo on top of this, would be nice to have by may
 *	zoom will just become z-panning
 */

#define	FNodesz	((float)Nodesz)
#define	Nodethiccc	(FNodesz/Ptsz)
#define	FOV	60.0f

typedef struct Params Params;
typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;

#define	hjdicks __attribute((packed))
struct hjdicks Params{
	HMM_Mat4 mvp;
	HMM_Vec2 scale;	/* FIXME */
};
struct hjdicks GLNode{
	HMM_Vec2 pos;
	HMM_Vec3 col;
	float θ;
	u32int idx;
};
struct hjdicks GLEdge{
	HMM_Vec2 pos1;
	HMM_Vec2 pos2;
	HMM_Vec3 col;
	u32int idx;
};
static GLNode *nodev;
static GLEdge *edgev;
static HMM_Mat4 mvp;

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

static Channel *drawc;

Color *
newcolor(u32int v)
{
	Color *c;

	c = emalloc(sizeof *c);
	c->col = v;
	c->r = (v >> 16 & 0xff) / 255.f;
	c->g = (v >> 8 & 0xff) / 255.f;
	c->b = (v & 0xff) / 255.f;
	return c;
}

u32int
col2int(Color *c)
{
	return c->col;
}

u32int
scrobj(int x, int y)
{
	return sg_query_image_pixel(x, y, pickfb);
}

/* FIXME: parts in shared port code */
void
showobj(Obj *o)
{
	char s[128];
	Node *n;
	Edge *e;
	static Edge *ee;
	static Node *nn;

	if(selected.type == Onil)
		return;
	switch(selected.type){
	case Oedge:
		e = o->g->edges + o->idx;
		if(e == ee)
			return;
		snprint(s, sizeof s, "E[%zx] %zx,%zx", o->idx, e->u, e->v);
		ee = e;
		pushcmd("print %d, ledge[%d]", e->id, e->id);
		break;
	case Onode:
		n = o->g->nodes + o->idx;
		if(n == nn)
			return;
		snprint(s, sizeof s, "V[%zx] %zx", o->idx, n->id);
		nn = n;
		pushcmd("print %d, lnode[%d]", n->id, n->id);
		break;
	}
	//string(screen, statr.min, color(theme[Ctext])->i, ZP, font, s);
	warn("selected: %s\n", s);
}

int
drawlabel(Node *, Color *)
{
	return 0;
}

// FIXME: generalize drawbezier, merge to drawline?
int
drawline(Vertex a, Vertex b, double w, int emph, s32int idx, Color *c)
{
	GLEdge e = {
		.pos1 = {
			.X = a.x,
			.Y = a.y,
		},
		.pos2 = {
			.X = b.x,
			.Y = b.y,
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
		},
		.idx = idx < 0 ? 0 : (u32int)idx + 1,
	};

	dypush(edgev, e);
	return 0;
}

int
drawbezier(Vertex a, Vertex b, double w, s32int idx, Color *c)
{
	// FIXME
	return drawline(a, b, w, 0, idx, c);
}

int
drawquad(Vertex pos, Vertex rot, s32int idx, Color *c)
{
	GLNode v = {
		.pos = {
			.X = pos.x,
			.Y = pos.y,
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
		},
		.θ = rot.z,
		.idx = idx < 0 ? 0 : (u32int)idx + 1,
	};

	dypush(nodev, v);
	return 0;
}

void
cleardraw(void)
{
	dyreset(nodev);
	dyreset(edgev);
}

// FIXME: merge with port
static void
updateview(void)
{
	HMM_Vec3 eye, center, up;
	HMM_Mat4 vw, proj;

	view.Δeye = subv(view.eye, view.center);
	view.ar = view.w / view.h;
	proj = HMM_Perspective_RH_NO(60.0f, view.ar, 0.01f, 1000.0f);
	eye = HMM_V3(view.eye.x, view.eye.y, view.eye.z);
	center = HMM_V3(view.center.x, view.center.y, view.center.z);
	up = HMM_V3(view.up.x, view.up.y, view.up.z);
	vw = HMM_LookAt_RH(eye, center, up);
	mvp = HMM_MulM4(proj, vw);
}

void
zoomdraw(float Δ)
{
	view.eye = subv(view.eye, mulv(view.Δeye, Δ));
	updateview();
}

void
pandraw(float Δx, float Δy)
{
	float len;
	HMM_Vec3 dy, dx, c;

	Δx /= view.w;
	Δy /= view.h;
	view.center.x += Δx * 2.0f * view.Δeye.z * view.ar * tan(FOV / 2.0f);
	view.center.y -= Δy * 2.0f * view.Δeye.z * tan(FOV / 2.0f);
	view.eye.x = view.center.x;
	view.eye.y = view.center.y;
	updateview();
}

static inline void
renderedges(Params p)
{
	ssize n;
	sg_buffer_desc d;

	n = dylen(edgev);
	d = sg_query_buffer_desc(edgebind.vertex_buffers[1]);
	if(d.size / sizeof *edgev < n){
		sg_destroy_buffer(edgebind.vertex_buffers[1]);
		edgebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *edgev,
			.usage = SG_USAGE_STREAM,
		});
	}
	sg_update_buffer(edgebind.vertex_buffers[1], &(sg_range){
		.ptr = edgev,
		.size = n * sizeof *edgev,
	});
	sg_begin_pass(&(sg_pass){
		.action = offscreen_pass_action,
		.attachments = offscreen_attachments,
	});
	sg_apply_pipeline(edgepip);
	sg_apply_bindings(&edgebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, 2, n);
	sg_end_pass();
}

static inline void
rendernodes(Params p)
{
	ssize n;
	sg_buffer_desc d;

	n = dylen(nodev);
	d = sg_query_buffer_desc(nodebind.vertex_buffers[1]);
	if(d.size / sizeof *nodev < n){
		sg_destroy_buffer(nodebind.vertex_buffers[1]);
		nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *nodev,
			.usage = SG_USAGE_STREAM,
		});
	}
	sg_update_buffer(nodebind.vertex_buffers[1], &(sg_range){
		.ptr = nodev,
		.size = n * sizeof *nodev,
	});
	sg_begin_pass(&(sg_pass){
		.action = offscreen_pass_action2,
		.attachments = offscreen_attachments,
	});
	sg_apply_pipeline(nodepip);
	sg_apply_bindings(&nodebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, 6, n);
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
		.scale = HMM_V2(FNodesz, FNodesz),
	};
	if(dylen(nodev) > 0){
		renderedges(p);
		rendernodes(p);
	}
	renderscreen(p);
	sg_commit();
}

void
startdrawclock(void)
{
}

void
stopdrawclock(void)
{
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

void
reqdraw(int r)
{
	static ulong f;

	f |= r;
	if(nbsendul(drawc, f) != 0)
		f = 0;
}

void
frame(void)
{
	int req;
	struct nk_context *ctx;
	Clk clk = {.lab = "flush"}, fclk = {.lab = "frame"};

	req = nbrecvul(drawc) | Reqrefresh;
	CLK0(fclk);
	ctx = snk_new_frame();
	if((req & Reqresetdraw) != 0)
		resize();
	if((req & Reqresetui) != 0){
		resetui();
		updateview();
	}
	pollcmd();
	// FIXME: actually implement shallow redraw, etc.
	if(req != Reqshallowdraw){
		if(!redraw((req & Reqrefresh) != 0 || (req & Reqshape) != 0))
			stopdrawclock();
		_drawui(ctx);
	}
	CLK0(clk);
	flush();
	CLK1(clk);
	CLK1(fclk);
}

static void
initgl(void)
{
	Color *c;

	/* geometry */
	float nodevert[] = {
		-FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, -Nodethiccc/2.0f,
		-FNodesz/2.0f, -Nodethiccc/2.0f,
	};
	u16int nodeidx[] = {
		0, 1, 2,	// first triangle
		0, 2, 3,	// second triangle
	};
	float edgevert[] = { -FNodesz/2.0f, 0, +FNodesz/2.0f, 0 };
	u16int edgeidx[] = { 0, 1 };

	/* bindings for instancing + offscreen rendering: vertex buffer slot 1
	 * is used for instance data */
	nodebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(nodevert),
			}),
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = 1024 * sizeof(GLNode),
				.usage = SG_USAGE_STREAM,
			}),
		},
		.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(nodeidx),
		}),
	};
	edgebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(edgevert),
			}),
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = 1024 * sizeof(GLEdge),
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
				[1] = { .name="s", .type=SG_UNIFORMTYPE_FLOAT2 },
			},
		},
		/* FIXME: mvp and s: redundancy, s should be part of mvp or w/e */
		.vs.source = node_vertsh,
		.fs.source = node_fragsh,
	});
	sg_shader edgesh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
				[1] = { .name="s", .type=SG_UNIFORMTYPE_FLOAT2 },
			},
		},
		.vs.source = edge_vertsh,
		.fs.source = edge_fragsh,
	});

	c = color(theme[Cbg]);
	/* clear colors on the first pass, don't do anything on the second */
	offscreen_pass_action = (sg_pass_action){
		.colors = {
			[0] {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { c->r, c->g, c->b, 0.0f },
			},
			[1] {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { 0 },
			},
		},
	};
	offscreen_pass_action2 = (sg_pass_action){
		.colors = {
			[0] {
				.load_action = SG_LOADACTION_DONTCARE,
				.store_action = SG_STOREACTION_DONTCARE,
			},
			[1] {
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
				[0] {
					.stride = 2 * sizeof(float),
				},
				[1] {
					.stride = sizeof(GLNode),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[0] {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[1] {
					.offset = offsetof(GLNode, pos),
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 1,
				},
				[2] {
					.offset = offsetof(GLNode, col),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[3] {
					.offset = offsetof(GLNode, θ),
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
				[4] {
					.offset = offsetof(GLNode, idx),
					.format = SG_VERTEXFORMAT_UINT,
					.buffer_index = 1,
				},
			}
		},
		.shader = nodesh,
		.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true
		},
		.color_count = 2,
		/* NOTE: per-attachment blend state may not be supported */
		.colors = {
			[0] {
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
			[1] {
				.pixel_format = SG_PIXELFORMAT_R32UI,
			},
		},
		.sample_count = 1,
	});
	edgepip = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.buffers = {
				[0] {
					.stride = 2 * sizeof(float),
				},
				[1] {
					.stride = sizeof(GLEdge),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[0] {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 0,
				},
				[1] {
					.offset = offsetof(GLEdge, pos1),
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 1,
				},
				[2] {
					.offset = offsetof(GLEdge, pos2),
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 1,
				},
				[3] {
					.offset = offsetof(GLEdge, col),
					.format = SG_VERTEXFORMAT_FLOAT3,
					.buffer_index = 1,
				},
				[4] {
					.offset = offsetof(GLEdge, idx),
					.format = SG_VERTEXFORMAT_UINT,
					.buffer_index = 1,
				},
			}
		},
		.shader = edgesh,
		.primitive_type = SG_PRIMITIVETYPE_LINES,
		.index_type = SG_INDEXTYPE_UINT16,
		.depth = {
			.pixel_format = SG_PIXELFORMAT_DEPTH,
			.compare = SG_COMPAREFUNC_LESS_EQUAL,
			.write_enabled = true,
		},
		.color_count = 2,
		.colors = {
			[0] {
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
			[1] {
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
				[0] {
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
	snk_setup(&(snk_desc_t){
		.dpi_scale = sapp_dpi_scale(),
		.logger.func = slog_func,
	});
	setnktheme();
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
	if((drawc = chancreate(sizeof(ulong), 64)) == nil)
		sysfatal("initsysdraw: chancreate");
}

void
evloop(void)
{
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
