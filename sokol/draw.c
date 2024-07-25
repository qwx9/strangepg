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
extern char *bezier_vertsh, *bezier_tcssh, *bezier_tessh, *bezier_geomsh, *bezier_fragsh;

void	event(const sapp_event*);
void	_drawui(struct nk_context*);
void	initnk(void);

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
 * - start/stopdrawclock (larger draw buffer than screen + redraw when necessary?)
 *	redraw only, not re-render etc; or rather toggle continuous redraw?
 *	does performance/load suffer?
 * - draw labels, arrows
 * - extend for 3d vis and navigation, later port to plan9
 */

#define	FNodesz	((float)Nodesz)
#define	Nodethiccc	(FNodesz/Ptsz)
#define	FOV	60.0f

typedef struct Params Params;
typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;
typedef struct Shader Shader;

/* FIXME: provide aligned data as per std140 instead of this shit */
#define	hjdicks __attribute((packed))
struct hjdicks Params{
	HMM_Mat4 mvp;
	HMM_Vec2 scale;	/* FIXME */
};
struct hjdicks GLNode{
	HMM_Vec2 pos;
	HMM_Vec2 dir;
	HMM_Vec4 col;
};
struct hjdicks GLEdge{
	HMM_Vec2 pos1;
	HMM_Vec2 pos2;
	HMM_Vec4 col;
};
static GLNode *nodev;
static GLEdge *edgev;
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

static Channel *drawc;

static GLuint stupid, stupidvao, stupidvbo;

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

ioff
mousepick(int x, int y)
{
	ioff i;

	if((i = sg_query_image_pixel(x, y, pickfb)) == 0)
		return -1;
	return i;
}

void
drawselected(void)
{
	ioff id;
	static ioff osel = -1;

	if(selected == -1 || selected == osel)
		return;
	else if((selected & (1<<31)) == 0){
		id = (uint)selected;
		pushcmd("print \"selected node \" %d, lnode[%d]", id, id);
	}else{
		id = (uint)selected & ~(1<<31);
		pushcmd("print \"selected edge \"  %d, ledge[%d]", id, id);
	}
	warn("sel %zd osel %zd\n", selected, osel);
	osel = selected;
}

int
drawlabel(Node *, ioff, Color *)
{
	return 0;
}

/* FIXME: this would mess up indexing */
int
drawline(Vertex a, Vertex b, double w, int emph, ioff idx, Color *c)
{
	return 0;

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
			.W = 0.6f,
		},
	};

	dypush(edgev, e);
	return 0;
}

int
drawbezier(Vertex a, Vertex b, ioff idx, Color *c)
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
			.W = 0.2f,
		},
	};

	dypush(edgev, e);
	return 0;
}

