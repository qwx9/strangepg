#include "strpg.h"
#include <signal.h>
#include "drw.h"
#include "threads.h"
#include "flextgl/flextGL.h"
#define HANDMADE_MATH_IMPLEMENTATION
//#define HANDMADE_MATH_NO_SSE
#include "lib/HandmadeMath.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
//#define SOKOL_DEBUG
#include "lib/sokol_gfx.h"
#include "lib/sokol_log.h"
#include "lib/glfw_glue.h"

struct Color{
	u32int col;
	float r;
	float g;
	float b;
};

/* FIXME: todo:
 * - edge pipeline and shader
 * - line drawing; see sgl-lines-sapp, primtypes-sapp; draw straight lines
 */

/* FIXME: blackjack and hookers:
 * - compiled shaders: better performance? just push source and compiled
 *   ones to the repo, use sokol-tools
 * - pragma once?
 * - skip offscreen rendering for onscreen buffer? can we draw simultaneously to
 *   display and offscreen?
 * - bonus for navigation: rotate in (any?) axis, experiment with ui
 * - refactor rend, draw to use the vertex buffers, mvp and transfo matrices (ofc plan9
 *   would render quads directly instead of triangles, etc.)
 *   use floats, etc.
 *	 draw/, plan9/: remove all the geom.c bullshit, huge slowdown
 * - free and realloc attachments on resize? (since we're using the screen size)
 *	would be nice to avoid, but for now just do it always or never
 *  doesn't really change that much either; perhaps we could just instead render onto
 *	fixed size buffer and call it a day
 * - use mvp for zoom in/zoom out (eye z coord) AND panning (center or xy coords) AND rotation (up vector)
 * - don't output stuff that falls outside view area wrt mvp?
 * - mouse picking; see inject-glfw.c
 * - decouple input from rendering
 * - draw labels, arrows
 * - another layer: arrange nodes and edges in a grid matrix => easy coarsening based on position + chunking, occlusion maybe
 * - use cimgui (see https://github.com/peko/cimgui-c-example for cimgui+glfw) and glfw/imgui-glfw.cc
 * - beziers, thickness: https://thebookofshaders.com/05/
 * - get rid of unnecessary glue code (that isn't even updated)
 * - extend for 3d vis and navigation, later port to plan9
 * - 3d layout algo on top of this, would be nice to have by may
 */

#define FNodesz	((float)Nodesz)
#define Nodethiccc	(FNodesz/Ptsz)

typedef struct Params Params;
typedef struct GLNode GLNode;
typedef struct GLEdge GLEdge;

#define hjdicks __attribute((packed))
struct hjdicks Params{
	HMM_Mat4 mvp;
	HMM_Vec2 scale;	/* FIXME */
};
struct hjdicks GLNode{
	HMM_Vec2 pos;
	HMM_Vec3 col;
	/* no SG_VERTEXFORMAT_INT yet */
	union {
		int i;
		float f;
	} idx;
	float θ;
};
struct hjdicks GLEdge{
	HMM_Vec2 pos[2];
	HMM_Vec3 col;
	/* no SG_VERTEXFORMAT_INT yet */
	union {
		int i;
		float f;
	} idx;
};
static GLNode *nodev;
static GLEdge *edgev;

static sg_pass_action offscreen_pass_action;
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

int
scrobj(Vertex)
{
	return -1;
}

void
showobj(Obj *)
{
}

int
drawlabel(Node *, Quad, Quad, Quad, vlong, Color *)
{
	return 0;
}

int
drawline(Quad q, double w, int emph, int idx, Color *c)
{
	GLEdge e = {
		.pos = {
			[0] {
				.X = q.o.x,
				.Y = q.o.y,
			},
			[1] {
				.X = q.o.x + q.v.x,
				.Y = q.o.y + q.v.y,
			},
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
		},
		.idx.i = idx,
	};

	dypush(edgev, e);
	return 0;
}

int
drawbezier(Quad q, double w, int idx, Color *c)
{
	// FIXME
	return drawline(q, w, 0, idx, c);
}

