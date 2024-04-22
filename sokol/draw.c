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
#include "sokol_gfx_ext.h"
#include "lib/glfw_glue.h"

struct Color{
	u32int col;
	float r;
	float g;
	float b;
};

/* FIXME:
 * - refactor rend and draw
 *	and geom; use macros ± inline shit like handmade math and also hmm; and view
 * - debug node rotation → requires refactoring rend/draw
 *	using the scale param is also wrong anyway
 *	rend angle calculation is just wrong
 * - debug scaling
 */
/* FIXME: blackjack and hookers:
 * - resize: resize images etc as well, otherwise it just stretches shit; push up
 *	default window size + clamp to screen size, or always set to screen size, or
 *	add a flag, etc.
 * - make quads thick lines? generalize to edges? maybe that way we can shape
 *	algorithmically nodes and edges both
 * - we want 60fps → draw, render and flush together must be below 16.67ms
 * - shader benchmarking? flush is fast but what happens on the gpu side? why
 *	is it not smooth even though flush is fast? because of rend stalling mouse
 *	events? is it even on the same thread? lockgraphs?
 *	=> draw thread calls draw, then rerender, then flushes; both draw and rerender
 *	become expensive; improvements: pass by ref, don't use geom, use floats, hmm
 *	simplify rend; make sure it doesn't modify state that others use and maybe
 *	move to its own thread as well, it should really only set angle...
 * - bezier shaders + GLEdge improvements
 * - basic cimgui
 * - start/stopdrawclock (larger draw buffer than screen + redraw when necessary?)
 *	redraw only, not re-render etc
 * - extreme zoom out: make sure stuff is still visible (as 1px dots or lines?)
 * - benchmark: w/ rotate through hmm; no transparency
 * - compiled shaders: better performance? just push source and compiled
 *   ones to the repo, use sokol-tools
 * - pragma once?
 * - bonus for navigation: rotate in (any?) axis, experiment with ui
 * - refactor rend, draw to use the vertex buffers, mvp and transfo matrices (ofc plan9
 *   would render quads directly instead of triangles, etc.)
 *   use floats, etc.
 *	 draw/, plan9/: remove all the geom.c bullshit, huge slowdown
 * - decouple input from rendering
 * - draw labels, arrows
 * - another layer: arrange nodes and edges in a grid matrix => easy coarsening based on position + chunking, occlusion maybe
 *	unnecessary for drawing, gl can do it on its own via the depth buffer
 *	SO: geometry + compute shaders for layouting
 * - use cimgui (see https://github.com/peko/cimgui-c-example for cimgui+glfw) and glfw/imgui-glfw.cc
 * - beziers, thickness: https://thebookofshaders.com/05/
 * - get rid of unnecessary glue code (that isn't even updated)
 * - extend for 3d vis and navigation, later port to plan9
 * - 3d layout algo on top of this, would be nice to have by may
 *	zoom will just become z-panning
 *	depth tricks would be employed only in non-3d layouts
 */

#define FNodesz	((float)Nodesz)
#define Nodethiccc	(FNodesz/Ptsz)
#define	FOV	60.0f

typedef struct Camera Camera;
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

