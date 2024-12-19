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

SOKOL_GFX_API_DECL uint32_t sgx_query_image_pixel(int x, int y, sg_image img_id);
SOKOL_GFX_API_DECL void sgx_query_image_pixels(sg_image img_id, void* pixels);
SOKOL_GFX_API_DECL void sgx_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size);
SOKOL_GFX_API_DECL void sgx_enable_debug_log(void);

#endif // SOKOL_GFX_EXT_INCLUDED

#ifdef SOKOL_GFX_EXT_IMPL
#ifndef SOKOL_GFX_EXT_IMPL_INCLUDED
#define SOKOL_GFX_EXT_IMPL_INCLUDED

#ifndef SOKOL_GFX_IMPL_INCLUDED
#error "Please include sokol_gfx.h implementation before sokol_gp.h implementation"
#endif

#if defined(_SOKOL_ANY_GL)

static void _sgx_gl_query_image_pixels(_sg_image_t* img, void* pixels) {
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

static void _sgx_gl_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
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

static uint32_t _sgx_gl_query_image_pixel(int x, int y, _sg_image_t* img) {
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

static void _sgx_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userparam) {
	fprintf(stderr, "[%#x:%#x] %s GL_CALLBACK: %s\n",
		severity, type, type == GL_DEBUG_TYPE_ERROR ? "GL ERROR" : "", message);
}

#elif defined(SOKOL_D3D11)

static inline void _sgx_d3d11_Texture2D_GetDesc(ID3D11Texture2D* self, D3D11_TEXTURE2D_DESC* pDesc) {
    self->lpVtbl->GetDesc(self, pDesc);
}

static inline void _sgx_d3d11_SamplerState_GetDesc(ID3D11SamplerState* self, D3D11_SAMPLER_DESC* pDesc) {
    self->lpVtbl->GetDesc(self, pDesc);
}

static inline void _sgx_d3d11_CopySubresourceRegion(ID3D11DeviceContext* self, ID3D11Resource *pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource *pSrcResource, UINT SrcSubresource, const D3D11_BOX *pSrcBox) {
    self->lpVtbl->CopySubresourceRegion(self, pDstResource, DstSubresource, DstX, DstY, DstZ, pSrcResource, SrcSubresource, pSrcBox);
}

static inline void _sgx_d3d11_OMGetRenderTargets(ID3D11DeviceContext* self, UINT NumViews, ID3D11RenderTargetView **ppRenderTargetViews, ID3D11DepthStencilView **ppDepthStencilView) {
    self->lpVtbl->OMGetRenderTargets(self, NumViews, ppRenderTargetViews, ppDepthStencilView);
}

static inline void _sgx_d3d11_RenderTargetView_GetResource(ID3D11RenderTargetView* self, ID3D11Resource** ppResource) {
    self->lpVtbl->GetResource(self, ppResource);
}

static void _sgx_d3d11_query_image_pixels(_sg_image_t* img, void* pixels) {
    SOKOL_ASSERT(_sg.d3d11.ctx);
    SOKOL_ASSERT(img->d3d11.tex2d);
    HRESULT hr;
    _SOKOL_UNUSED(hr);

    // create staging texture
    ID3D11Texture2D* staging_tex = NULL;
    D3D11_TEXTURE2D_DESC staging_desc = {
        .Width = (UINT)img->cmn.width,
        .Height = (UINT)img->cmn.height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = img->d3d11.format,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Usage = D3D11_USAGE_STAGING,
        .BindFlags = 0,
        .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
        .MiscFlags = 0
    };
    hr = _sg_d3d11_CreateTexture2D(_sg.d3d11.dev, &staging_desc, NULL, &staging_tex);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy pixels to staging texture
    _sgx_d3d11_CopySubresourceRegion(_sg.d3d11.ctx,
        (ID3D11Resource*)staging_tex,
        0, 0, 0, 0,
        (ID3D11Resource*)img->d3d11.tex2d,
        0, NULL);

    // map the staging texture's data to CPU-accessible memory
    D3D11_MAPPED_SUBRESOURCE msr = {.pData = NULL};
    hr = _sg_d3d11_Map(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0, D3D11_MAP_READ, 0, &msr);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy the data into the desired buffer, converting pixels to the desired format at the same time
    memcpy(pixels, msr.pData, img->cmn.width * img->cmn.height * 4);

    // unmap the texture
    _sg_d3d11_Unmap(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0);

    if(staging_tex) _sg_d3d11_Release(staging_tex);
}

#ifdef nope
static void _sgx_d3d11_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
    // get current render target
    ID3D11RenderTargetView* render_target_view = NULL;
    _sgx_d3d11_OMGetRenderTargets(_sg.d3d11.ctx, 1, &render_target_view, NULL);

    // fallback to window render target
    if(!render_target_view)
        render_target_view = (ID3D11RenderTargetView*)_sg.d3d11.rtv_cb();
    SOKOL_ASSERT(render_target_view);

    // get the back buffer texture
    ID3D11Texture2D *back_buffer = NULL;
    _sgx_d3d11_RenderTargetView_GetResource(render_target_view, (ID3D11Resource**)&back_buffer);
    SOKOL_ASSERT(back_buffer);

    // create a staging texture to copy the screen's data to
    D3D11_TEXTURE2D_DESC staging_desc;
    _sgx_d3d11_Texture2D_GetDesc(back_buffer, &staging_desc);
    staging_desc.Width = w;
    staging_desc.Height = h;
    staging_desc.BindFlags = 0;
    staging_desc.MiscFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    ID3D11Texture2D *staging_tex = NULL;
    HRESULT hr = _sg_d3d11_CreateTexture2D(_sg.d3d11.dev, &staging_desc, NULL, &staging_tex);
    SOKOL_ASSERT(SUCCEEDED(hr));
    _SOKOL_UNUSED(hr);

    // copy the desired portion of the back buffer to the staging texture
    y = (origin_top_left ? y : (_sg.d3d11.cur_height - (y + h)));
    D3D11_BOX src_box = {
        .left = (UINT)x,
        .top = (UINT)y,
        .front = 0,
        .right = (UINT)(x + w),
        .bottom = (UINT)(y + w),
        .back = 1,
    };
    _sgx_d3d11_CopySubresourceRegion(_sg.d3d11.ctx,
        (ID3D11Resource*)staging_tex,
        0, 0, 0, 0,
        (ID3D11Resource*)back_buffer,
        0, &src_box);

    // map the staging texture's data to CPU-accessible memory
    D3D11_MAPPED_SUBRESOURCE msr = {.pData = NULL};
    hr = _sg_d3d11_Map(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0, D3D11_MAP_READ, 0, &msr);
    SOKOL_ASSERT(SUCCEEDED(hr));

    // copy the data into the desired buffer, converting pixels to the desired format at the same time
    memcpy(pixels, msr.pData, w * h * 4);

    // unmap the texture
    _sg_d3d11_Unmap(_sg.d3d11.ctx, (ID3D11Resource*)staging_tex, 0);

    if(back_buffer) _sg_d3d11_Release(back_buffer);
    if(staging_tex) _sg_d3d11_Release(staging_tex);
}
#endif