/* FIXME: we need untransformed shapes, where plan9 needs the opposite; fix this */
/* FIXME: q1 and q2 are meant for a series of lines and are invalid rectangles */
int
drawquad(Quad q1, Quad q2, Quad q, double θ, int idx, Color *c)
{
	GLNode v = {
		.pos = {
			.X = q.o.x,
			.Y = q.o.y,
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
		},
		.θ = θ + PI/4,	// FIXME
		.idx.i = idx,
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

static void
quitdraw(void)
{
	// FIXME: free shit or who cares?
	sg_shutdown();
	glfwTerminate();
}

static 
void flush(void)
{
	ssize n;
	float w, h;
	Params p;
	HMM_Mat4 proj, vw;
	HMM_Vec2 pan;

	w = view.dim.v.x;
	h = view.dim.v.y;
	// FIXME: not quite correct, pan coords are in pixel space, not using this right
	pan = HMM_V2(-(float)view.pan.x / w, (float)view.pan.y / h);
	proj = HMM_Perspective_RH_NO(60.0f, w/h, 0.1f, 100.0f);
	vw = HMM_LookAt_RH(HMM_V3(pan.X, pan.Y, 10/view.zoom), HMM_V3(pan.X, pan.Y, 0.0f), HMM_V3(0.0f, 1.0f, 0.0f));
	p = (Params){
		.mvp = HMM_MulM4(proj, vw),
		.scale = HMM_V2(FNodesz, FNodesz),
	};
	n = dylen(nodev);
	sg_buffer_desc d = sg_query_buffer_desc(nodebind.vertex_buffers[1]);
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
		.action = offscreen_pass_action,
		.attachments = offscreen_attachments,
	});
	sg_apply_pipeline(nodepip);
	sg_apply_bindings(&nodebind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(p));
	sg_draw(0, 6, n);
	sg_end_pass();
	sg_begin_pass(&(sg_pass){
		.action = default_pass_action,
		.swapchain = glfw_swapchain(),
	});
	sg_apply_pipeline(fsq_pip);
	sg_apply_bindings(&fsq_bind);
	sg_draw(0, 4, 1);
	sg_end_pass();
	sg_commit();
	glfwSwapBuffers(glfw_window());
}

void
startdrawclock(void)
{
}

void
stopdrawclock(void)
{
}

void
evloop(void)
{
	int req;
	static Clk clk = {.lab = "flush"};

	/* FIXME: if we want the plan9 lilu dallas mooltithreading, this needs
	 *	a different design; on plan9 input and drawing are decoupled,
	 *	whereas here the same thread processes both; either figure out a
	 *	way to decouple them again (ticker, multithread glfw, locks) or
	 *	do it the old fashion way (poll for events, process, sleep); the
	 *	latter sucks because slow redraw throttles input, and we don't want
	 *	that */
	req = 0;
	while (!glfwWindowShouldClose(glfw_window())) {
		if((req = nbrecvul(drawc)) != 0){
			/* Reqresetdraw for resizing: handled by callback */
			if((req & Reqresetui) != 0)
				resetui(1);
			if((req & Reqrefresh) != 0 && !norefresh || (req & Reqrender) != 0){
				if(!rerender(req & Reqrender))
					stopdrawclock();
			}
			// FIXME: actually implement shallow redraw, etc.
			if(req != Reqshallowdraw){
				redraw();
				if(dylen(nodev) == 0)
					continue;
				CLK0(clk);
				flush();
				CLK1(clk);
			}
		}
		//if(drawstep)...?
		reqdraw(Reqrefresh);	/* FIXME */
		glfwWaitEvents();		/* FIXME: kind of redundant with nbrecv */
	}
	quitdraw();
}

void
reqdraw(int r)
{
	static ulong f;

	f |= r;
	if(nbsendul(drawc, f) != 0){
		glfwPostEmptyEvent();
		f = 0;
	}
}