/* FIXME: sokol/ui? */
struct Camera{
	float ar;
	HMM_Vec3 eye;
	HMM_Vec3 center;
	HMM_Vec3 Δeye;
	HMM_Vec3 up;
	HMM_Mat4 mvp;
};
static Camera cam;

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
drawline(Quad q, double w, int emph, s32int idx, Color *c)
{
	GLEdge e = {
		.pos1 = {
			.X = q.o.x,
			.Y = q.o.y,
		},
		.pos2 = {
			.X = q.o.x + q.v.x,
			.Y = q.o.y + q.v.y,
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
drawbezier(Quad q, double w, s32int idx, Color *c)
{
	// FIXME
	return drawline(q, w, 0, idx, c);
}

/* FIXME: plan9: don't use geom or pass by value; refactor draw, rend; use hmm */
/* FIXME: we need untransformed shapes, where plan9 needs the opposite; fix this */
/* FIXME: q1 and q2 are meant for a series of lines and are invalid rectangles */
int
drawquad(Quad q1, Quad q2, Quad q, double θ, s32int idx, Color *c)
{
	GLNode v = {
		.pos = {
			.X = q.o.x+q.v.x,
			.Y = q.o.y+q.v.y,
		},
		.col = {
			.X = c->r,
			.Y = c->g,
			.Z = c->b,
		},
		.θ = θ,
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

static void
quitdraw(void)
{
	// FIXME: free shit or who cares?
	sg_shutdown();
	glfwTerminate();
}

static void
updatecam(void)
{
	HMM_Mat4 vw, proj;

	cam.Δeye = HMM_SubV3(cam.eye, cam.center);
	cam.ar = view.dim.v.x / view.dim.v.y;
	proj = HMM_Perspective_RH_NO(60.0f, cam.ar, 0.01f, 1000.0f);
	vw = HMM_LookAt_RH(cam.eye, cam.center, cam.up);
	cam.mvp = HMM_MulM4(proj, vw);
}

void
zoomdraw(float Δ)
{
	cam.eye = HMM_SubV3(cam.eye, HMM_MulV3F(cam.Δeye, Δ));
	updatecam();
}

void
pandraw(float Δx, float Δy)
{
	float len;
	HMM_Vec3 dy, dx, c;

	Δx /= view.dim.v.x;
	Δy /= view.dim.v.y;
	cam.center.X += Δx * 2.0f * cam.Δeye.Z * cam.ar * tan(FOV / 2.0f);
	cam.center.Y -= Δy * 2.0f * cam.Δeye.Z * tan(FOV / 2.0f);
	cam.eye.X = cam.center.X;
	cam.eye.Y = cam.center.Y;
	updatecam();
}

static void
flush(void)
{
	ssize n;
	Params p;
	sg_buffer_desc d;

	p = (Params){
		.mvp = cam.mvp,
		.scale = HMM_V2(FNodesz, FNodesz),
	};

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
	Clk clk = {.lab = "flush"}, fclk = {.lab = "frame"};

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
			CLK0(fclk);
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
			CLK1(fclk);
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
	/* FIXME: if initial window is too small this will be triggered */
	/* FIXME: unhandled */
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
		.vs.source =
			"#version 330\n"
			"//precision mediump float;\n"
			"precision lowp float;\n"
			"uniform mat4 mvp;\n"
			"uniform vec2 s;\n"
			"layout(location=0) in vec2 geom;\n"
			"layout(location=1) in vec2 pos;\n"
			"layout(location=2) in vec3 col0;\n"
			"layout(location=3) in float theta;\n"
			"layout(location=4) in uint idx0;\n"
			"out vec3 col;\n"
			"flat out uint idx;\n"
			"vec2 rotatez(vec2 v, float angle){\n"
			"  float s = sin(angle);\n"
			"  float c = cos(angle);\n"
			"  //return mat3(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0) * v;\n"
			"  return mat2(c, -s, s, c) * v;\n"
			"}\n"
			"void main() {\n"
			"  float z = gl_InstanceID * 0.000001;\n"
			"  gl_Position = mvp * vec4((rotatez(geom, theta) + pos) / s, z, 1.0);\n"
			"  col = col0;\n"
			"  idx = idx0;\n"
			"  //idx = uint(gl_InstanceID);\n"
			"}\n",
		.fs.source =
			"#version 330\n"
			"in vec3 col;\n"
			"flat in uint idx;\n"
			"layout(location=0) out vec4 c0;\n"
			"layout(location=1) out uint c1;\n"
			"void main() {\n"
			"  c0 = vec4(col, 0.8);\n"
			"  c1 = idx;\n"
			"}\n",
	});
	sg_shader edgesh = sg_make_shader(&(sg_shader_desc){
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
			"precision lowp float;\n"
			"uniform mat4 mvp;\n"
			"uniform vec2 s;\n"
			"layout(location=0) in vec2 v;\n"
			"layout(location=1) in vec2 p1;\n"
			"layout(location=2) in vec2 p2;\n"
			"layout(location=3) in vec3 col0;\n"
			"layout(location=4) in uint idx0;\n"
			"out vec3 col;\n"
			"flat out uint idx;\n"
			"void main() {\n"
			"  vec2 p = gl_VertexID == 0 ? p1 : p2;\n"
			"  float z = gl_InstanceID * -0.000001;\n"
			"  gl_Position = mvp * vec4(p / s, z, 1.0);\n"
			"  col = col0;\n"
			"  idx = idx0;\n"
			"}\n",
		.fs.source =
			"#version 330\n"
			"in vec3 col;\n"
			"flat in uint idx;\n"
			"layout(location=0) out vec4 c0;\n"
			"layout(location=1) out uint c1;\n"
			"void main() {\n"
			"  c0 = vec4(col, 0.25);\n"
			"  c1 = idx;\n"
			"}\n",
	});

	/* multi-target rendering targets, without multisampling */
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
		.pixel_format = SG_PIXELFORMAT_R32UI,
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
		.fs.images[0] = fb,
		.fs.samplers[0] = smp,
	};
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
	cam.eye = HMM_V3(0.0f, 0.0f, 10.0f);
	cam.center = HMM_V3(0.0f, 0.0f, 0.0f);
	cam.up = HMM_V3(0.0f, 1.0f, 0.0f);
	updatecam();
	//atexit(quitdraw);
}