#elif defined(SOKOL_METAL)

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

static void _sgx_mtl_commit_command_buffer() {
    SOKOL_ASSERT(!_sg.cur_pass.in_pass);
    if(_sg.mtl.cmd_buffer) {
        #if defined(_SG_TARGET_MACOS)
        [_sg.mtl.uniform_buffers[_sg.mtl.cur_frame_rotate_index] didModifyRange:NSMakeRange(0, _sg.mtl.cur_ub_offset)];
        #endif
        [_sg.mtl.cmd_buffer commit];
        [_sg.mtl.cmd_buffer waitUntilCompleted];
        _sg.mtl.cmd_buffer = [_sg.mtl.cmd_queue commandBufferWithUnretainedReferences];
    }
}

static void _sgx_mtl_encode_texture_pixels(int x, int y, int w, int h, bool origin_top_left, id<MTLTexture> mtl_src_texture, void* pixels) {
    SOKOL_ASSERT(!_sg.cur_pass.in_pass);
    _sgx_mtl_commit_command_buffer();
    MTLTextureDescriptor* mtl_dst_texture_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mtl_src_texture.pixelFormat width:w height:h mipmapped:NO];
    mtl_dst_texture_desc.storageMode = MTLStorageModeManaged;
    mtl_dst_texture_desc.resourceOptions = MTLResourceStorageModeManaged;
    mtl_dst_texture_desc.usage = MTLTextureUsageShaderRead + MTLTextureUsageShaderWrite;
    id<MTLTexture> mtl_dst_texture = [mtl_src_texture.device newTextureWithDescriptor:mtl_dst_texture_desc];
    id<MTLCommandBuffer> cmd_buffer = [_sg.mtl.cmd_queue commandBuffer];
    id<MTLBlitCommandEncoder> blit_encoder = [cmd_buffer blitCommandEncoder];
    [blit_encoder copyFromTexture:mtl_src_texture
        sourceSlice:0
        sourceLevel:0
        sourceOrigin:MTLOriginMake(x,(origin_top_left ? y : (mtl_src_texture.height - (y + h))),0)
        sourceSize:MTLSizeMake(w,h,1)
        toTexture:mtl_dst_texture
        destinationSlice:0
        destinationLevel:0
        destinationOrigin:MTLOriginMake(0,0,0)
    ];
    [blit_encoder synchronizeTexture:mtl_dst_texture slice:0 level:0];
    [blit_encoder endEncoding];
    [cmd_buffer commit];
    [cmd_buffer waitUntilCompleted];
    MTLRegion mtl_region = MTLRegionMake2D(0, 0, w, h);
    [mtl_dst_texture getBytes:pixels bytesPerRow:w * 4 fromRegion:mtl_region mipmapLevel:0];
}