int
drawquad(Vertex pos, Vertex dir, ioff idx, Color *c)
{
	GLNode v = {
		.pos = {
			.X = pos.x,
			.Y = pos.y,
		},
		.dir = {
			.X = dir.x,
			.Y = dir.y,
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
			.W = 0.6f,
		},
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
	ioff n;

	n = dylen(edgev);
	/*
	sg_buffer_desc d;
	d = sg_query_buffer_desc(edgebind.vertex_buffers[1]);
	if(d.size / sizeof *edgev < n){
		sg_destroy_buffer(edgebind.vertex_buffers[1]);
		edgebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *edgev,
			.usage = SG_USAGE_STREAM,
		});
	}
	*/
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
	ioff n;

	n = dylen(nodev);
	/*
	sg_buffer_desc d;
	d = sg_query_buffer_desc(nodebind.vertex_buffers[1]);
	if(d.size / sizeof *nodev < n){
		warn("nodes: yep %x < %x\n", d.size, n);
		sg_destroy_buffer(nodebind.vertex_buffers[1]);
		nodebind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
			.size = n * sizeof *nodev,
			.usage = SG_USAGE_STREAM,
		});
	}
	*/
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
renderstupid(Params p)
{
	GLuint pmvp;

	sg_begin_pass(&(sg_pass){
		.action = offscreen_pass_action2,
		.attachments = offscreen_attachments,
	});
	glUseProgram(stupid);
	pmvp = glGetUniformLocation(stupid, "mvp");
	glUniformMatrix4fv(pmvp, 1, GL_FALSE, &mvp);
	glBindVertexArray(stupidvao);
	glDrawArrays(GL_PATCHES, 0, 4);
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
	//renderstupid(p);
	renderscreen(p);
	sg_commit();
}

void
startdrawclock(void)
{
	reqdraw(Reqredraw);
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

static int
loadshader(GLuint pgm, GLenum type, char **src, char *name)
{
	GLint r;
	GLuint s;

	s = glCreateShader(type);
	glShaderSource(s, 1, src, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &r);
	if(!r){
		werrstr("shader %s: compilation failed", name);
		return -1;
	}
	glAttachShader(pgm, s);
	return 0;
}

static int
linkshader(GLuint pgm)
{
	GLint r;

	glLinkProgram(pgm);
	glGetProgramiv(pgm, GL_LINK_STATUS, &r);
	if(!r){
		werrstr("shader linking failed");
		return -1;
	}
	return 0;
}

static int
newshader(Shader *s, GLuint *pgm)
{
	GLuint p;

	if(s == nil){
		warn("newshader: nothing");
		return -1;
	}
	p = glCreateProgram();
	for(; s->src!=nil; s++)
		if(loadshader(p, s->type, s->src, s->name) < 0)
			return -1;
	if(linkshader(p) < 0)
		return -1;
	*pgm = p;
	return 0;
}

static void
initbullshit(void)
{
	static GLfloat cp[][2] = {
		{-10.5f, -10.5f},
		{-10.5f,  10.5f},
		{ 10.5f,  10.5f},
		{ 10.5f, -10.5f},
	};
	static Shader sh[] = {
		{"bezier vert", GL_VERTEX_SHADER, &bezier_vertsh},
		{"bezier tcs", GL_TESS_CONTROL_SHADER, &bezier_tcssh},
		{"bezier tes", GL_TESS_EVALUATION_SHADER, &bezier_tessh},
		{"bezier geom", GL_GEOMETRY_SHADER, &bezier_geomsh},
		{"bezier frag", GL_FRAGMENT_SHADER, &bezier_fragsh},
		{nil, -1, nil},
	};

	if(newshader(sh, &stupid) < 0)
		sysfatal("initgl: bezier shader: %s", error());
	glGenVertexArrays(1, &stupidvao);
	glBindVertexArray(stupidvao);
	glGenBuffers(1, &stupidvbo);
	glBindBuffer(GL_ARRAY_BUFFER, stupidvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof cp, cp, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
}

static void
initgl(void)
{
	Color *c;
	Graph *g;

	g = graphs;
	/* geometry */
	float nodevert[] = {
		-FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, -Nodethiccc/2.0f,
		-FNodesz/2.0f, -Nodethiccc/2.0f,
	};
	u16int nodeidx[] = {
		0, 2, 1,	// first triangle
		0, 3, 2,	// second triangle
	};
	float edgevert[] = { -1, 1 };
	u16int edgeidx[] = { 0, 1 };

	/* bindings for instancing + offscreen rendering: vertex buffer slot 1
	 * is used for instance data */
	nodebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(nodevert),
			}),
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = dylen(g->nodes) * sizeof(GLNode),
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
				.size = dylen(g->edges) * sizeof(GLEdge),
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
					.offset = offsetof(GLNode, dir),
					.format = SG_VERTEXFORMAT_FLOAT2,
					.buffer_index = 1,
				},
				[3] {
					.offset = offsetof(GLNode, col),
					.format = SG_VERTEXFORMAT_FLOAT4,
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
					.stride = sizeof(float),
				},
				[1] {
					.stride = sizeof(GLEdge),
					.step_func = SG_VERTEXSTEP_PER_INSTANCE
				},
			},
			.attrs = {
				[0] {
					.offset = 0,
					.format = SG_VERTEXFORMAT_FLOAT,
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
	initbullshit();
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
	if((drawc = chancreate(sizeof(ulong), 64)) == nil)
		sysfatal("initsysdraw: chancreate");
}

void
evloop(void)
{
	/* wait until at least one file asks for a redraw */
	while(recvul(drawc) != Reqredraw)
		;
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
