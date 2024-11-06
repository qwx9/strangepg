/* based on:
 * https://github.com/edubart/sokol_gp/blob/beedb873724c53eb56fe3539882218e7a92aac80/sokol_gfx_ext.h
 * see also: https://github.com/floooh/sokol/issues/282
 */

#if defined(SOKOL_IMPL) && !defined(SOKOL_GFX_EXT_IMPL)
#define SOKOL_GFX_EXT_IMPL
#endif

#ifndef SOKOL_GFX_EXT_INCLUDED
#define SOKOL_GFX_EXT_INCLUDED

#ifndef SOKOL_GFX_INCLUDED
#error "Please include sokol_gfx.h before sokol_gfx_ext.h"
#endif

SOKOL_GFX_API_DECL uint32_t sg_query_image_pixel(int x, int y, sg_image img_id);
SOKOL_GFX_API_DECL void sg_query_image_pixels(sg_image img_id, void* pixels);
SOKOL_GFX_API_DECL void sg_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size);
SOKOL_GFX_API_DECL void sg_enable_debug_log(void);

#endif // SOKOL_GFX_EXT_INCLUDED

#ifdef SOKOL_GFX_EXT_IMPL
#ifndef SOKOL_GFX_EXT_IMPL_INCLUDED
#define SOKOL_GFX_EXT_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#if defined(_SOKOL_ANY_GL)

static void _sg_gl_query_image_pixels(_sg_image_t* img, void* pixels) {
    SOKOL_ASSERT(img->gl.target == GL_TEXTURE_2D);
    SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
#if defined(SOKOL_GLCORE)
    _sg_gl_cache_store_texture_sampler_binding(0);
    _sg_gl_cache_bind_texture_sampler(0, img->gl.target, img->gl.tex[img->cmn.active_slot], 0);
    const GLenum gl_format = _sg_gl_teximage_format(img->cmn.pixel_format);
    const GLenum gl_type = _sg_gl_teximage_type(img->cmn.pixel_format);
    glGetTexImage(img->gl.target, 0, gl_format, gl_type, pixels);
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_restore_texture_sampler_binding(0);
#else
    static GLuint newFbo = 0;
    GLuint oldFbo = 0;
    if(newFbo == 0) {
        glGenFramebuffers(1, &newFbo);
    }
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, newFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->gl.tex[img->cmn.active_slot], 0);
    glReadPixels(0, 0, img->cmn.width, img->cmn.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindFramebuffer(GL_FRAMEBUFFER, oldFbo);
    //glDeleteFramebuffers(1, &newFbo);
    _SG_GL_CHECK_ERROR();
#endif
}

static void _sg_gl_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
    SOKOL_ASSERT(pixels);
    GLuint gl_fb;
    GLint dims[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&gl_fb);
    _SG_GL_CHECK_ERROR();
    glGetIntegerv(GL_VIEWPORT, dims);
    int cur_height = dims[3];
    y = origin_top_left ? (cur_height - (y+h)) : y;
    _SG_GL_CHECK_ERROR();
#if defined(SOKOL_GLES2) // use NV extension instead
    glReadBufferNV(gl_fb == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
#else
    glReadBuffer(gl_fb == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
#endif
    _SG_GL_CHECK_ERROR();
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    _SG_GL_CHECK_ERROR();
}

static uint32_t _sg_gl_query_image_pixel(int x, int y, _sg_image_t* img) {
	union { uint8_t b[4]; uint32_t i; } u;
    GLuint oldFbo = 0;
    static GLuint newFbo = 0;
    const GLenum gl_format = _sg_gl_teximage_format(img->cmn.pixel_format);
    const GLenum gl_type = _sg_gl_teximage_type(img->cmn.pixel_format);
    if(newFbo == 0) {
        glGenFramebuffers(1, &newFbo);
    }
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, newFbo);
    y = img->cmn.height - (y+1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->gl.tex[img->cmn.active_slot], 0);
    glReadPixels(x, y, 1, 1, gl_format, gl_type, u.b);
    glBindFramebuffer(GL_FRAMEBUFFER, oldFbo);
    //glDeleteFramebuffers(1, &newFbo);
    _SG_GL_CHECK_ERROR();
	return u.i;
}

static void _sg_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userparam) {
	warn("[%#x:%#x] %s GL_CALLBACK: %s\n",
		severity, type, type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "", message);
}

#endif	// _SOKOL_ANY_GL

void sg_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size) {
    SOKOL_ASSERT(pixels);
    SOKOL_ASSERT(size >= w*h);
    _SOKOL_UNUSED(size);
#if defined(_SOKOL_ANY_GL)
    _sg_gl_query_pixels(x, y, w, h, origin_top_left, pixels);
#endif
}

void sg_query_image_pixels(sg_image img_id, void* pixels) {
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
#if defined(_SOKOL_ANY_GL)
    _sg_gl_query_image_pixels(img, pixels);
#endif
}

uint32_t sg_query_image_pixel(int x, int y, sg_image img_id) {
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
#if defined(_SOKOL_ANY_GL)
    return _sg_gl_query_image_pixel(x, y, img);
#endif
}

void sg_enable_debug_log(void) {
#if defined(_SOKOL_ANY_GL)
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(_sg_gl_debug_callback, 0);
#endif
}

#endif // SOKOL_GFX_EXT_IMPL_INCLUDED
#endif // SOKOL_GFX_EXT_IMPL