static void
glresize(GLFWwindow *, int w, int h)
{
	view.dim.v = Vec2(w, h);
	resetui(1);
}

static void
glerr(int err, const char *desc)
{
	warn("glerr %d: %s\n", err, desc);
}

/* FIXME: error handling + error strings */
static void
initgl(void)
{
	Color *c;

	glfw_init(&(glfw_desc_t){
		.title = "strpg",
		.width = view.dim.v.x,
		.height = view.dim.v.y,
		.sample_count = 4,
	});
	glfwSetErrorCallback(glerr);
	if(!flextInit(glfw_window()))
		sysfatal("flextGL: failed to initialize: %s", error());
	sg_setup(&(sg_desc){
		.environment = glfw_environment(),
		.logger.func = slog_func,
	});
	assert(sg_isvalid());

	/* GEOMETRY */
	float vertices[] = {
		-FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, +Nodethiccc/2.0f,
		+FNodesz/2.0f, -Nodethiccc/2.0f,
		-FNodesz/2.0f, -Nodethiccc/2.0f,
	};
	u16int indices[] = {
		0, 1, 2,	// first triangle
		0, 2, 3,	// second triangle
	};

	/* INSTANCING */
	// resource bindings for offscreen rendering
	// note how the instance-data buffer goes into vertex buffer slot 1
	nodebind = (sg_bindings){
		.vertex_buffers = {
			[0] = sg_make_buffer(&(sg_buffer_desc){
				.data = SG_RANGE(vertices),
			}),
			// empty, dynamic instance-data vertex buffer (goes into vertex buffer bind slot 1)
			[1] = sg_make_buffer(&(sg_buffer_desc){
				.size = 1024 * sizeof(GLNode),
				.usage = SG_USAGE_STREAM,
			}),
		},
		.index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(indices),
		}),
	};
	// create a shader
	sg_shader nodesh = sg_make_shader(&(sg_shader_desc){
		.vs.uniform_blocks[0] = {
			.size = sizeof(Params),
			.uniforms = {
				[0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
				[1] = { .name="s", .type=SG_UNIFORMTYPE_FLOAT2 },
			},
		},
		.vs.source =
			"#version 330\n"
			"//precision mediump float;\n"
			"//precision lowp float;\n"
			"uniform mat4 mvp;\n"
			"uniform vec2 s;\n"
			"layout(location=0) in vec2 geom;\n"
			"layout(location=1) in vec2 pos;\n"
			"layout(location=2) in vec3 col0;\n"
			"layout(location=3) in float idx0;\n"
			"layout(location=4) in float theta;\n"
			"out vec3 col;\n"
			"flat out int idx;\n"
			"vec2 rotatez(vec2 v, float angle){\n"
			"  float s = sin(angle);\n"
			"  float c = cos(angle);\n"
			"  //return mat3(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0) * v;\n"
			"  return mat2(c, -s, s, c) * v;\n"
			"}\n"
			"void main() {\n"
			"  //float i = gl_InstanceID;\n"
			"  gl_Position = mvp * vec4((rotatez(geom, theta) + pos) / s, 0.0, 1.0);\n"
			"  //gl_Position = mvp * vec4((geom + pos) / s, 0.0, 1.0);\n"
			"  col = col0;\n"
			"  idx = int(idx0);\n"
			"}\n",
		.fs.source =
			"#version 330\n"
			"in vec3 col;\n"
			"flat in int idx;\n"
			"layout(location=0) out vec4 c0;\n"
			"//layout(location=1) out int c1;\n"
			"layout(location=1) out vec4 c1;\n"
			"void main() {\n"
			"  c0 = vec4(col, 0.8);\n"
			"  c1 = vec4(col, 1.0);\n"
			"  //c1 = int(idx);\n"
			"}\n",
	});

/* FIXME: is depth image/stencil and sampler always required? */
	/* OFFSCREEN MULTI TARGET RENDERING (no msaa) */
	const int msaa_samples = 1;
	fb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = glfw_width(),
		.height = glfw_height(),
		.sample_count = 1,
	});
	pickfb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = glfw_width(),
		.height = glfw_height(),
		//.pixel_format = SG_PIXELFORMAT_R32UI,
		.sample_count = 1,
	});
	zfb = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = glfw_width(),
		.height = glfw_height(),
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
	// a matching pass action with clear colors
	c = color(theme[Cbg]);
	offscreen_pass_action = (sg_pass_action){
		.colors = {
			[0] {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { c->r, c->g, c->b, 1.0f }
			},
			[1] {
				.load_action = SG_LOADACTION_CLEAR,
				.store_action = SG_STOREACTION_DONTCARE,
				.clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
			},
		}
	};
	// pipeline state object, note the vertex layout definition
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
					.offset = offsetof(GLNode, idx),
					.format = SG_VERTEXFORMAT_FLOAT,
					.buffer_index = 1,
				},
				[4] {
					.offset = offsetof(GLNode, θ),
					.format = SG_VERTEXFORMAT_FLOAT,
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
		.cull_mode = SG_CULLMODE_BACK,
		.color_count = 2,
		.sample_count = msaa_samples
	});

	/* DISPLAY RENDERING */
	// a vertex buffer to render a fullscreen rectangle
	float quad_vertices[] = {
		0.0f, 0.0f, 
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};
	sg_buffer quad_vbuf = sg_make_buffer(&(sg_buffer_desc){
		.data = SG_RANGE(quad_vertices)
	});
	// a sampler with linear filtering and clamp-to-edge
	sg_sampler smp = sg_make_sampler(&(sg_sampler_desc){
		.min_filter = SG_FILTER_LINEAR,
		.mag_filter = SG_FILTER_LINEAR,
		.wrap_u = SG_WRAP_CLAMP_TO_EDGE,
		.wrap_v = SG_WRAP_CLAMP_TO_EDGE,
	});
	// resource bindings to render the fullscreen quad
	fsq_bind = (sg_bindings){
		.vertex_buffers[0] = quad_vbuf,
		.fs.images[0] = fb,
		.fs.samplers[0] = smp,
	};
	// a shader to render a fullscreen rectangle
	sg_shader fsq_shd = sg_make_shader(&(sg_shader_desc){
		.vs = {
			.source =
				"#version 330\n"
				"layout(location=0) in vec2 pos;\n"
				"out vec2 uv0;\n"
				"void main() {\n"
				"  gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);\n"
				"  uv0 = pos;\n"
				"}\n",
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
			.source =
				"#version 330\n"
				"uniform sampler2D tex0;\n"
				"in vec2 uv0;\n"
				"out vec4 frag_color;\n"
				"void main() {\n"
				"  vec3 c0 = texture(tex0, uv0).xyz;\n"
				"  frag_color = vec4(c0, 1.0);\n"
				"}\n"
		}
	});
	// the pipeline object for the fullscreen rectangle
	fsq_pip = sg_make_pipeline(&(sg_pipeline_desc){
		.layout = {
			.attrs[0].format=SG_VERTEXFORMAT_FLOAT2
		},
		.shader = fsq_shd,
		.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP
	});
	// default pass action, no clear needed, since whole screen is overwritten
	default_pass_action = (sg_pass_action){
		.colors[0].load_action = SG_LOADACTION_DONTCARE,
		.depth.load_action = SG_LOADACTION_DONTCARE,
		.stencil.load_action = SG_LOADACTION_DONTCARE
	};
}

/* FIXME: error handling */
void
initsysdraw(void)
{
	view.dim.o = ZV;	/* FIXME: horrible */
	view.dim.v = Vec2(Vdefw, Vdefh);
	initgl();
	glfwSetFramebufferSizeCallback(glfw_window(), glresize);
	/* FIXME: the reason why this can't be 0 is because of the fucking
	 * input handling being synchronous */
	if((drawc = chancreate(sizeof(ulong), 64)) == nil)
		sysfatal("initsysdraw: chancreate");
	//atexit(quitdraw);
}