static void _sgx_mtl_query_image_pixels(_sg_image_t* img, void* pixels) {
    id<MTLTexture> mtl_src_texture = _sg.mtl.idpool.pool[img->mtl.tex[0]];
    _sgx_mtl_encode_texture_pixels(0, 0, mtl_src_texture.width, mtl_src_texture.height, true, mtl_src_texture, pixels);
}

static void _sgx_mtl_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
    id<CAMetalDrawable> mtl_drawable = (__bridge id<CAMetalDrawable>)sapp_metal_get_current_drawable();
    _sgx_mtl_encode_texture_pixels(x, y, w, h, origin_top_left, mtl_drawable.texture, pixels);
}

#endif

void sgx_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels, int size) {
    SOKOL_ASSERT(pixels);
    SOKOL_ASSERT(size >= w*h);
    _SOKOL_UNUSED(size);
#if defined(_SOKOL_ANY_GL)
    _sgx_gl_query_pixels(x, y, w, h, origin_top_left, pixels);
#elif defined(SOKOL_D3D11)
//    _sgx_d3d11_query_pixels(x, y, w, h, origin_top_left, pixels);
#elif defined(SOKOL_METAL)
    _sgx_mtl_query_pixels(x, y, w, h, origin_top_left, pixels);
#endif
}

void sgx_query_image_pixels(sg_image img_id, void* pixels) {
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
#if defined(_SOKOL_ANY_GL)
    _sgx_gl_query_image_pixels(img, pixels);
#elif defined(SOKOL_D3D11)
    _sgx_d3d11_query_image_pixels(img, pixels);
#elif defined(SOKOL_METAL)
    _sgx_mtl_query_image_pixels(img, pixels);
#endif
}

uint32_t sgx_query_image_pixel(int x, int y, sg_image img_id) {
    SOKOL_ASSERT(img_id.id != SG_INVALID_ID);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    SOKOL_ASSERT(img);
#if defined(_SOKOL_ANY_GL)
    return _sgx_gl_query_image_pixel(x, y, img);
#else
	return 0;
#endif
}

void sgx_enable_debug_log(void) {
#if defined(_SOKOL_ANY_GL)
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(_sgx_gl_debug_callback, 0);
#endif
}

#endif // SOKOL_GFX_EXT_IMPL_INCLUDED
#endif // SOKOL_GFX_EXT_IMPL
