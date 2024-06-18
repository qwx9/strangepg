#if defined(SOKOL_IMPL) && !defined(SOKOL_GFX_IMPL)
#define SOKOL_GFX_IMPL
#endif
#ifndef SOKOL_GFX_INCLUDED
/*
    sokol_gfx.h -- simple 3D API wrapper

    Project URL: https://github.com/floooh/sokol

    Example code: https://github.com/floooh/sokol-samples

    Do this:
        #define SOKOL_IMPL or
        #define SOKOL_GFX_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    In the same place define one of the following to select the rendering
    backend:
        #define SOKOL_GLCORE
        #define SOKOL_GLES3
        #define SOKOL_D3D11
        #define SOKOL_METAL
        #define SOKOL_WGPU
        #define SOKOL_DUMMY_BACKEND

    I.e. for the desktop GL it should look like this:

    #include ...
    #include ...
    #define SOKOL_IMPL
    #define SOKOL_GLCORE
    #include "sokol_gfx.h"

    The dummy backend replaces the platform-specific backend code with empty
    stub functions. This is useful for writing tests that need to run on the
    command line.

    Optionally provide the following defines with your own implementations:

    SOKOL_ASSERT(c)             - your own assert macro (default: assert(c))
    SOKOL_UNREACHABLE()         - a guard macro for unreachable code (default: assert(false))
    SOKOL_GFX_API_DECL          - public function declaration prefix (default: extern)
    SOKOL_API_DECL              - same as SOKOL_GFX_API_DECL
    SOKOL_API_IMPL              - public function implementation prefix (default: -)
    SOKOL_TRACE_HOOKS           - enable trace hook callbacks (search below for TRACE HOOKS)
    SOKOL_EXTERNAL_GL_LOADER    - indicates that you're using your own GL loader, in this case
                                  sokol_gfx.h will not include any platform GL headers and disable
                                  the integrated Win32 GL loader

    If sokol_gfx.h is compiled as a DLL, define the following before
    including the declaration or implementation:

    SOKOL_DLL

    On Windows, SOKOL_DLL will define SOKOL_GFX_API_DECL as __declspec(dllexport)
    or __declspec(dllimport) as needed.

    If you want to compile without deprecated structs and functions,
    define:

    SOKOL_NO_DEPRECATED

    Optionally define the following to force debug checks and validations
    even in release mode:

    SOKOL_DEBUG         - by default this is defined if _DEBUG is defined

    sokol_gfx DOES NOT:
    ===================
    - create a window, swapchain or the 3D-API context/device, you must do this
      before sokol_gfx is initialized, and pass any required information
      (like 3D device pointers) to the sokol_gfx initialization call

    - present the rendered frame, how this is done exactly usually depends
      on how the window and 3D-API context/device was created

    - provide a unified shader language, instead 3D-API-specific shader
      source-code or shader-bytecode must be provided (for the "official"
      offline shader cross-compiler, see here:
      https://github.com/floooh/sokol-tools/blob/master/docs/sokol-shdc.md)


    STEP BY STEP
    ============
    --- to initialize sokol_gfx, after creating a window and a 3D-API
        context/device, call:

            sg_setup(const sg_desc*)

        Depending on the selected 3D backend, sokol-gfx requires some
        information, like a device pointer, default swapchain pixel formats
        and so on. If you are using sokol_app.h for the window system
        glue, you can use a helper function provided in the sokol_glue.h
        header:

            #include "sokol_gfx.h"
            #include "sokol_app.h"
            #include "sokol_glue.h"
            //...
            sg_setup(&(sg_desc){
                .environment = sglue_environment(),
            });

        To get any logging output for errors and from the validation layer, you
        need to provide a logging callback. Easiest way is through sokol_log.h:

            #include "sokol_log.h"
            //...
            sg_setup(&(sg_desc){
                //...
                .logger.func = slog_func,
            });

    --- create resource objects (at least buffers, shaders and pipelines,
        and optionally images, samplers and render-pass-attachments):

            sg_buffer sg_make_buffer(const sg_buffer_desc*)
            sg_image sg_make_image(const sg_image_desc*)
            sg_sampler sg_make_sampler(const sg_sampler_desc*)
            sg_shader sg_make_shader(const sg_shader_desc*)
            sg_pipeline sg_make_pipeline(const sg_pipeline_desc*)
            sg_attachments sg_make_attachments(const sg_attachments_desc*)

    --- start a render pass:

            sg_begin_pass(const sg_pass* pass);

        Typically, passes render into an externally provided swapchain which
        presents the rendering result on the display. Such a 'swapchain pass'
        is started like this:

            sg_begin_pass(&(sg_pass){ .action = { ... }, .swapchain = sglue_swapchain() })

        ...where .action is an sg_pass_action struct containing actions to be performed
        at the start and end of a render pass (such as clearing the render surfaces to
        a specific color), and .swapchain is an sg_swapchain
        struct all the required information to render into the swapchain's surfaces.

        To start an 'offscreen pass' into sokol-gfx image objects, an sg_attachment
        object handle is required instead of an sg_swapchain struct. An offscreen
        pass is started like this (assuming attachments is an sg_attachments handle):

            sg_begin_pass(&(sg_pass){ .action = { ... }, .attachments = attachments });

    --- set the render pipeline state for the next draw call with:

            sg_apply_pipeline(sg_pipeline pip)

    --- fill an sg_bindings struct with the resource bindings for the next
        draw call (0..N vertex buffers, 0 or 1 index buffer, 0..N image-objects,
        samplers and storage-buffers), and call:

            sg_apply_bindings(const sg_bindings* bindings)

        to update the resource bindings

    --- optionally update shader uniform data with:

            sg_apply_uniforms(sg_shader_stage stage, int ub_index, const sg_range* data)

        Read the section 'UNIFORM DATA LAYOUT' to learn about the expected memory layout
        of the uniform data passed into sg_apply_uniforms().

    --- kick off a draw call with:

            sg_draw(int base_element, int num_elements, int num_instances)

        The sg_draw() function unifies all the different ways to render primitives
        in a single call (indexed vs non-indexed rendering, and instanced vs non-instanced
        rendering). In case of indexed rendering, base_element and num_element specify
        indices in the currently bound index buffer. In case of non-indexed rendering
        base_element and num_elements specify vertices in the currently bound
        vertex-buffer(s). To perform instanced rendering, the rendering pipeline
        must be setup for instancing (see sg_pipeline_desc below), a separate vertex buffer
        containing per-instance data must be bound, and the num_instances parameter
        must be > 1.

    --- finish the current rendering pass with:

            sg_end_pass()

    --- when done with the current frame, call

            sg_commit()

    --- at the end of your program, shutdown sokol_gfx with:

            sg_shutdown()

    --- if you need to destroy resources before sg_shutdown(), call:

            sg_destroy_buffer(sg_buffer buf)
            sg_destroy_image(sg_image img)
            sg_destroy_sampler(sg_sampler smp)
            sg_destroy_shader(sg_shader shd)
            sg_destroy_pipeline(sg_pipeline pip)
            sg_destroy_attachments(sg_attachments atts)

    --- to set a new viewport rectangle, call

            sg_apply_viewport(int x, int y, int width, int height, bool origin_top_left)

        ...or if you want to specify the viewport rectangle with float values:

            sg_apply_viewportf(float x, float y, float width, float height, bool origin_top_left)

    --- to set a new scissor rect, call:

            sg_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left)

        ...or with float values:

            sg_apply_scissor_rectf(float x, float y, float width, float height, bool origin_top_left)

        Both sg_apply_viewport() and sg_apply_scissor_rect() must be called
        inside a rendering pass

        Note that sg_begin_default_pass() and sg_begin_pass() will reset both the
        viewport and scissor rectangles to cover the entire framebuffer.

    --- to update (overwrite) the content of buffer and image resources, call:

            sg_update_buffer(sg_buffer buf, const sg_range* data)
            sg_update_image(sg_image img, const sg_image_data* data)

        Buffers and images to be updated must have been created with
        SG_USAGE_DYNAMIC or SG_USAGE_STREAM

        Only one update per frame is allowed for buffer and image resources when
        using the sg_update_*() functions. The rationale is to have a simple
        countermeasure to avoid the CPU scribbling over data the GPU is currently
        using, or the CPU having to wait for the GPU

        Buffer and image updates can be partial, as long as a rendering
        operation only references the valid (updated) data in the
        buffer or image.

    --- to append a chunk of data to a buffer resource, call:

            int sg_append_buffer(sg_buffer buf, const sg_range* data)

        The difference to sg_update_buffer() is that sg_append_buffer()
        can be called multiple times per frame to append new data to the
        buffer piece by piece, optionally interleaved with draw calls referencing
        the previously written data.

        sg_append_buffer() returns a byte offset to the start of the
        written data, this offset can be assigned to
        sg_bindings.vertex_buffer_offsets[n] or
        sg_bindings.index_buffer_offset

        Code example:

        for (...) {
            const void* data = ...;
            const int num_bytes = ...;
            int offset = sg_append_buffer(buf, &(sg_range) { .ptr=data, .size=num_bytes });
            bindings.vertex_buffer_offsets[0] = offset;
            sg_apply_pipeline(pip);
            sg_apply_bindings(&bindings);
            sg_apply_uniforms(...);
            sg_draw(...);
        }

        A buffer to be used with sg_append_buffer() must have been created
        with SG_USAGE_DYNAMIC or SG_USAGE_STREAM.

        If the application appends more data to the buffer then fits into
        the buffer, the buffer will go into the "overflow" state for the
        rest of the frame.

        Any draw calls attempting to render an overflown buffer will be
        silently dropped (in debug mode this will also result in a
        validation error).

        You can also check manually if a buffer is in overflow-state by calling

            bool sg_query_buffer_overflow(sg_buffer buf)

        You can manually check to see if an overflow would occur before adding
        any data to a buffer by calling

            bool sg_query_buffer_will_overflow(sg_buffer buf, size_t size)

        NOTE: Due to restrictions in underlying 3D-APIs, appended chunks of
        data will be 4-byte aligned in the destination buffer. This means
        that there will be gaps in index buffers containing 16-bit indices
        when the number of indices in a call to sg_append_buffer() is
        odd. This isn't a problem when each call to sg_append_buffer()
        is associated with one draw call, but will be problematic when
        a single indexed draw call spans several appended chunks of indices.

    --- to check at runtime for optional features, limits and pixelformat support,
        call:

            sg_features sg_query_features()
            sg_limits sg_query_limits()
            sg_pixelformat_info sg_query_pixelformat(sg_pixel_format fmt)

    --- if you need to call into the underlying 3D-API directly, you must call:

            sg_reset_state_cache()

        ...before calling sokol_gfx functions again

    --- you can inspect the original sg_desc structure handed to sg_setup()
        by calling sg_query_desc(). This will return an sg_desc struct with
        the default values patched in instead of any zero-initialized values

    --- you can get a desc struct matching the creation attributes of a
        specific resource object via:

            sg_buffer_desc sg_query_buffer_desc(sg_buffer buf)
            sg_image_desc sg_query_image_desc(sg_image img)
            sg_sampler_desc sg_query_sampler_desc(sg_sampler smp)
            sg_shader_desc sq_query_shader_desc(sg_shader shd)
            sg_pipeline_desc sg_query_pipeline_desc(sg_pipeline pip)
            sg_attachments_desc sg_query_attachments_desc(sg_attachments atts)

        ...but NOTE that the returned desc structs may be incomplete, only
        creation attributes that are kept around internally after resource
        creation will be filled in, and in some cases (like shaders) that's
        very little. Any missing attributes will be set to zero. The returned
        desc structs might still be useful as partial blueprint for creating
        similar resources if filled up with the missing attributes.

        Calling the query-desc functions on an invalid resource will return
        completely zeroed structs (it makes sense to check  the resource state
        with sg_query_*_state() first)

    --- you can query the default resource creation parameters through the functions

            sg_buffer_desc sg_query_buffer_defaults(const sg_buffer_desc* desc)
            sg_image_desc sg_query_image_defaults(const sg_image_desc* desc)
            sg_sampler_desc sg_query_sampler_defaults(const sg_sampler_desc* desc)
            sg_shader_desc sg_query_shader_defaults(const sg_shader_desc* desc)
            sg_pipeline_desc sg_query_pipeline_defaults(const sg_pipeline_desc* desc)
            sg_attachments_desc sg_query_attachments_defaults(const sg_attachments_desc* desc)

        These functions take a pointer to a desc structure which may contain
        zero-initialized items for default values. These zero-init values
        will be replaced with their concrete values in the returned desc
        struct.

    --- you can inspect various internal resource runtime values via:

            sg_buffer_info sg_query_buffer_info(sg_buffer buf)
            sg_image_info sg_query_image_info(sg_image img)
            sg_sampler_info sg_query_sampler_info(sg_sampler smp)
            sg_shader_info sg_query_shader_info(sg_shader shd)
            sg_pipeline_info sg_query_pipeline_info(sg_pipeline pip)
            sg_attachments_info sg_query_attachments_info(sg_attachments atts)

        ...please note that the returned info-structs are tied quite closely
        to sokol_gfx.h internals, and may change more often than other
        public API functions and structs.

    --- you can query frame stats and control stats collection via:

            sg_query_frame_stats()
            sg_enable_frame_stats()
            sg_disable_frame_stats()
            sg_frame_stats_enabled()

    --- you can ask at runtime what backend sokol_gfx.h has been compiled for:

            sg_backend sg_query_backend(void)

    --- call the following helper functions to compute the number of
        bytes in a texture row or surface for a specific pixel format.
        These functions might be helpful when preparing image data for consumption
        by sg_make_image() or sg_update_image():

            int sg_query_row_pitch(sg_pixel_format fmt, int width, int int row_align_bytes);
            int sg_query_surface_pitch(sg_pixel_format fmt, int width, int height, int row_align_bytes);

        Width and height are generally in number pixels, but note that 'row' has different meaning
        for uncompressed vs compressed pixel formats: for uncompressed formats, a row is identical
        with a single line if pixels, while in compressed formats, one row is a line of *compression blocks*.

        This is why calling sg_query_surface_pitch() for a compressed pixel format and height
        N, N+1, N+2, ... may return the same result.

        The row_align_bytes parammeter is for added flexibility. For image data that goes into
        the sg_make_image() or sg_update_image() this should generally be 1, because these
        functions take tightly packed image data as input no matter what alignment restrictions
        exist in the backend 3D APIs.

    ON INITIALIZATION:
    ==================
    When calling sg_setup(), a pointer to an sg_desc struct must be provided
    which contains initialization options. These options provide two types
    of information to sokol-gfx:

        (1) upper bounds and limits needed to allocate various internal
            data structures:
                - the max number of resources of each type that can
                  be alive at the same time, this is used for allocating
                  internal pools
                - the max overall size of uniform data that can be
                  updated per frame, including a worst-case alignment
                  per uniform update (this worst-case alignment is 256 bytes)
                - the max size of all dynamic resource updates (sg_update_buffer,
                  sg_append_buffer and sg_update_image) per frame
            Not all of those limit values are used by all backends, but it is
            good practice to provide them none-the-less.

        (2) 3D backend "environment information" in a nested sg_environment struct:
            - pointers to backend-specific context- or device-objects (for instance
              the D3D11, WebGPU or Metal device objects)
            - defaults for external swapchain pixel formats and sample counts,
              these will be used as default values in image and pipeline objects,
              and the sg_swapchain struct passed into sg_begin_pass()
            Usually you provide a complete sg_environment struct through
            a helper function, as an example look at the sglue_environment()
            function in the sokol_glue.h header.

    See the documentation block of the sg_desc struct below for more information.


    ON RENDER PASSES
    ================
    Relevant samples:
        - https://floooh.github.io/sokol-html5/offscreen-sapp.html
        - https://floooh.github.io/sokol-html5/offscreen-msaa-sapp.html
        - https://floooh.github.io/sokol-html5/mrt-sapp.html
        - https://floooh.github.io/sokol-html5/mrt-pixelformats-sapp.html

    A render pass groups rendering commands into a set of render target images
    (called 'pass attachments'). Render target images can be used in subsequent
    passes as textures (it is invalid to use the same image both as render target
    and as texture in the same pass).

    The following sokol-gfx functions must only be called inside a render pass:

        sg_apply_viewport(f)
        sg_apply_scissor_rect(f)
        sg_apply_pipeline
        sg_apply_bindings
        sg_apply_uniforms
        sg_draw

    A frame must have at least one 'swapchain render pass' which renders into an
    externally provided swapchain provided as an sg_swapchain struct to the
    sg_begin_pass() function. The sg_swapchain struct must contain the
    following information:

        - the color pixel-format of the swapchain's render surface
        - an optional depth/stencil pixel format if the swapchain
          has a depth/stencil buffer
        - an optional sample-count for MSAA rendering
        - NOTE: the above three values can be zero-initialized, in that
          case the defaults from the sg_environment struct will be used that
          had been passed to the sg_setup() function.
        - a number of backend specific objects:
            - GL/GLES3: just a GL framebuffer handle
            - D3D11:
                - an ID3D11RenderTargetView for the rendering surface
                - if MSAA is used, an ID3D11RenderTargetView as
                  MSAA resolve-target
                - an optional ID3D11DepthStencilView for the
                  depth/stencil buffer
            - WebGPU
                - a WGPUTextureView object for the rendering surface
                - if MSAA is used, a WGPUTextureView object as MSAA resolve target
                - an optional WGPUTextureView for the
            - Metal (NOTE that the roles of provided surfaces is slightly
              different in Metal than in D3D11 or WebGPU, notably, the
              CAMetalDrawable is either rendered to directly, or serves
              as MSAA resolve target):
                - a CAMetalDrawable object which is either rendered
                  into directly, or in case of MSAA rendering, serves
                  as MSAA-resolve-target
                - if MSAA is used, an multisampled MTLTexture where
                  rendering goes into
                - an optional MTLTexture for the depth/stencil buffer

    It's recommended that you create a helper function which returns an
    initialized sg_swapchain struct by value. This can then be directly plugged
    into the sg_begin_pass function like this:

        sg_begin_pass(&(sg_pass){ .swapchain = sglue_swapchain() });

    As an example for such a helper function check out the function sglue_swapchain()
    in the sokol_glue.h header.

    For offscreen render passes, the render target images used in a render pass
    are baked into an immutable sg_attachments object.

    For a simple offscreen scenario with one color-, one depth-stencil-render
    target and without multisampling, creating an attachment object looks like this:

    First create two render target images, one with a color pixel format,
    and one with the depth- or depth-stencil pixel format. Both images
    must have the same dimensions:

        const sg_image color_img = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = 256,
            .height = 256,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .sample_count = 1,
        });
        const sg_image depth_img = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = 256,
            .height = 256,
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .sample_count = 1,
        });

    NOTE: when creating render target images, have in mind that some default values
    are aligned with the default environment attributes in the sg_environment struct
    that was passed into the sg_setup() call:

        - the default value for sg_image_desc.pixel_format is taken from
          sg_environment.defaults.color_format
        - the default value for sg_image_desc.sample_count is taken from
          sg_environment.defaults.sample_count
        - the default value for sg_image_desc.num_mipmaps is always 1

    Next create an attachments object:

        const sg_attachments atts = sg_make_attachments(&(sg_attachments_desc){
            .colors[0].image = color_img,
            .depth_stencil.image = depth_img,
        });

    This attachments object is then passed into the sg_begin_pass() function
    in place of the swapchain struct:

        sg_begin_pass(&(sg_pass){ .attachments = atts });

    Swapchain and offscreen passes form dependency trees each with a swapchain
    pass at the root, offscreen passes as nodes, and render target images as
    dependencies between passes.

    sg_pass_action structs are used to define actions that should happen at the
    start and end of rendering passes (such as clearing pass attachments to a
    specific color or depth-value, or performing an MSAA resolve operation at
    the end of a pass).

    A typical sg_pass_action object which clears the color attachment to black
    might look like this:

        const sg_pass_action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        };

    This omits the defaults for the color attachment store action, and
    the depth-stencil-attachments actions. The same pass action with the
    defaults explicitly filled in would look like this:

        const sg_pass_action pass_action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .store_action = SG_STOREACTION_STORE,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
            },
            .depth = = {
                .load_action = SG_LOADACTION_CLEAR,
                .store_action = SG_STOREACTION_DONTCARE,
                .clear_value = 1.0f,
            },
            .stencil = {
                .load_action = SG_LOADACTION_CLEAR,
                .store_action = SG_STOREACTION_DONTCARE,
                .clear_value = 0
            }
        };

    With the sg_pass object and sg_pass_action struct in place everything
    is ready now for the actual render pass:

    Using such this prepared sg_pass_action in a swapchain pass looks like
    this:

        sg_begin_pass(&(sg_pass){
            .action = pass_action,
            .swapchain = sglue_swapchain()
        });
        ...
        sg_end_pass();

    ...of alternatively in one offscreen pass:

        sg_begin_pass(&(sg_pass){
            .action = pass_action,
            .attachments = attachments,
        });
        ...
        sg_end_pass();

    Offscreen rendering can also go into a mipmap, or a slice/face of
    a cube-, array- or 3d-image (which some restrictions, for instance
    it's not possible to create a 3D image with a depth/stencil pixel format,
    these exceptions are generally caught by the sokol-gfx validation layer).

    The mipmap/slice selection happens at attachments creation time, for instance
    to render into mipmap 2 of slice 3 of an array texture:

        const sg_attachments atts = sg_make_attachments(&(sg_attachments_desc){
            .colors[0] = {
                .image = color_img,
                .mip_level = 2,
                .slice = 3,
            },
            .depth_stencil.image = depth_img,
        });

    If MSAA offscreen rendering is desired, the multi-sample rendering result
    must be 'resolved' into a separate 'resolve image', before that image can
    be used as texture.

    NOTE: currently multisample-images cannot be bound as textures.

    Creating a simple attachments object for multisampled rendering requires
    3 attachment images: the color attachment image which has a sample
    count > 1, a resolve attachment image of the same size and pixel format
    but a sample count == 1, and a depth/stencil attachment image with
    the same size and sample count as the color attachment image:

        const sg_image color_img = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = 256,
            .height = 256,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .sample_count = 4,
        });
        const sg_image resolve_img = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = 256,
            .height = 256,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .sample_count = 1,
        });
        const sg_image depth_img = sg_make_image(&(sg_image_desc){
            .render_target = true,
            .width = 256,
            .height = 256,
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .sample_count = 4,
        });

    ...create the attachments object:

        const sg_attachments atts = sg_make_attachments(&(sg_attachments_desc){
            .colors[0].image = color_img,
            .resolves[0].image = resolve_img,
            .depth_stencil.image = depth_img,
        });

    If an attachments object defines a resolve image in a specific resolve attachment slot,
    an 'msaa resolve operation' will happen in sg_end_pass().

    In this scenario, the content of the MSAA color attachment doesn't need to be
    preserved (since it's only needed inside sg_end_pass for the msaa-resolve), so
    the .store_action should be set to "don't care":

        const sg_pass_action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .store_action = SG_STOREACTION_DONTCARE,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
            }
        };

    The actual render pass looks as usual:

        sg_begin_pass(&(sg_pass){ .action = pass_action, .attachments = atts });
        ...
        sg_end_pass();

    ...after sg_end_pass() the only difference to the non-msaa scenario is that the
    rendering result which is going to be used as texture in a followup pass is
    in 'resolve_img', not in 'color_img' (in fact, trying to bind color_img as a
    texture would result in a validation error).


    ON SHADER CREATION
    ==================
    sokol-gfx doesn't come with an integrated shader cross-compiler, instead
    backend-specific shader sources or binary blobs need to be provided when
    creating a shader object, along with information about the shader resource
    binding interface needed in the sokol-gfx validation layer and to properly
    bind shader resources on the CPU-side to be consumable by the GPU-side.

    The easiest way to provide all this shader creation data is to use the
    sokol-shdc shader compiler tool to compile shaders from a common
    GLSL syntax into backend-specific sources or binary blobs, along with
    shader interface information and uniform blocks mapped to C structs.

    To create a shader using a C header which has been code-generated by sokol-shdc:

        // include the C header code-generated by sokol-shdc:
        #include "myshader.glsl.h"
        ...

        // create shader using a code-generated helper function from the C header:
        sg_shader shd = sg_make_shader(myshader_shader_desc(sg_query_backend()));

    The samples in the 'sapp' subdirectory of the sokol-samples project
    also use the sokol-shdc approach:

        https://github.com/floooh/sokol-samples/tree/master/sapp

    If you're planning to use sokol-shdc, you can stop reading here, instead
    continue with the sokol-shdc documentation:

        https://github.com/floooh/sokol-tools/blob/master/docs/sokol-shdc.md

    To create shaders with backend-specific shader code or binary blobs,
    the sg_make_shader() function requires the following information:

    - Shader code or shader binary blobs for the vertex- and fragment- shader-stage:
        - for the desktop GL backend, source code can be provided in '#version 410' or
          '#version 430', version 430 is required for storage buffer support, but note
          that this is not available on macOS
        - for the GLES3 backend, source code must be provided in '#version 300 es' syntax
        - for the D3D11 backend, shaders can be provided as source or binary blobs, the
          source code should be in HLSL4.0 (for best compatibility) or alternatively
          in HLSL5.0 syntax (other versions may work but are not tested), NOTE: when
          shader source code is provided for the D3D11 backend, sokol-gfx will dynamically
          load 'd3dcompiler_47.dll'
        - for the Metal backends, shaders can be provided as source or binary blobs, the
          MSL version should be in 'metal-1.1' (other versions may work but are not tested)
        - for the WebGPU backend, shader must be provided as WGSL source code
        - optionally the following shader-code related attributes can be provided:
            - an entry function name (only on D3D11 or Metal, but not OpenGL)
            - on D3D11 only, a compilation target (default is "vs_4_0" and "ps_4_0")

    - Depending on backend, information about the input vertex attributes used by the
      vertex shader:
        - Metal: no information needed since vertex attributes are always bound
          by their attribute location defined in the shader via '[[attribute(N)]]'
        - WebGPU: no information needed since vertex attributes are always
          bound by their attribute location defined in the shader via `@location(N)`
        - GLSL: vertex attribute names can be optionally provided, in that case their
          location will be looked up by name, otherwise, the vertex attribute location
          can be defined with 'layout(location = N)', PLEASE NOTE that the name-lookup method
          may be removed at some point
        - D3D11: a 'semantic name' and 'semantic index' must be provided for each vertex
          attribute, e.g. if the vertex attribute is defined as 'TEXCOORD1' in the shader,
          the semantic name would be 'TEXCOORD', and the semantic index would be '1'

    - Information about each uniform block used in the shader:
        - The size of the uniform block in number of bytes.
        - A memory layout hint (currently 'native' or 'std140') where 'native' defines a
          backend-specific memory layout which shouldn't be used for cross-platform code.
          Only std140 guarantees a backend-agnostic memory layout.
        - For GLSL only: a description of the internal uniform block layout, which maps
          member types and their offsets on the CPU side to uniform variable names
          in the GLSL shader
        - please also NOTE the documentation sections about UNIFORM DATA LAYOUT
          and CROSS-BACKEND COMMON UNIFORM DATA LAYOUT below!

    - A description of each storage buffer used in the shader:
        - a boolean 'readonly' flag, note that currently only
          readonly storage buffers are supported
        - note that storage buffers are not supported on all backends
          and platforms

    - A description of each texture/image used in the shader:
        - the expected image type:
            - SG_IMAGETYPE_2D
            - SG_IMAGETYPE_CUBE
            - SG_IMAGETYPE_3D
            - SG_IMAGETYPE_ARRAY
        - the expected 'image sample type':
            - SG_IMAGESAMPLETYPE_FLOAT
            - SG_IMAGESAMPLETYPE_DEPTH
            - SG_IMAGESAMPLETYPE_SINT
            - SG_IMAGESAMPLETYPE_UINT
            - SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT
        - a flag whether the texture is expected to be multisampled
          (currently it's not supported to fetch data from multisampled
          textures in shaders, but this is planned for a later time)

    - A description of each texture sampler used in the shader:
        - SG_SAMPLERTYPE_FILTERING,
        - SG_SAMPLERTYPE_NONFILTERING,
        - SG_SAMPLERTYPE_COMPARISON,

    - An array of 'image-sampler-pairs' used by the shader to sample textures,
      for D3D11, Metal and WebGPU this is used for validation purposes to check
      whether the texture and sampler are compatible with each other (especially
      WebGPU is very picky about combining the correct
      texture-sample-type with the correct sampler-type). For GLSL an
      additional 'combined-image-sampler name' must be provided because 'OpenGL
      style GLSL' cannot handle separate texture and sampler objects, but still
      groups them into a traditional GLSL 'sampler object'.

    Compatibility rules for image-sample-type vs sampler-type are as follows:

        - SG_IMAGESAMPLETYPE_FLOAT => (SG_SAMPLERTYPE_FILTERING or SG_SAMPLERTYPE_NONFILTERING)
        - SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT => SG_SAMPLERTYPE_NONFILTERING
        - SG_IMAGESAMPLETYPE_SINT => SG_SAMPLERTYPE_NONFILTERING
        - SG_IMAGESAMPLETYPE_UINT => SG_SAMPLERTYPE_NONFILTERING
        - SG_IMAGESAMPLETYPE_DEPTH => SG_SAMPLERTYPE_COMPARISON

    For example code of how to create backend-specific shader objects,
    please refer to the following samples:

        - for D3D11:    https://github.com/floooh/sokol-samples/tree/master/d3d11
        - for Metal:    https://github.com/floooh/sokol-samples/tree/master/metal
        - for OpenGL:   https://github.com/floooh/sokol-samples/tree/master/glfw
        - for GLES3:    https://github.com/floooh/sokol-samples/tree/master/html5
        - for WebGPI:   https://github.com/floooh/sokol-samples/tree/master/wgpu


    ON SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT AND SG_SAMPLERTYPE_NONFILTERING
    ========================================================================
    The WebGPU backend introduces the concept of 'unfilterable-float' textures,
    which can only be combined with 'nonfiltering' samplers (this is a restriction
    specific to WebGPU, but since the same sokol-gfx code should work across
    all backend, the sokol-gfx validation layer also enforces this restriction
    - the alternative would be undefined behaviour in some backend APIs on
    some devices).

    The background is that some mobile devices (most notably iOS devices) can
    not perform linear filtering when sampling textures with certain pixel
    formats, most notable the 32F formats:

        - SG_PIXELFORMAT_R32F
        - SG_PIXELFORMAT_RG32F
        - SG_PIXELFORMAT_RGBA32F

    The information of whether a shader is going to be used with such an
    unfilterable-float texture must already be provided in the sg_shader_desc
    struct when creating the shader (see the above section "ON SHADER CREATION").

    If you are using the sokol-shdc shader compiler, the information whether a
    texture/sampler binding expects an 'unfilterable-float/nonfiltering'
    texture/sampler combination cannot be inferred from the shader source
    alone, you'll need to provide this hint via annotation-tags. For instance
    here is an example from the ozz-skin-sapp.c sample shader which samples an
    RGBA32F texture with skinning matrices in the vertex shader:

    ```glsl
    @image_sample_type joint_tex unfilterable_float
    uniform texture2D joint_tex;
    @sampler_type smp nonfiltering
    uniform sampler smp;
    ```

    This will result in SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT and
    SG_SAMPLERTYPE_NONFILTERING being written to the code-generated
    sg_shader_desc struct.


    UNIFORM DATA LAYOUT:
    ====================
    NOTE: if you use the sokol-shdc shader compiler tool, you don't need to worry
    about the following details.

    The data that's passed into the sg_apply_uniforms() function must adhere to
    specific layout rules so that the GPU shader finds the uniform block
    items at the right offset.

    For the D3D11 and Metal backends, sokol-gfx only cares about the size of uniform
    blocks, but not about the internal layout. The data will just be copied into
    a uniform/constant buffer in a single operation and it's up you to arrange the
    CPU-side layout so that it matches the GPU side layout. This also means that with
    the D3D11 and Metal backends you are not limited to a 'cross-platform' subset
    of uniform variable types.

    If you ever only use one of the D3D11, Metal *or* WebGPU backend, you can stop reading here.

    For the GL backends, the internal layout of uniform blocks matters though,
    and you are limited to a small number of uniform variable types. This is
    because sokol-gfx must be able to locate the uniform block members in order
    to upload them to the GPU with glUniformXXX() calls.

    To describe the uniform block layout to sokol-gfx, the following information
    must be passed to the sg_make_shader() call in the sg_shader_desc struct:

        - a hint about the used packing rule (either SG_UNIFORMLAYOUT_NATIVE or
          SG_UNIFORMLAYOUT_STD140)
        - a list of the uniform block members types in the correct order they
          appear on the CPU side

    For example if the GLSL shader has the following uniform declarations:

        uniform mat4 mvp;
        uniform vec2 offset0;
        uniform vec2 offset1;
        uniform vec2 offset2;

    ...and on the CPU side, there's a similar C struct:

        typedef struct {
            float mvp[16];
            float offset0[2];
            float offset1[2];
            float offset2[2];
        } params_t;

    ...the uniform block description in the sg_shader_desc must look like this:

        sg_shader_desc desc = {
            .vs.uniform_blocks[0] = {
                .size = sizeof(params_t),
                .layout = SG_UNIFORMLAYOUT_NATIVE,  // this is the default and can be omitted
                .uniforms = {
                    // order must be the same as in 'params_t':
                    [0] = { .name = "mvp", .type = SG_UNIFORMTYPE_MAT4 },
                    [1] = { .name = "offset0", .type = SG_UNIFORMTYPE_VEC2 },
                    [2] = { .name = "offset1", .type = SG_UNIFORMTYPE_VEC2 },
                    [3] = { .name = "offset2", .type = SG_UNIFORMTYPE_VEC2 },
                }
            }
        };

    With this information sokol-gfx can now compute the correct offsets of the data items
    within the uniform block struct.

    The SG_UNIFORMLAYOUT_NATIVE packing rule works fine if only the GL backends are used,
    but for proper D3D11/Metal/GL a subset of the std140 layout must be used which is
    described in the next section:


    CROSS-BACKEND COMMON UNIFORM DATA LAYOUT
    ========================================
    For cross-platform / cross-3D-backend code it is important that the same uniform block
    layout on the CPU side can be used for all sokol-gfx backends. To achieve this,
    a common subset of the std140 layout must be used:

    - The uniform block layout hint in sg_shader_desc must be explicitly set to
      SG_UNIFORMLAYOUT_STD140.
    - Only the following GLSL uniform types can be used (with their associated sokol-gfx enums):
        - float => SG_UNIFORMTYPE_FLOAT
        - vec2  => SG_UNIFORMTYPE_FLOAT2
        - vec3  => SG_UNIFORMTYPE_FLOAT3
        - vec4  => SG_UNIFORMTYPE_FLOAT4
        - int   => SG_UNIFORMTYPE_INT
        - ivec2 => SG_UNIFORMTYPE_INT2
        - ivec3 => SG_UNIFORMTYPE_INT3
        - ivec4 => SG_UNIFORMTYPE_INT4
        - mat4  => SG_UNIFORMTYPE_MAT4
    - Alignment for those types must be as follows (in bytes):
        - float => 4
        - vec2  => 8
        - vec3  => 16
        - vec4  => 16
        - int   => 4
        - ivec2 => 8
        - ivec3 => 16
        - ivec4 => 16
        - mat4  => 16
    - Arrays are only allowed for the following types: vec4, int4, mat4.

    Note that the HLSL cbuffer layout rules are slightly different from the
    std140 layout rules, this means that the cbuffer declarations in HLSL code
    must be tweaked so that the layout is compatible with std140.

    The by far easiest way to tackle the common uniform block layout problem is
    to use the sokol-shdc shader cross-compiler tool!

    ON STORAGE BUFFERS
    ==================
    Storage buffers can be used to pass large amounts of random access structured
    data fromt the CPU side to the shaders. They are similar to data textures, but are
    more convenient to use both on the CPU and shader side since they can be accessed
    in shaders as as a 1-dimensional array of struct items.

    Storage buffers are *NOT* supported on the following platform/backend combos:

    - macOS+GL (because storage buffers require GL 4.3, while macOS only goes up to GL 4.1)
    - all GLES3 platforms (WebGL2, iOS, Android - with the option that support on
      Android may be added at a later point)

    Currently only 'readonly' storage buffers are supported (meaning it's not possible
    to write to storage buffers from shaders).

    To use storage buffers, the following steps are required:

        - write a shader which uses storage buffers (also see the example links below)
        - create one or more storage buffers via sg_make_buffer() with the
          buffer type SG_BUFFERTYPE_STORAGEBUFFER
        - when creating a shader via sg_make_shader(), populate the sg_shader_desc
          struct with binding info (when using sokol-shdc, this step will be taken care
          of automatically)
            - which storage buffer bind slots on the vertex- and fragment-stage
              are occupied
            - whether the storage buffer on that bind slot is readonly (this is currently required
              to be true)
        - when calling sg_apply_bindings(), apply the matching bind slots with the previously
          created storage buffers
        - ...and that's it.

    For more details, see the following backend-agnostic sokol samples:

    - simple vertex pulling from a storage buffer:
        - C code: https://github.com/floooh/sokol-samples/blob/master/sapp/vertexpull-sapp.c
        - shader: https://github.com/floooh/sokol-samples/blob/master/sapp/vertexpull-sapp.glsl
    - instanced rendering via storage buffers (vertex- and instance-pulling):
        - C code: https://github.com/floooh/sokol-samples/blob/master/sapp/instancing-pull-sapp.c
        - shader: https://github.com/floooh/sokol-samples/blob/master/sapp/instancing-pull-sapp.glsl
    - storage buffers both on the vertex- and fragment-stage:
        - C code: https://github.com/floooh/sokol-samples/blob/master/sapp/sbuftex-sapp.c
        - shader: https://github.com/floooh/sokol-samples/blob/master/sapp/sbuftex-sapp.glsl
    - the Ozz animation sample rewritten to pull all rendering data from storage buffers:
        - C code: https://github.com/floooh/sokol-samples/blob/master/sapp/ozz-storagebuffer-sapp.cc
        - shader: https://github.com/floooh/sokol-samples/blob/master/sapp/ozz-storagebuffer-sapp.glsl

    ...also see the following backend-specific vertex pulling samples (those also don't use sokol-shdc):

    - D3D11: https://github.com/floooh/sokol-samples/blob/master/d3d11/vertexpulling-d3d11.c
    - desktop GL: https://github.com/floooh/sokol-samples/blob/master/glfw/vertexpulling-glfw.c
    - Metal: https://github.com/floooh/sokol-samples/blob/master/metal/vertexpulling-metal.c
    - WebGPU: https://github.com/floooh/sokol-samples/blob/master/wgpu/vertexpulling-wgpu.c

    Storage buffer shader authoring caveats when using sokol-shdc:

        - declare a storage buffer interface block with `readonly buffer [name] { ... }`
        - do NOT annotate storage buffers with `layout(...)`, sokol-shdc will take care of that
        - declare a struct which describes a single array item in the storage buffer interface block
        - only put a single flexible array member into the storage buffer interface block

        E.g. a complete example in 'sokol-shdc GLSL':

        ```glsl
        // declare a struct:
        struct sb_vertex {
            vec3 pos;
            vec4 color;
        }
        // declare a buffer interface block with a single flexible struct array:
        readonly buffer vertices {
            sb_vertex vtx[];
        }
        // in the shader function, access the storage buffer like this:
        void main() {
            vec3 pos = vtx[gl_VertexIndex].pos;
            ...
        }
        ```

    Backend-specific storage-buffer caveats (not relevant when using sokol-shdc):

        D3D11:
            - storage buffers are created as 'raw' Byte Address Buffers
              (https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-intro#raw-views-of-buffers)
            - in HLSL, use a ByteAddressBuffer to access the buffer content
              (https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-byteaddressbuffer)
            - in D3D11, storage buffers and textures share the same bind slots, sokol-gfx reserves
              shader resource slots 0..15 for textures and 16..23 for storage buffers.
            - e.g. in HLSL, storage buffer bindings start at register(t16) no matter the shader stage

        Metal:
            - in Metal there is no internal difference between vertex-, uniform- and
              storage-buffers, all are bound to the same 'buffer bind slots' with the
              following reserved ranges:
                - vertex shader stage:
                    - uniform buffers (internal): slots 0..3
                    - vertex buffers: slots 4..11
                    - storage buffers: slots 12..19
                - fragment shader stage:
                    - uniform buffers (internal): slots 0..3
                    - storage buffers: slots 4..11
            - this means in MSL, storage buffer bindings start at [[buffer(12)]] in the vertex
              shaders, and at [[buffer(4)]] in fragment shaders

        GL:
            - the GL backend doesn't use name-lookup to find storage buffer bindings, this
              means you must annotate buffers with `layout(std430, binding=N)` in GLSL
            - ...where N is 0..7 in the vertex shader, and 8..15 in the fragment shader

        WebGPU:
            - in WGSL, use the following bind locations for the various shader resource types:
            - vertex shader stage:
                - textures `@group(1) @binding(0..15)`
                - samplers `@group(1) @binding(16..31)`
                - storage buffers `@group(1) @binding(32..47)`
            - fragment shader stage:
                - textures `@group(1) @binding(48..63)`
                - samplers `@group(1) @binding(64..79)`
                - storage buffers `@group(1) @binding(80..95)`

    TRACE HOOKS:
    ============
    sokol_gfx.h optionally allows to install "trace hook" callbacks for
    each public API functions. When a public API function is called, and
    a trace hook callback has been installed for this function, the
    callback will be invoked with the parameters and result of the function.
    This is useful for things like debugging- and profiling-tools, or
    keeping track of resource creation and destruction.

    To use the trace hook feature:

    --- Define SOKOL_TRACE_HOOKS before including the implementation.

    --- Setup an sg_trace_hooks structure with your callback function
        pointers (keep all function pointers you're not interested
        in zero-initialized), optionally set the user_data member
        in the sg_trace_hooks struct.

    --- Install the trace hooks by calling sg_install_trace_hooks(),
        the return value of this function is another sg_trace_hooks
        struct which contains the previously set of trace hooks.
        You should keep this struct around, and call those previous
        functions pointers from your own trace callbacks for proper
        chaining.

    As an example of how trace hooks are used, have a look at the
    imgui/sokol_gfx_imgui.h header which implements a realtime
    debugging UI for sokol_gfx.h on top of Dear ImGui.


    A NOTE ON PORTABLE PACKED VERTEX FORMATS:
    =========================================
    There are two things to consider when using packed
    vertex formats like UBYTE4, SHORT2, etc which need to work
    across all backends:

    - D3D11 can only convert *normalized* vertex formats to
      floating point during vertex fetch, normalized formats
      have a trailing 'N', and are "normalized" to a range
      -1.0..+1.0 (for the signed formats) or 0.0..1.0 (for the
      unsigned formats):

        - SG_VERTEXFORMAT_BYTE4N
        - SG_VERTEXFORMAT_UBYTE4N
        - SG_VERTEXFORMAT_SHORT2N
        - SG_VERTEXFORMAT_USHORT2N
        - SG_VERTEXFORMAT_SHORT4N
        - SG_VERTEXFORMAT_USHORT4N

      D3D11 will not convert *non-normalized* vertex formats to floating point
      vertex shader inputs, those can only be uses with the *ivecn* vertex shader
      input types when D3D11 is used as backend (GL and Metal can use both formats)

        - SG_VERTEXFORMAT_BYTE4,
        - SG_VERTEXFORMAT_UBYTE4
        - SG_VERTEXFORMAT_SHORT2
        - SG_VERTEXFORMAT_SHORT4

    For a vertex input layout which works on all platforms, only use the following
    vertex formats, and if needed "expand" the normalized vertex shader
    inputs in the vertex shader by multiplying with 127.0, 255.0, 32767.0 or
    65535.0:

        - SG_VERTEXFORMAT_FLOAT,
        - SG_VERTEXFORMAT_FLOAT2,
        - SG_VERTEXFORMAT_FLOAT3,
        - SG_VERTEXFORMAT_FLOAT4,
        - SG_VERTEXFORMAT_BYTE4N,
        - SG_VERTEXFORMAT_UBYTE4N,
        - SG_VERTEXFORMAT_SHORT2N,
        - SG_VERTEXFORMAT_USHORT2N
        - SG_VERTEXFORMAT_SHORT4N,
        - SG_VERTEXFORMAT_USHORT4N
        - SG_VERTEXFORMAT_UINT10_N2
        - SG_VERTEXFORMAT_HALF2
        - SG_VERTEXFORMAT_HALF4


    MEMORY ALLOCATION OVERRIDE
    ==========================
    You can override the memory allocation functions at initialization time
    like this:

        void* my_alloc(size_t size, void* user_data) {
            return malloc(size);
        }

        void my_free(void* ptr, void* user_data) {
            free(ptr);
        }

        ...
            sg_setup(&(sg_desc){
                // ...
                .allocator = {
                    .alloc_fn = my_alloc,
                    .free_fn = my_free,
                    .user_data = ...,
                }
            });
        ...

    If no overrides are provided, malloc and free will be used.

    This only affects memory allocation calls done by sokol_gfx.h
    itself though, not any allocations in OS libraries.


    ERROR REPORTING AND LOGGING
    ===========================
    To get any logging information at all you need to provide a logging callback in the setup call
    the easiest way is to use sokol_log.h:

        #include "sokol_log.h"

        sg_setup(&(sg_desc){ .logger.func = slog_func });

    To override logging with your own callback, first write a logging function like this:

        void my_log(const char* tag,                // e.g. 'sg'
                    uint32_t log_level,             // 0=panic, 1=error, 2=warn, 3=info
                    uint32_t log_item_id,           // SG_LOGITEM_*
                    const char* message_or_null,    // a message string, may be nullptr in release mode
                    uint32_t line_nr,               // line number in sokol_gfx.h
                    const char* filename_or_null,   // source filename, may be nullptr in release mode
                    void* user_data)
        {
            ...
        }

    ...and then setup sokol-gfx like this:

        sg_setup(&(sg_desc){
            .logger = {
                .func = my_log,
                .user_data = my_user_data,
            }
        });

    The provided logging function must be reentrant (e.g. be callable from
    different threads).

    If you don't want to provide your own custom logger it is highly recommended to use
    the standard logger in sokol_log.h instead, otherwise you won't see any warnings or
    errors.


    COMMIT LISTENERS
    ================
    It's possible to hook callback functions into sokol-gfx which are called from
    inside sg_commit() in unspecified order. This is mainly useful for libraries
    that build on top of sokol_gfx.h to be notified about the end/start of a frame.

    To add a commit listener, call:

        static void my_commit_listener(void* user_data) {
            ...
        }

        bool success = sg_add_commit_listener((sg_commit_listener){
            .func = my_commit_listener,
            .user_data = ...,
        });

    The function returns false if the internal array of commit listeners is full,
    or the same commit listener had already been added.

    If the function returns true, my_commit_listener() will be called each frame
    from inside sg_commit().

    By default, 1024 distinct commit listeners can be added, but this number
    can be tweaked in the sg_setup() call:

        sg_setup(&(sg_desc){
            .max_commit_listeners = 2048,
        });

    An sg_commit_listener item is equal to another if both the function
    pointer and user_data field are equal.

    To remove a commit listener:

        bool success = sg_remove_commit_listener((sg_commit_listener){
            .func = my_commit_listener,
            .user_data = ...,
        });

    ...where the .func and .user_data field are equal to a previous
    sg_add_commit_listener() call. The function returns true if the commit
    listener item was found and removed, and false otherwise.


    RESOURCE CREATION AND DESTRUCTION IN DETAIL
    ===========================================
    The 'vanilla' way to create resource objects is with the 'make functions':

        sg_buffer sg_make_buffer(const sg_buffer_desc* desc)
        sg_image sg_make_image(const sg_image_desc* desc)
        sg_sampler sg_make_sampler(const sg_sampler_desc* desc)
        sg_shader sg_make_shader(const sg_shader_desc* desc)
        sg_pipeline sg_make_pipeline(const sg_pipeline_desc* desc)
        sg_attachments sg_make_attachments(const sg_attachments_desc* desc)

    This will result in one of three cases:

        1. The returned handle is invalid. This happens when there are no more
           free slots in the resource pool for this resource type. An invalid
           handle is associated with the INVALID resource state, for instance:

                sg_buffer buf = sg_make_buffer(...)
                if (sg_query_buffer_state(buf) == SG_RESOURCESTATE_INVALID) {
                    // buffer pool is exhausted
                }

        2. The returned handle is valid, but creating the underlying resource
           has failed for some reason. This results in a resource object in the
           FAILED state. The reason *why* resource creation has failed differ
           by resource type. Look for log messages with more details. A failed
           resource state can be checked with:

                sg_buffer buf = sg_make_buffer(...)
                if (sg_query_buffer_state(buf) == SG_RESOURCESTATE_FAILED) {
                    // creating the resource has failed
                }

        3. And finally, if everything goes right, the returned resource is
           in resource state VALID and ready to use. This can be checked
           with:

                sg_buffer buf = sg_make_buffer(...)
                if (sg_query_buffer_state(buf) == SG_RESOURCESTATE_VALID) {
                    // creating the resource has failed
                }

    When calling the 'make functions', the created resource goes through a number
    of states:

        - INITIAL: the resource slot associated with the new resource is currently
          free (technically, there is no resource yet, just an empty pool slot)
        - ALLOC: a handle for the new resource has been allocated, this just means
          a pool slot has been reserved.
        - VALID or FAILED: in VALID state any 3D API backend resource objects have
          been successfully created, otherwise if anything went wrong, the resource
          will be in FAILED state.

    Sometimes it makes sense to first grab a handle, but initialize the
    underlying resource at a later time. For instance when loading data
    asynchronously from a slow data source, you may know what buffers and
    textures are needed at an early stage of the loading process, but actually
    loading the buffer or texture content can only be completed at a later time.

    For such situations, sokol-gfx resource objects can be created in two steps.
    You can allocate a handle upfront with one of the 'alloc functions':

        sg_buffer sg_alloc_buffer(void)
        sg_image sg_alloc_image(void)
        sg_sampler sg_alloc_sampler(void)
        sg_shader sg_alloc_shader(void)
        sg_pipeline sg_alloc_pipeline(void)
        sg_attachments sg_alloc_attachments(void)

    This will return a handle with the underlying resource object in the
    ALLOC state:

        sg_image img = sg_alloc_image();
        if (sg_query_image_state(img) == SG_RESOURCESTATE_ALLOC) {
            // allocating an image handle has succeeded, otherwise
            // the image pool is full
        }

    Such an 'incomplete' handle can be used in most sokol-gfx rendering functions
    without doing any harm, sokol-gfx will simply skip any rendering operation
    that involve resources which are not in VALID state.

    At a later time (for instance once the texture has completed loading
    asynchronously), the resource creation can be completed by calling one of
    the 'init functions', those functions take an existing resource handle and
    'desc struct':

        void sg_init_buffer(sg_buffer buf, const sg_buffer_desc* desc)
        void sg_init_image(sg_image img, const sg_image_desc* desc)
        void sg_init_sampler(sg_sampler smp, const sg_sampler_desc* desc)
        void sg_init_shader(sg_shader shd, const sg_shader_desc* desc)
        void sg_init_pipeline(sg_pipeline pip, const sg_pipeline_desc* desc)
        void sg_init_attachments(sg_attachments atts, const sg_attachments_desc* desc)

    The init functions expect a resource in ALLOC state, and after the function
    returns, the resource will be either in VALID or FAILED state. Calling
    an 'alloc function' followed by the matching 'init function' is fully
    equivalent with calling the 'make function' alone.

    Destruction can also happen as a two-step process. The 'uninit functions'
    will put a resource object from the VALID or FAILED state back into the
    ALLOC state:

        void sg_uninit_buffer(sg_buffer buf)
        void sg_uninit_image(sg_image img)
        void sg_uninit_sampler(sg_sampler smp)
        void sg_uninit_shader(sg_shader shd)
        void sg_uninit_pipeline(sg_pipeline pip)
        void sg_uninit_attachments(sg_attachments pass)

    Calling the 'uninit functions' with a resource that is not in the VALID or
    FAILED state is a no-op.

    To finally free the pool slot for recycling call the 'dealloc functions':

        void sg_dealloc_buffer(sg_buffer buf)
        void sg_dealloc_image(sg_image img)
        void sg_dealloc_sampler(sg_sampler smp)
        void sg_dealloc_shader(sg_shader shd)
        void sg_dealloc_pipeline(sg_pipeline pip)
        void sg_dealloc_attachments(sg_attachments atts)

    Calling the 'dealloc functions' on a resource that's not in ALLOC state is
    a no-op, but will generate a warning log message.

    Calling an 'uninit function' and 'dealloc function' in sequence is equivalent
    with calling the associated 'destroy function':

        void sg_destroy_buffer(sg_buffer buf)
        void sg_destroy_image(sg_image img)
        void sg_destroy_sampler(sg_sampler smp)
        void sg_destroy_shader(sg_shader shd)
        void sg_destroy_pipeline(sg_pipeline pip)
        void sg_destroy_attachments(sg_attachments atts)

    The 'destroy functions' can be called on resources in any state and generally
    do the right thing (for instance if the resource is in ALLOC state, the destroy
    function will be equivalent to the 'dealloc function' and skip the 'uninit part').

    And finally to close the circle, the 'fail functions' can be called to manually
    put a resource in ALLOC state into the FAILED state:

        sg_fail_buffer(sg_buffer buf)
        sg_fail_image(sg_image img)
        sg_fail_sampler(sg_sampler smp)
        sg_fail_shader(sg_shader shd)
        sg_fail_pipeline(sg_pipeline pip)
        sg_fail_attachments(sg_attachments atts)

    This is recommended if anything went wrong outside of sokol-gfx during asynchronous
    resource setup (for instance a file loading operation failed). In this case,
    the 'fail function' should be called instead of the 'init function'.

    Calling a 'fail function' on a resource that's not in ALLOC state is a no-op,
    but will generate a warning log message.

    NOTE: that two-step resource creation usually only makes sense for buffers
    and images, but not for samplers, shaders, pipelines or attachments. Most notably, trying
    to create a pipeline object with a shader that's not in VALID state will
    trigger a validation layer error, or if the validation layer is disabled,
    result in a pipeline object in FAILED state. Same when trying to create
    an attachments object with invalid image objects.


    WEBGPU CAVEATS
    ==============
    For a general overview and design notes of the WebGPU backend see:

        https://floooh.github.io/2023/10/16/sokol-webgpu.html

    In general, don't expect an automatic speedup when switching from the WebGL2
    backend to the WebGPU backend. Some WebGPU functions currently actually
    have a higher CPU overhead than similar WebGL2 functions, leading to the
    paradoxical situation that some WebGPU code may be slower than similar WebGL2
    code.

    - when writing WGSL shader code by hand, a specific bind-slot convention
      must be used:

      All uniform block structs must use `@group(0)`, with up to
      4 uniform blocks per shader stage.
        - Vertex shader uniform block bindings must start at `@group(0) @binding(0)`
        - Fragment shader uniform blocks bindings must start at `@group(0) @binding(4)`

      All textures and samplers must use `@group(1)` and start at specific
      offsets depending on resource type and shader stage.
        - Vertex shader textures must start at `@group(1) @binding(0)`
        - Vertex shader samplers must start at `@group(1) @binding(16)`
        - Vertex shader storage buffers must start at `@group(1) @binding(32)`
        - Fragment shader textures must start at `@group(1) @binding(48)`
        - Fragment shader samplers must start at `@group(1) @binding(64)`
        - Fragment shader storage buffers must start at `@group(1) @binding(80)`

      Note that the actual number of allowed per-stage texture- and sampler-bindings
      in sokol-gfx is currently lower than the above ranges (currently only up to
      12 textures, 8 samplers and 8 storage buffers are allowed per shader stage).

      If you use sokol-shdc to generate WGSL shader code, you don't need to worry
      about the above binding convention since sokol-shdc assigns bind slots
      automatically.

    - The sokol-gfx WebGPU backend uses the sg_desc.uniform_buffer_size item
      to allocate a single per-frame uniform buffer which must be big enough
      to hold all data written by sg_apply_uniforms() during a single frame,
      including a worst-case 256-byte alignment (e.g. each sg_apply_uniform
      call will cost 256 bytes of uniform buffer size). The default size
      is 4 MB, which is enough for 16384 sg_apply_uniform() calls per
      frame (assuming the uniform data 'payload' is less than 256 bytes
      per call). These rules are the same as for the Metal backend, so if
      you are already using the Metal backend you'll be fine.

    - sg_apply_bindings(): the sokol-gfx WebGPU backend implements a bindgroup
      cache to prevent excessive creation and destruction of BindGroup objects
      when calling sg_apply_bindings(). The number of slots in the bindgroups
      cache is defined in sg_desc.wgpu_bindgroups_cache_size when calling
      sg_setup. The cache size must be a power-of-2 number, with the default being
      1024. The bindgroups cache behaviour can be observed by calling the new
      function sg_query_frame_stats(), where the following struct items are
      of interest:

        .wgpu.num_bindgroup_cache_hits
        .wgpu.num_bindgroup_cache_misses
        .wgpu.num_bindgroup_cache_collisions
        .wgpu.num_bindgroup_cache_vs_hash_key_mismatch

      The value to pay attention to is `.wgpu.num_bindgroup_cache_collisions`,
      if this number if consistently higher than a few percent of the
      .wgpu.num_set_bindgroup value, it might be a good idea to bump the
      bindgroups cache size to the next power-of-2.

    - sg_apply_viewport(): WebGPU currently has a unique restriction that viewport
      rectangles must be contained entirely within the framebuffer. As a shitty
      workaround sokol_gfx.h will clip incoming viewport rectangles against
      the framebuffer, but this will distort the clipspace-to-screenspace mapping.
      There's no proper way to handle this inside sokol_gfx.h, this must be fixed
      in a future WebGPU update.

    - The sokol shader compiler generally adds `diagnostic(off, derivative_uniformity);`
      into the WGSL output. Currently only the Chrome WebGPU implementation seems
      to recognize this.

    - The vertex format SG_VERTEXFORMAT_UINT10_N2 is currently not supported because
      WebGPU lacks a matching vertex format (this is currently being worked on though,
      as soon as the vertex format shows up in webgpu.h, sokol_gfx.h will add support.

    - Likewise, the following sokol-gfx vertex formats are not supported in WebGPU:
      R16, R16SN, RG16, RG16SN, RGBA16, RGBA16SN and all PVRTC compressed format.
      Unlike unsupported vertex formats, unsupported pixel formats can be queried
      in cross-backend code via sg_query_pixel_format() though.

    - The Emscripten WebGPU shim currently doesn't support the Closure minification
      post-link-step (e.g. currently the emcc argument '--closure 1' or '--closure 2'
      will generate broken Javascript code.

    - sokol-gfx requires the WebGPU device feature `depth32float-stencil8` to be enabled
      (this should be widely supported)

    - sokol-gfx expects that the WebGPU device feature `float32-filterable` to *not* be
      enabled (since this would exclude all iOS devices)


    LICENSE
    =======
    zlib/libpng license

    Copyright (c) 2018 Andre Weissflog

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define SOKOL_GFX_INCLUDED (1)
#include <stddef.h>     // size_t
#include <stdint.h>
#include <stdbool.h>

#if defined(SOKOL_API_DECL) && !defined(SOKOL_GFX_API_DECL)
#define SOKOL_GFX_API_DECL SOKOL_API_DECL
#endif
#ifndef SOKOL_GFX_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_GFX_IMPL)
#define SOKOL_GFX_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_GFX_API_DECL __declspec(dllimport)
#else
#define SOKOL_GFX_API_DECL extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
    Resource id typedefs:

    sg_buffer:      vertex- and index-buffers
    sg_image:       images used as textures and render targets
    sg_sampler      sampler object describing how a texture is sampled in a shader
    sg_shader:      vertex- and fragment-shaders and shader interface information
    sg_pipeline:    associated shader and vertex-layouts, and render states
    sg_attachments: a baked collection of render pass attachment images

    Instead of pointers, resource creation functions return a 32-bit
    number which uniquely identifies the resource object.

    The 32-bit resource id is split into a 16-bit pool index in the lower bits,
    and a 16-bit 'generation counter' in the upper bits. The index allows fast
    pool lookups, and combined with the generation-counter it allows to detect
    'dangling accesses' (trying to use an object which no longer exists, and
    its pool slot has been reused for a new object)

    The resource ids are wrapped into a strongly-typed struct so that
    trying to pass an incompatible resource id is a compile error.
*/
typedef struct sg_buffer        { uint32_t id; } sg_buffer;
typedef struct sg_image         { uint32_t id; } sg_image;
typedef struct sg_sampler       { uint32_t id; } sg_sampler;
typedef struct sg_shader        { uint32_t id; } sg_shader;
typedef struct sg_pipeline      { uint32_t id; } sg_pipeline;
typedef struct sg_attachments   { uint32_t id; } sg_attachments;

/*
    sg_range is a pointer-size-pair struct used to pass memory blobs into
    sokol-gfx. When initialized from a value type (array or struct), you can
    use the SG_RANGE() macro to build an sg_range struct. For functions which
    take either a sg_range pointer, or a (C++) sg_range reference, use the
    SG_RANGE_REF macro as a solution which compiles both in C and C++.
*/
typedef struct sg_range {
    const void* ptr;
    size_t size;
} sg_range;

// disabling this for every includer isn't great, but the warnings are also quite pointless
#if defined(_MSC_VER)
#pragma warning(disable:4221)   // /W4 only: nonstandard extension used: 'x': cannot be initialized using address of automatic variable 'y'
#pragma warning(disable:4204)   // VS2015: nonstandard extension used: non-constant aggregate initializer
#endif
#if defined(__cplusplus)
#define SG_RANGE(x) sg_range{ &x, sizeof(x) }
#define SG_RANGE_REF(x) sg_range{ &x, sizeof(x) }
#else
#define SG_RANGE(x) (sg_range){ &x, sizeof(x) }
#define SG_RANGE_REF(x) &(sg_range){ &x, sizeof(x) }
#endif

//  various compile-time constants
enum {
    SG_INVALID_ID = 0,
    SG_NUM_SHADER_STAGES = 2,
    SG_NUM_INFLIGHT_FRAMES = 2,
    SG_MAX_COLOR_ATTACHMENTS = 4,
    SG_MAX_VERTEX_BUFFERS = 8,
    SG_MAX_SHADERSTAGE_IMAGES = 12,
    SG_MAX_SHADERSTAGE_SAMPLERS = 8,
    SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS = 12,
    SG_MAX_SHADERSTAGE_STORAGEBUFFERS = 8,
    SG_MAX_SHADERSTAGE_UBS = 4,
    SG_MAX_UB_MEMBERS = 16,
    SG_MAX_VERTEX_ATTRIBUTES = 16,
    SG_MAX_MIPMAPS = 16,
    SG_MAX_TEXTUREARRAY_LAYERS = 128
};

/*
    sg_color

    An RGBA color value.
*/
typedef struct sg_color { float r, g, b, a; } sg_color;

/*
    sg_backend

    The active 3D-API backend, use the function sg_query_backend()
    to get the currently active backend.
*/
typedef enum sg_backend {
    SG_BACKEND_GLCORE,
    SG_BACKEND_GLES3,
    SG_BACKEND_D3D11,
    SG_BACKEND_METAL_IOS,
    SG_BACKEND_METAL_MACOS,
    SG_BACKEND_METAL_SIMULATOR,
    SG_BACKEND_WGPU,
    SG_BACKEND_DUMMY,
} sg_backend;

/*
    sg_pixel_format

    sokol_gfx.h basically uses the same pixel formats as WebGPU, since these
    are supported on most newer GPUs.

    A pixelformat name consist of three parts:

        - components (R, RG, RGB or RGBA)
        - bit width per component (8, 16 or 32)
        - component data type:
            - unsigned normalized (no postfix)
            - signed normalized (SN postfix)
            - unsigned integer (UI postfix)
            - signed integer (SI postfix)
            - float (F postfix)

    Not all pixel formats can be used for everything, call sg_query_pixelformat()
    to inspect the capabilities of a given pixelformat. The function returns
    an sg_pixelformat_info struct with the following members:

        - sample: the pixelformat can be sampled as texture at least with
                  nearest filtering
        - filter: the pixelformat can be samples as texture with linear
                  filtering
        - render: the pixelformat can be used for render targets
        - blend:  blending is supported when using the pixelformat for
                  render targets
        - msaa:   multisample-antialiasing is supported when using the
                  pixelformat for render targets
        - depth:  the pixelformat can be used for depth-stencil attachments
        - compressed: this is a block-compressed format
        - bytes_per_pixel: the numbers of bytes in a pixel (0 for compressed formats)

    The default pixel format for texture images is SG_PIXELFORMAT_RGBA8.

    The default pixel format for render target images is platform-dependent
    and taken from the sg_environment struct passed into sg_setup(). Typically
    the default formats are:

        - for the Metal, D3D11 and WebGPU backends: SG_PIXELFORMAT_BGRA8
        - for GL backends: SG_PIXELFORMAT_RGBA8
*/
typedef enum sg_pixel_format {
    _SG_PIXELFORMAT_DEFAULT,    // value 0 reserved for default-init
    SG_PIXELFORMAT_NONE,

    SG_PIXELFORMAT_R8,
    SG_PIXELFORMAT_R8SN,
    SG_PIXELFORMAT_R8UI,
    SG_PIXELFORMAT_R8SI,

    SG_PIXELFORMAT_R16,
    SG_PIXELFORMAT_R16SN,
    SG_PIXELFORMAT_R16UI,
    SG_PIXELFORMAT_R16SI,
    SG_PIXELFORMAT_R16F,
    SG_PIXELFORMAT_RG8,
    SG_PIXELFORMAT_RG8SN,
    SG_PIXELFORMAT_RG8UI,
    SG_PIXELFORMAT_RG8SI,

    SG_PIXELFORMAT_R32UI,
    SG_PIXELFORMAT_R32SI,
    SG_PIXELFORMAT_R32F,
    SG_PIXELFORMAT_RG16,
    SG_PIXELFORMAT_RG16SN,
    SG_PIXELFORMAT_RG16UI,
    SG_PIXELFORMAT_RG16SI,
    SG_PIXELFORMAT_RG16F,
    SG_PIXELFORMAT_RGBA8,
    SG_PIXELFORMAT_SRGB8A8,
    SG_PIXELFORMAT_RGBA8SN,
    SG_PIXELFORMAT_RGBA8UI,
    SG_PIXELFORMAT_RGBA8SI,
    SG_PIXELFORMAT_BGRA8,
    SG_PIXELFORMAT_RGB10A2,
    SG_PIXELFORMAT_RG11B10F,
    SG_PIXELFORMAT_RGB9E5,

    SG_PIXELFORMAT_RG32UI,
    SG_PIXELFORMAT_RG32SI,
    SG_PIXELFORMAT_RG32F,
    SG_PIXELFORMAT_RGBA16,
    SG_PIXELFORMAT_RGBA16SN,
    SG_PIXELFORMAT_RGBA16UI,
    SG_PIXELFORMAT_RGBA16SI,
    SG_PIXELFORMAT_RGBA16F,

    SG_PIXELFORMAT_RGBA32UI,
    SG_PIXELFORMAT_RGBA32SI,
    SG_PIXELFORMAT_RGBA32F,

    // NOTE: when adding/removing pixel formats before DEPTH, also update sokol_app.h/_SAPP_PIXELFORMAT_*
    SG_PIXELFORMAT_DEPTH,
    SG_PIXELFORMAT_DEPTH_STENCIL,

    // NOTE: don't put any new compressed format in front of here
    SG_PIXELFORMAT_BC1_RGBA,
    SG_PIXELFORMAT_BC2_RGBA,
    SG_PIXELFORMAT_BC3_RGBA,
    SG_PIXELFORMAT_BC3_SRGBA,
    SG_PIXELFORMAT_BC4_R,
    SG_PIXELFORMAT_BC4_RSN,
    SG_PIXELFORMAT_BC5_RG,
    SG_PIXELFORMAT_BC5_RGSN,
    SG_PIXELFORMAT_BC6H_RGBF,
    SG_PIXELFORMAT_BC6H_RGBUF,
    SG_PIXELFORMAT_BC7_RGBA,
    SG_PIXELFORMAT_BC7_SRGBA,
    SG_PIXELFORMAT_PVRTC_RGB_2BPP,      // FIXME: deprecated
    SG_PIXELFORMAT_PVRTC_RGB_4BPP,      // FIXME: deprecated
    SG_PIXELFORMAT_PVRTC_RGBA_2BPP,     // FIXME: deprecated
    SG_PIXELFORMAT_PVRTC_RGBA_4BPP,     // FIXME: deprecated
    SG_PIXELFORMAT_ETC2_RGB8,
    SG_PIXELFORMAT_ETC2_SRGB8,
    SG_PIXELFORMAT_ETC2_RGB8A1,
    SG_PIXELFORMAT_ETC2_RGBA8,
    SG_PIXELFORMAT_ETC2_SRGB8A8,
    SG_PIXELFORMAT_EAC_R11,
    SG_PIXELFORMAT_EAC_R11SN,
    SG_PIXELFORMAT_EAC_RG11,
    SG_PIXELFORMAT_EAC_RG11SN,

    SG_PIXELFORMAT_ASTC_4x4_RGBA,
    SG_PIXELFORMAT_ASTC_4x4_SRGBA,

    _SG_PIXELFORMAT_NUM,
    _SG_PIXELFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_pixel_format;

/*
    Runtime information about a pixel format, returned
    by sg_query_pixelformat().
*/
typedef struct sg_pixelformat_info {
    bool sample;            // pixel format can be sampled in shaders at least with nearest filtering
    bool filter;            // pixel format can be sampled with linear filtering
    bool render;            // pixel format can be used as render target
    bool blend;             // alpha-blending is supported
    bool msaa;              // pixel format can be used as MSAA render target
    bool depth;             // pixel format is a depth format
    bool compressed;        // true if this is a hardware-compressed format
    int bytes_per_pixel;    // NOTE: this is 0 for compressed formats, use sg_query_row_pitch() / sg_query_surface_pitch() as alternative
} sg_pixelformat_info;

/*
    Runtime information about available optional features,
    returned by sg_query_features()
*/
typedef struct sg_features {
    bool origin_top_left;               // framebuffer and texture origin is in top left corner
    bool image_clamp_to_border;         // border color and clamp-to-border UV-wrap mode is supported
    bool mrt_independent_blend_state;   // multiple-render-target rendering can use per-render-target blend state
    bool mrt_independent_write_mask;    // multiple-render-target rendering can use per-render-target color write masks
    bool storage_buffer;                // storage buffers are supported
} sg_features;

/*
    Runtime information about resource limits, returned by sg_query_limit()
*/
typedef struct sg_limits {
    int max_image_size_2d;          // max width/height of SG_IMAGETYPE_2D images
    int max_image_size_cube;        // max width/height of SG_IMAGETYPE_CUBE images
    int max_image_size_3d;          // max width/height/depth of SG_IMAGETYPE_3D images
    int max_image_size_array;       // max width/height of SG_IMAGETYPE_ARRAY images
    int max_image_array_layers;     // max number of layers in SG_IMAGETYPE_ARRAY images
    int max_vertex_attrs;           // max number of vertex attributes, clamped to SG_MAX_VERTEX_ATTRIBUTES
    int gl_max_vertex_uniform_components;    // <= GL_MAX_VERTEX_UNIFORM_COMPONENTS (only on GL backends)
    int gl_max_combined_texture_image_units; // <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS (only on GL backends)
} sg_limits;

/*
    sg_resource_state

    The current state of a resource in its resource pool.
    Resources start in the INITIAL state, which means the
    pool slot is unoccupied and can be allocated. When a resource is
    created, first an id is allocated, and the resource pool slot
    is set to state ALLOC. After allocation, the resource is
    initialized, which may result in the VALID or FAILED state. The
    reason why allocation and initialization are separate is because
    some resource types (e.g. buffers and images) might be asynchronously
    initialized by the user application. If a resource which is not
    in the VALID state is attempted to be used for rendering, rendering
    operations will silently be dropped.

    The special INVALID state is returned in sg_query_xxx_state() if no
    resource object exists for the provided resource id.
*/
typedef enum sg_resource_state {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID,
    _SG_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} sg_resource_state;

/*
    sg_usage

    A resource usage hint describing the update strategy of
    buffers and images. This is used in the sg_buffer_desc.usage
    and sg_image_desc.usage members when creating buffers
    and images:

    SG_USAGE_IMMUTABLE:     the resource will never be updated with
                            new data, instead the content of the
                            resource must be provided on creation
    SG_USAGE_DYNAMIC:       the resource will be updated infrequently
                            with new data (this could range from "once
                            after creation", to "quite often but not
                            every frame")
    SG_USAGE_STREAM:        the resource will be updated each frame
                            with new content

    The rendering backends use this hint to prevent that the
    CPU needs to wait for the GPU when attempting to update
    a resource that might be currently accessed by the GPU.

    Resource content is updated with the functions sg_update_buffer() or
    sg_append_buffer() for buffer objects, and sg_update_image() for image
    objects. For the sg_update_*() functions, only one update is allowed per
    frame and resource object, while sg_append_buffer() can be called
    multiple times per frame on the same buffer. The application must update
    all data required for rendering (this means that the update data can be
    smaller than the resource size, if only a part of the overall resource
    size is used for rendering, you only need to make sure that the data that
    *is* used is valid).

    The default usage is SG_USAGE_IMMUTABLE.
*/
typedef enum sg_usage {
    _SG_USAGE_DEFAULT,      // value 0 reserved for default-init
    SG_USAGE_IMMUTABLE,
    SG_USAGE_DYNAMIC,
    SG_USAGE_STREAM,
    _SG_USAGE_NUM,
    _SG_USAGE_FORCE_U32 = 0x7FFFFFFF
} sg_usage;

/*
    sg_buffer_type

    Indicates whether a buffer will be bound as vertex-,
    index- or storage-buffer.

    Used in the sg_buffer_desc.type member when creating a buffer.

    The default value is SG_BUFFERTYPE_VERTEXBUFFER.
*/
typedef enum sg_buffer_type {
    _SG_BUFFERTYPE_DEFAULT,         // value 0 reserved for default-init
    SG_BUFFERTYPE_VERTEXBUFFER,
    SG_BUFFERTYPE_INDEXBUFFER,
    SG_BUFFERTYPE_STORAGEBUFFER,
    _SG_BUFFERTYPE_NUM,
    _SG_BUFFERTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_buffer_type;

/*
    sg_index_type

    Indicates whether indexed rendering (fetching vertex-indices from an
    index buffer) is used, and if yes, the index data type (16- or 32-bits).
    This is used in the sg_pipeline_desc.index_type member when creating a
    pipeline object.

    The default index type is SG_INDEXTYPE_NONE.
*/
typedef enum sg_index_type {
    _SG_INDEXTYPE_DEFAULT,   // value 0 reserved for default-init
    SG_INDEXTYPE_NONE,
    SG_INDEXTYPE_UINT16,
    SG_INDEXTYPE_UINT32,
    _SG_INDEXTYPE_NUM,
    _SG_INDEXTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_index_type;

/*
    sg_image_type

    Indicates the basic type of an image object (2D-texture, cubemap,
    3D-texture or 2D-array-texture). Used in the sg_image_desc.type member when
    creating an image, and in sg_shader_image_desc to describe a sampled texture
    in the shader (both must match and will be checked in the validation layer
    when calling sg_apply_bindings).

    The default image type when creating an image is SG_IMAGETYPE_2D.
*/
typedef enum sg_image_type {
    _SG_IMAGETYPE_DEFAULT,  // value 0 reserved for default-init
    SG_IMAGETYPE_2D,
    SG_IMAGETYPE_CUBE,
    SG_IMAGETYPE_3D,
    SG_IMAGETYPE_ARRAY,
    _SG_IMAGETYPE_NUM,
    _SG_IMAGETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_image_type;

/*
    sg_image_sample_type

    The basic data type of a texture sample as expected by a shader.
    Must be provided in sg_shader_image_desc and used by the validation
    layer in sg_apply_bindings() to check if the provided image object
    is compatible with what the shader expects. Apart from the sokol-gfx
    validation layer, WebGPU is the only backend API which actually requires
    matching texture and sampler type to be provided upfront for validation
    (other 3D APIs treat texture/sampler type mismatches as undefined behaviour).

    NOTE that the following texture pixel formats require the use
    of SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT, combined with a sampler
    of type SG_SAMPLERTYPE_NONFILTERING:

    - SG_PIXELFORMAT_R32F
    - SG_PIXELFORMAT_RG32F
    - SG_PIXELFORMAT_RGBA32F

    (when using sokol-shdc, also check out the meta tags `@image_sample_type`
    and `@sampler_type`)
*/
typedef enum sg_image_sample_type {
    _SG_IMAGESAMPLETYPE_DEFAULT,  // value 0 reserved for default-init
    SG_IMAGESAMPLETYPE_FLOAT,
    SG_IMAGESAMPLETYPE_DEPTH,
    SG_IMAGESAMPLETYPE_SINT,
    SG_IMAGESAMPLETYPE_UINT,
    SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT,
    _SG_IMAGESAMPLETYPE_NUM,
    _SG_IMAGESAMPLETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_image_sample_type;

/*
    sg_sampler_type

    The basic type of a texture sampler (sampling vs comparison) as
    defined in a shader. Must be provided in sg_shader_sampler_desc.

    sg_image_sample_type and sg_sampler_type for a texture/sampler
    pair must be compatible with each other, specifically only
    the following pairs are allowed:

    - SG_IMAGESAMPLETYPE_FLOAT => (SG_SAMPLERTYPE_FILTERING or SG_SAMPLERTYPE_NONFILTERING)
    - SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT => SG_SAMPLERTYPE_NONFILTERING
    - SG_IMAGESAMPLETYPE_SINT => SG_SAMPLERTYPE_NONFILTERING
    - SG_IMAGESAMPLETYPE_UINT => SG_SAMPLERTYPE_NONFILTERING
    - SG_IMAGESAMPLETYPE_DEPTH => SG_SAMPLERTYPE_COMPARISON
*/
typedef enum sg_sampler_type {
    _SG_SAMPLERTYPE_DEFAULT,
    SG_SAMPLERTYPE_FILTERING,
    SG_SAMPLERTYPE_NONFILTERING,
    SG_SAMPLERTYPE_COMPARISON,
    _SG_SAMPLERTYPE_NUM,
    _SG_SAMPLERTYPE_FORCE_U32,
} sg_sampler_type;

/*
    sg_cube_face

    The cubemap faces. Use these as indices in the sg_image_desc.content
    array.
*/
typedef enum sg_cube_face {
    SG_CUBEFACE_POS_X,
    SG_CUBEFACE_NEG_X,
    SG_CUBEFACE_POS_Y,
    SG_CUBEFACE_NEG_Y,
    SG_CUBEFACE_POS_Z,
    SG_CUBEFACE_NEG_Z,
    SG_CUBEFACE_NUM,
    _SG_CUBEFACE_FORCE_U32 = 0x7FFFFFFF
} sg_cube_face;

/*
    sg_shader_stage

    There are 2 shader stages: vertex- and fragment-shader-stage.
    Each shader stage

    - SG_MAX_SHADERSTAGE_UBS slots for applying uniform data
    - SG_MAX_SHADERSTAGE_IMAGES slots for images used as textures
    - SG_MAX_SHADERSTAGE_SAMPLERS slots for texture samplers
    - SG_MAX_SHADERSTAGE_STORAGEBUFFERS slots for storage buffer bindings
*/
typedef enum sg_shader_stage {
    SG_SHADERSTAGE_VS,
    SG_SHADERSTAGE_FS,
    _SG_SHADERSTAGE_FORCE_U32 = 0x7FFFFFFF
} sg_shader_stage;

/*
    sg_primitive_type

    This is the common subset of 3D primitive types supported across all 3D
    APIs. This is used in the sg_pipeline_desc.primitive_type member when
    creating a pipeline object.

    The default primitive type is SG_PRIMITIVETYPE_TRIANGLES.
*/
typedef enum sg_primitive_type {
    _SG_PRIMITIVETYPE_DEFAULT,  // value 0 reserved for default-init
    SG_PRIMITIVETYPE_POINTS,
    SG_PRIMITIVETYPE_LINES,
    SG_PRIMITIVETYPE_LINE_STRIP,
    SG_PRIMITIVETYPE_TRIANGLES,
    SG_PRIMITIVETYPE_TRIANGLE_STRIP,
    _SG_PRIMITIVETYPE_NUM,
    _SG_PRIMITIVETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_primitive_type;

/*
    sg_filter

    The filtering mode when sampling a texture image. This is
    used in the sg_sampler_desc.min_filter, sg_sampler_desc.mag_filter
    and sg_sampler_desc.mipmap_filter members when creating a sampler object.

    For min_filter and mag_filter the default is SG_FILTER_NEAREST.

    For mipmap_filter the default is SG_FILTER_NONE.
*/
typedef enum sg_filter {
    _SG_FILTER_DEFAULT, // value 0 reserved for default-init
    SG_FILTER_NONE,     // FIXME: deprecated
    SG_FILTER_NEAREST,
    SG_FILTER_LINEAR,
    _SG_FILTER_NUM,
    _SG_FILTER_FORCE_U32 = 0x7FFFFFFF
} sg_filter;

/*
    sg_wrap

    The texture coordinates wrapping mode when sampling a texture
    image. This is used in the sg_image_desc.wrap_u, .wrap_v
    and .wrap_w members when creating an image.

    The default wrap mode is SG_WRAP_REPEAT.

    NOTE: SG_WRAP_CLAMP_TO_BORDER is not supported on all backends
    and platforms. To check for support, call sg_query_features()
    and check the "clamp_to_border" boolean in the returned
    sg_features struct.

    Platforms which don't support SG_WRAP_CLAMP_TO_BORDER will silently fall back
    to SG_WRAP_CLAMP_TO_EDGE without a validation error.
*/
typedef enum sg_wrap {
    _SG_WRAP_DEFAULT,   // value 0 reserved for default-init
    SG_WRAP_REPEAT,
    SG_WRAP_CLAMP_TO_EDGE,
    SG_WRAP_CLAMP_TO_BORDER,
    SG_WRAP_MIRRORED_REPEAT,
    _SG_WRAP_NUM,
    _SG_WRAP_FORCE_U32 = 0x7FFFFFFF
} sg_wrap;

/*
    sg_border_color

    The border color to use when sampling a texture, and the UV wrap
    mode is SG_WRAP_CLAMP_TO_BORDER.

    The default border color is SG_BORDERCOLOR_OPAQUE_BLACK
*/
typedef enum sg_border_color {
    _SG_BORDERCOLOR_DEFAULT,    // value 0 reserved for default-init
    SG_BORDERCOLOR_TRANSPARENT_BLACK,
    SG_BORDERCOLOR_OPAQUE_BLACK,
    SG_BORDERCOLOR_OPAQUE_WHITE,
    _SG_BORDERCOLOR_NUM,
    _SG_BORDERCOLOR_FORCE_U32 = 0x7FFFFFFF
} sg_border_color;

/*
    sg_vertex_format

    The data type of a vertex component. This is used to describe
    the layout of vertex data when creating a pipeline object.
*/
typedef enum sg_vertex_format {
    SG_VERTEXFORMAT_INVALID,
    SG_VERTEXFORMAT_FLOAT,
    SG_VERTEXFORMAT_FLOAT2,
    SG_VERTEXFORMAT_FLOAT3,
    SG_VERTEXFORMAT_FLOAT4,
    SG_VERTEXFORMAT_BYTE4,
    SG_VERTEXFORMAT_BYTE4N,
    SG_VERTEXFORMAT_UBYTE4,
    SG_VERTEXFORMAT_UBYTE4N,
    SG_VERTEXFORMAT_SHORT2,
    SG_VERTEXFORMAT_SHORT2N,
    SG_VERTEXFORMAT_USHORT2N,
    SG_VERTEXFORMAT_SHORT4,
    SG_VERTEXFORMAT_SHORT4N,
    SG_VERTEXFORMAT_USHORT4N,
    SG_VERTEXFORMAT_UINT10_N2,
    SG_VERTEXFORMAT_HALF2,
    SG_VERTEXFORMAT_HALF4,
    SG_VERTEXFORMAT_UINT,
    _SG_VERTEXFORMAT_NUM,
    _SG_VERTEXFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_format;

/*
    sg_vertex_step

    Defines whether the input pointer of a vertex input stream is advanced
    'per vertex' or 'per instance'. The default step-func is
    SG_VERTEXSTEP_PER_VERTEX. SG_VERTEXSTEP_PER_INSTANCE is used with
    instanced-rendering.

    The vertex-step is part of the vertex-layout definition
    when creating pipeline objects.
*/
typedef enum sg_vertex_step {
    _SG_VERTEXSTEP_DEFAULT,     // value 0 reserved for default-init
    SG_VERTEXSTEP_PER_VERTEX,
    SG_VERTEXSTEP_PER_INSTANCE,
    _SG_VERTEXSTEP_NUM,
    _SG_VERTEXSTEP_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_step;

/*
    sg_uniform_type

    The data type of a uniform block member. This is used to
    describe the internal layout of uniform blocks when creating
    a shader object.
*/
typedef enum sg_uniform_type {
    SG_UNIFORMTYPE_INVALID,
    SG_UNIFORMTYPE_FLOAT,
    SG_UNIFORMTYPE_FLOAT2,
    SG_UNIFORMTYPE_FLOAT3,
    SG_UNIFORMTYPE_FLOAT4,
    SG_UNIFORMTYPE_INT,
    SG_UNIFORMTYPE_INT2,
    SG_UNIFORMTYPE_INT3,
    SG_UNIFORMTYPE_INT4,
    SG_UNIFORMTYPE_MAT4,
    _SG_UNIFORMTYPE_NUM,
    _SG_UNIFORMTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_type;

/*
    sg_uniform_layout

    A hint for the interior memory layout of uniform blocks. This is
    only really relevant for the GL backend where the internal layout
    of uniform blocks must be known to sokol-gfx. For all other backends the
    internal memory layout of uniform blocks doesn't matter, sokol-gfx
    will just pass uniform data as a single memory blob to the
    3D backend.

    SG_UNIFORMLAYOUT_NATIVE (default)
        Native layout means that a 'backend-native' memory layout
        is used. For the GL backend this means that uniforms
        are packed tightly in memory (e.g. there are no padding
        bytes).

    SG_UNIFORMLAYOUT_STD140
        The memory layout is a subset of std140. Arrays are only
        allowed for the FLOAT4, INT4 and MAT4. Alignment is as
        is as follows:

            FLOAT, INT:         4 byte alignment
            FLOAT2, INT2:       8 byte alignment
            FLOAT3, INT3:       16 byte alignment(!)
            FLOAT4, INT4:       16 byte alignment
            MAT4:               16 byte alignment
            FLOAT4[], INT4[]:   16 byte alignment

        The overall size of the uniform block must be a multiple
        of 16.

    For more information search for 'UNIFORM DATA LAYOUT' in the documentation block
    at the start of the header.
*/
typedef enum sg_uniform_layout {
    _SG_UNIFORMLAYOUT_DEFAULT,     // value 0 reserved for default-init
    SG_UNIFORMLAYOUT_NATIVE,       // default: layout depends on currently active backend
    SG_UNIFORMLAYOUT_STD140,       // std140: memory layout according to std140
    _SG_UNIFORMLAYOUT_NUM,
    _SG_UNIFORMLAYOUT_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_layout;

/*
    sg_cull_mode

    The face-culling mode, this is used in the
    sg_pipeline_desc.cull_mode member when creating a
    pipeline object.

    The default cull mode is SG_CULLMODE_NONE
*/
typedef enum sg_cull_mode {
    _SG_CULLMODE_DEFAULT,   // value 0 reserved for default-init
    SG_CULLMODE_NONE,
    SG_CULLMODE_FRONT,
    SG_CULLMODE_BACK,
    _SG_CULLMODE_NUM,
    _SG_CULLMODE_FORCE_U32 = 0x7FFFFFFF
} sg_cull_mode;

/*
    sg_face_winding

    The vertex-winding rule that determines a front-facing primitive. This
    is used in the member sg_pipeline_desc.face_winding
    when creating a pipeline object.

    The default winding is SG_FACEWINDING_CW (clockwise)
*/
typedef enum sg_face_winding {
    _SG_FACEWINDING_DEFAULT,    // value 0 reserved for default-init
    SG_FACEWINDING_CCW,
    SG_FACEWINDING_CW,
    _SG_FACEWINDING_NUM,
    _SG_FACEWINDING_FORCE_U32 = 0x7FFFFFFF
} sg_face_winding;

/*
    sg_compare_func

    The compare-function for configuring depth- and stencil-ref tests
    in pipeline objects, and for texture samplers which perform a comparison
    instead of regular sampling operation.

    sg_pipeline_desc
        .depth
            .compare
        .stencil
            .front.compare
            .back.compar

    sg_sampler_desc
        .compare

    The default compare func for depth- and stencil-tests is
    SG_COMPAREFUNC_ALWAYS.

    The default compare func for sampler is SG_COMPAREFUNC_NEVER.
*/
typedef enum sg_compare_func {
    _SG_COMPAREFUNC_DEFAULT,    // value 0 reserved for default-init
    SG_COMPAREFUNC_NEVER,
    SG_COMPAREFUNC_LESS,
    SG_COMPAREFUNC_EQUAL,
    SG_COMPAREFUNC_LESS_EQUAL,
    SG_COMPAREFUNC_GREATER,
    SG_COMPAREFUNC_NOT_EQUAL,
    SG_COMPAREFUNC_GREATER_EQUAL,
    SG_COMPAREFUNC_ALWAYS,
    _SG_COMPAREFUNC_NUM,
    _SG_COMPAREFUNC_FORCE_U32 = 0x7FFFFFFF
} sg_compare_func;

/*
    sg_stencil_op

    The operation performed on a currently stored stencil-value when a
    comparison test passes or fails. This is used when creating a pipeline
    object in the members:

    sg_pipeline_desc
        .stencil
            .front
                .fail_op
                .depth_fail_op
                .pass_op
            .back
                .fail_op
                .depth_fail_op
                .pass_op

    The default value is SG_STENCILOP_KEEP.
*/
typedef enum sg_stencil_op {
    _SG_STENCILOP_DEFAULT,      // value 0 reserved for default-init
    SG_STENCILOP_KEEP,
    SG_STENCILOP_ZERO,
    SG_STENCILOP_REPLACE,
    SG_STENCILOP_INCR_CLAMP,
    SG_STENCILOP_DECR_CLAMP,
    SG_STENCILOP_INVERT,
    SG_STENCILOP_INCR_WRAP,
    SG_STENCILOP_DECR_WRAP,
    _SG_STENCILOP_NUM,
    _SG_STENCILOP_FORCE_U32 = 0x7FFFFFFF
} sg_stencil_op;

/*
    sg_blend_factor

    The source and destination factors in blending operations.
    This is used in the following members when creating a pipeline object:

    sg_pipeline_desc
        .colors[i]
            .blend
                .src_factor_rgb
                .dst_factor_rgb
                .src_factor_alpha
                .dst_factor_alpha

    The default value is SG_BLENDFACTOR_ONE for source
    factors, and SG_BLENDFACTOR_ZERO for destination factors.
*/
typedef enum sg_blend_factor {
    _SG_BLENDFACTOR_DEFAULT,    // value 0 reserved for default-init
    SG_BLENDFACTOR_ZERO,
    SG_BLENDFACTOR_ONE,
    SG_BLENDFACTOR_SRC_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SG_BLENDFACTOR_SRC_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SG_BLENDFACTOR_DST_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SG_BLENDFACTOR_DST_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SG_BLENDFACTOR_SRC_ALPHA_SATURATED,
    SG_BLENDFACTOR_BLEND_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
    SG_BLENDFACTOR_BLEND_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
    _SG_BLENDFACTOR_NUM,
    _SG_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
} sg_blend_factor;

/*
    sg_blend_op

    Describes how the source and destination values are combined in the
    fragment blending operation. It is used in the following members when
    creating a pipeline object:

    sg_pipeline_desc
        .colors[i]
            .blend
                .op_rgb
                .op_alpha

    The default value is SG_BLENDOP_ADD.
*/
typedef enum sg_blend_op {
    _SG_BLENDOP_DEFAULT,    // value 0 reserved for default-init
    SG_BLENDOP_ADD,
    SG_BLENDOP_SUBTRACT,
    SG_BLENDOP_REVERSE_SUBTRACT,
    _SG_BLENDOP_NUM,
    _SG_BLENDOP_FORCE_U32 = 0x7FFFFFFF
} sg_blend_op;

/*
    sg_color_mask

    Selects the active color channels when writing a fragment color to the
    framebuffer. This is used in the members
    sg_pipeline_desc.colors[i].write_mask when creating a pipeline object.

    The default colormask is SG_COLORMASK_RGBA (write all colors channels)

    NOTE: since the color mask value 0 is reserved for the default value
    (SG_COLORMASK_RGBA), use SG_COLORMASK_NONE if all color channels
    should be disabled.
*/
typedef enum sg_color_mask {
    _SG_COLORMASK_DEFAULT = 0,    // value 0 reserved for default-init
    SG_COLORMASK_NONE   = 0x10,   // special value for 'all channels disabled
    SG_COLORMASK_R      = 0x1,
    SG_COLORMASK_G      = 0x2,
    SG_COLORMASK_RG     = 0x3,
    SG_COLORMASK_B      = 0x4,
    SG_COLORMASK_RB     = 0x5,
    SG_COLORMASK_GB     = 0x6,
    SG_COLORMASK_RGB    = 0x7,
    SG_COLORMASK_A      = 0x8,
    SG_COLORMASK_RA     = 0x9,
    SG_COLORMASK_GA     = 0xA,
    SG_COLORMASK_RGA    = 0xB,
    SG_COLORMASK_BA     = 0xC,
    SG_COLORMASK_RBA    = 0xD,
    SG_COLORMASK_GBA    = 0xE,
    SG_COLORMASK_RGBA   = 0xF,
    _SG_COLORMASK_FORCE_U32 = 0x7FFFFFFF
} sg_color_mask;

/*
    sg_load_action

    Defines the load action that should be performed at the start of a render pass:

    SG_LOADACTION_CLEAR:        clear the render target
    SG_LOADACTION_LOAD:         load the previous content of the render target
    SG_LOADACTION_DONTCARE:     leave the render target in an undefined state

    This is used in the sg_pass_action structure.

    The default load action for all pass attachments is SG_LOADACTION_CLEAR,
    with the values rgba = { 0.5f, 0.5f, 0.5f, 1.0f }, depth=1.0f and stencil=0.

    If you want to override the default behaviour, it is important to not
    only set the clear color, but the 'action' field as well (as long as this
    is _SG_LOADACTION_DEFAULT, the value fields will be ignored).
*/
typedef enum sg_load_action {
    _SG_LOADACTION_DEFAULT,
    SG_LOADACTION_CLEAR,
    SG_LOADACTION_LOAD,
    SG_LOADACTION_DONTCARE,
    _SG_LOADACTION_FORCE_U32 = 0x7FFFFFFF
} sg_load_action;

/*
    sg_store_action

    Defines the store action that be performed at the end of a render pass:

    SG_STOREACTION_STORE:       store the rendered content to the color attachment image
    SG_STOREACTION_DONTCARE:    allows the GPU to discard the rendered content
*/
typedef enum sg_store_action {
    _SG_STOREACTION_DEFAULT,
    SG_STOREACTION_STORE,
    SG_STOREACTION_DONTCARE,
    _SG_STOREACTION_FORCE_U32 = 0x7FFFFFFF
} sg_store_action;


/*
    sg_pass_action

    The sg_pass_action struct defines the actions to be performed
    at the start and end of a render pass.

    - at the start of the pass: whether the render targets should be cleared,
      loaded with their previous content, or start in an undefined state
    - for clear operations: the clear value (color, depth, or stencil values)
    - at the end of the pass: whether the rendering result should be
      stored back into the render target or discarded
*/
typedef struct sg_color_attachment_action {
    sg_load_action load_action;         // default: SG_LOADACTION_CLEAR
    sg_store_action store_action;       // default: SG_STOREACTION_STORE
    sg_color clear_value;               // default: { 0.5f, 0.5f, 0.5f, 1.0f }
} sg_color_attachment_action;

typedef struct sg_depth_attachment_action {
    sg_load_action load_action;         // default: SG_LOADACTION_CLEAR
    sg_store_action store_action;       // default: SG_STOREACTION_DONTCARE
    float clear_value;                  // default: 1.0
} sg_depth_attachment_action;

typedef struct sg_stencil_attachment_action {
    sg_load_action load_action;         // default: SG_LOADACTION_CLEAR
    sg_store_action store_action;       // default: SG_STOREACTION_DONTCARE
    uint8_t clear_value;                // default: 0
} sg_stencil_attachment_action;

typedef struct sg_pass_action {
    sg_color_attachment_action colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_depth_attachment_action depth;
    sg_stencil_attachment_action stencil;
} sg_pass_action;

/*
    sg_swapchain

    Used in sg_begin_pass() to provide details about an external swapchain
    (pixel formats, sample count and backend-API specific render surface objects).

    The following information must be provided:

    - the width and height of the swapchain surfaces in number of pixels,
    - the pixel format of the render- and optional msaa-resolve-surface
    - the pixel format of the optional depth- or depth-stencil-surface
    - the MSAA sample count for the render and depth-stencil surface

    If the pixel formats and MSAA sample counts are left zero-initialized,
    their defaults are taken from the sg_environment struct provided in the
    sg_setup() call.

    The width and height *must* be > 0.

    Additionally the following backend API specific objects must be passed in
    as 'type erased' void pointers:

    GL: on all GL backends, a GL framebuffer object must be provided. This
    can be zero for the default framebuffer.

    D3D11:
        - an ID3D11RenderTargetView for the rendering surface, without
          MSAA rendering this surface will also be displayed
        - an optional ID3D11DepthStencilView for the depth- or depth/stencil
          buffer surface
        - when MSAA rendering is used, another ID3D11RenderTargetView
          which serves as MSAA resolve target and will be displayed

    WebGPU (same as D3D11, except different types)
        - a WGPUTextureView for the rendering surface, without
          MSAA rendering this surface will also be displayed
        - an optional WGPUTextureView for the depth- or depth/stencil
          buffer surface
        - when MSAA rendering is used, another WGPUTextureView
          which serves as MSAA resolve target and will be displayed

    Metal (NOTE that the rolves of provided surfaces is slightly different
    than on D3D11 or WebGPU in case of MSAA vs non-MSAA rendering):

        - A current CAMetalDrawable (NOT an MTLDrawable!) which will be presented.
          This will either be rendered to directly (if no MSAA is used), or serve
          as MSAA-resolve target.
        - an optional MTLTexture for the depth- or depth-stencil buffer
        - an optional multisampled MTLTexture which serves as intermediate
          rendering surface which will then be resolved into the
          CAMetalDrawable.

    NOTE that for Metal you must use an ObjC __bridge cast to
    properly tunnel the ObjC object handle through a C void*, e.g.:

        swapchain.metal.current_drawable = (__bridge const void*) [mtkView currentDrawable];

    On all other backends you shouldn't need to mess with the reference count.

    It's a good practice to write a helper function which returns an initialized
    sg_swapchain structs, which can then be plugged directly into
    sg_pass.swapchain. Look at the function sglue_swapchain() in the sokol_glue.h
    as an example.
*/

typedef struct sg_gl_swapchain {
    uint32_t framebuffer;               // GL framebuffer object
} sg_gl_swapchain;

typedef struct sg_swapchain {
    int width;
    int height;
    int sample_count;
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    sg_gl_swapchain gl;
} sg_swapchain;

/*
    sg_pass

    The sg_pass structure is passed as argument into the sg_begin_pass()
    function.

    For an offscreen rendering pass, an sg_pass_action struct and sg_attachments
    object must be provided, and for swapchain passes, and sg_pass_action and
    an sg_swapchain struct. It is an error to provide both an sg_attachments
    handle and an initialized sg_swapchain struct in the same sg_begin_pass().

    An sg_begin_pass() call for an offscreen pass would look like this (where
    `attachments` is an sg_attachments handle):

        sg_begin_pass(&(sg_pass){
            .action = { ... },
            .attachments = attachments,
        });

    ...and a swapchain render pass would look like this (using the sokol_glue.h
    helper function sglue_swapchain() which gets the swapchain properties from
    sokol_app.h):

        sg_begin_pass(&(sg_pass){
            .action = { ... },
            .swapchain = sglue_swapchain(),
        });

    You can also omit the .action object to get default pass action behaviour
    (clear to color=grey, depth=1 and stencil=0).
*/
typedef struct sg_pass {
    uint32_t _start_canary;
    sg_pass_action action;
    sg_attachments attachments;
    sg_swapchain swapchain;
    const char* label;
    uint32_t _end_canary;
} sg_pass;

/*
    sg_bindings

    The sg_bindings structure defines the resource binding slots
    of the sokol_gfx render pipeline, used as argument to the
    sg_apply_bindings() function.

    A resource binding struct contains:

    - 1..N vertex buffers
    - 0..N vertex buffer offsets
    - 0..1 index buffers
    - 0..1 index buffer offsets
    - 0..N vertex shader stage images
    - 0..N vertex shader stage samplers
    - 0..N vertex shader storage buffers
    - 0..N fragment shader stage images
    - 0..N fragment shader stage samplers
    - 0..N fragment shader storage buffers

    For the max number of bindings, see the constant definitions:

    - SG_MAX_VERTEX_BUFFERS
    - SG_MAX_SHADERSTAGE_IMAGES
    - SG_MAX_SHADERSTAGE_SAMPLERS
    - SG_MAX_SHADERSTAGE_STORAGEBUFFERS

    The optional buffer offsets can be used to put different unrelated
    chunks of vertex- and/or index-data into the same buffer objects.
*/
typedef struct sg_stage_bindings {
    sg_image images[SG_MAX_SHADERSTAGE_IMAGES];
    sg_sampler samplers[SG_MAX_SHADERSTAGE_SAMPLERS];
    sg_buffer storage_buffers[SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
} sg_stage_bindings;

typedef struct sg_bindings {
    uint32_t _start_canary;
    sg_buffer vertex_buffers[SG_MAX_VERTEX_BUFFERS];
    int vertex_buffer_offsets[SG_MAX_VERTEX_BUFFERS];
    sg_buffer index_buffer;
    int index_buffer_offset;
    sg_stage_bindings vs;
    sg_stage_bindings fs;
    uint32_t _end_canary;
} sg_bindings;

/*
    sg_buffer_desc

    Creation parameters for sg_buffer objects, used in the
    sg_make_buffer() call.

    The default configuration is:

    .size:      0       (*must* be >0 for buffers without data)
    .type:      SG_BUFFERTYPE_VERTEXBUFFER
    .usage:     SG_USAGE_IMMUTABLE
    .data.ptr   0       (*must* be valid for immutable buffers)
    .data.size  0       (*must* be > 0 for immutable buffers)
    .label      0       (optional string label)

    For immutable buffers which are initialized with initial data,
    keep the .size item zero-initialized, and set the size together with the
    pointer to the initial data in the .data item.

    For mutable buffers without initial data, keep the .data item
    zero-initialized, and set the buffer size in the .size item instead.

    You can also set both size values, but currently both size values must
    be identical (this may change in the future when the dynamic resource
    management may become more flexible).

    ADVANCED TOPIC: Injecting native 3D-API buffers:

    The following struct members allow to inject your own GL, Metal
    or D3D11 buffers into sokol_gfx:

    .gl_buffers[SG_NUM_INFLIGHT_FRAMES]
    .mtl_buffers[SG_NUM_INFLIGHT_FRAMES]
    .d3d11_buffer

    You must still provide all other struct items except the .data item, and
    these must match the creation parameters of the native buffers you
    provide. For SG_USAGE_IMMUTABLE, only provide a single native 3D-API
    buffer, otherwise you need to provide SG_NUM_INFLIGHT_FRAMES buffers
    (only for GL and Metal, not D3D11). Providing multiple buffers for GL and
    Metal is necessary because sokol_gfx will rotate through them when
    calling sg_update_buffer() to prevent lock-stalls.

    Note that it is expected that immutable injected buffer have already been
    initialized with content, and the .content member must be 0!

    Also you need to call sg_reset_state_cache() after calling native 3D-API
    functions, and before calling any sokol_gfx function.
*/
typedef struct sg_buffer_desc {
    uint32_t _start_canary;
    size_t size;
    sg_buffer_type type;
    sg_usage usage;
    sg_range data;
    const char* label;
    // optionally inject backend-specific resources
    uint32_t gl_buffers[SG_NUM_INFLIGHT_FRAMES];
    uint32_t _end_canary;
} sg_buffer_desc;

/*
    sg_image_data

    Defines the content of an image through a 2D array of sg_range structs.
    The first array dimension is the cubemap face, and the second array
    dimension the mipmap level.
*/
typedef struct sg_image_data {
    sg_range subimage[SG_CUBEFACE_NUM][SG_MAX_MIPMAPS];
} sg_image_data;

/*
    sg_image_desc

    Creation parameters for sg_image objects, used in the sg_make_image() call.

    The default configuration is:

    .type:              SG_IMAGETYPE_2D
    .render_target:     false
    .width              0 (must be set to >0)
    .height             0 (must be set to >0)
    .num_slices         1 (3D textures: depth; array textures: number of layers)
    .num_mipmaps:       1
    .usage:             SG_USAGE_IMMUTABLE
    .pixel_format:      SG_PIXELFORMAT_RGBA8 for textures, or sg_desc.environment.defaults.color_format for render targets
    .sample_count:      1 for textures, or sg_desc.environment.defaults.sample_count for render targets
    .data               an sg_image_data struct to define the initial content
    .label              0 (optional string label for trace hooks)

    Q: Why is the default sample_count for render targets identical with the
    "default sample count" from sg_desc.environment.defaults.sample_count?

    A: So that it matches the default sample count in pipeline objects. Even
    though it is a bit strange/confusing that offscreen render targets by default
    get the same sample count as 'default swapchains', but it's better that
    an offscreen render target created with default parameters matches
    a pipeline object created with default parameters.

    NOTE:

    Images with usage SG_USAGE_IMMUTABLE must be fully initialized by
    providing a valid .data member which points to initialization data.

    ADVANCED TOPIC: Injecting native 3D-API textures:

    The following struct members allow to inject your own GL, Metal or D3D11
    textures into sokol_gfx:

    .gl_textures[SG_NUM_INFLIGHT_FRAMES]
    .mtl_textures[SG_NUM_INFLIGHT_FRAMES]
    .d3d11_texture
    .d3d11_shader_resource_view
    .wgpu_texture
    .wgpu_texture_view

    For GL, you can also specify the texture target or leave it empty to use
    the default texture target for the image type (GL_TEXTURE_2D for
    SG_IMAGETYPE_2D etc)

    For D3D11 and WebGPU, either only provide a texture, or both a texture and
    shader-resource-view / texture-view object. If you want to use access the
    injected texture in a shader you *must* provide a shader-resource-view.

    The same rules apply as for injecting native buffers (see sg_buffer_desc
    documentation for more details).
*/
typedef struct sg_image_desc {
    uint32_t _start_canary;
    sg_image_type type;
    bool render_target;
    int width;
    int height;
    int num_slices;
    int num_mipmaps;
    sg_usage usage;
    sg_pixel_format pixel_format;
    int sample_count;
    sg_image_data data;
    const char* label;
    // optionally inject backend-specific resources
    uint32_t gl_textures[SG_NUM_INFLIGHT_FRAMES];
    uint32_t gl_texture_target;
    uint32_t _end_canary;
} sg_image_desc;

/*
    sg_sampler_desc

    Creation parameters for sg_sampler objects, used in the sg_make_sampler() call

    .min_filter:        SG_FILTER_NEAREST
    .mag_filter:        SG_FILTER_NEAREST
    .mipmap_filter      SG_FILTER_NONE
    .wrap_u:            SG_WRAP_REPEAT
    .wrap_v:            SG_WRAP_REPEAT
    .wrap_w:            SG_WRAP_REPEAT (only SG_IMAGETYPE_3D)
    .min_lod            0.0f
    .max_lod            FLT_MAX
    .border_color       SG_BORDERCOLOR_OPAQUE_BLACK
    .compare            SG_COMPAREFUNC_NEVER
    .max_anisotropy     1 (must be 1..16)

*/
typedef struct sg_sampler_desc {
    uint32_t _start_canary;
    sg_filter min_filter;
    sg_filter mag_filter;
    sg_filter mipmap_filter;
    sg_wrap wrap_u;
    sg_wrap wrap_v;
    sg_wrap wrap_w;
    float min_lod;
    float max_lod;
    sg_border_color border_color;
    sg_compare_func compare;
    uint32_t max_anisotropy;
    const char* label;
    // optionally inject backend-specific resources
    uint32_t gl_sampler;
    uint32_t _end_canary;
} sg_sampler_desc;

/*
    sg_shader_desc

    The structure sg_shader_desc defines all creation parameters for shader
    programs, used as input to the sg_make_shader() function:

    - reflection information for vertex attributes (vertex shader inputs):
        - vertex attribute name (only optionally used by GLES3 and GL)
        - a semantic name and index (required for D3D11)
    - for each shader-stage (vertex and fragment):
        - the shader source or bytecode
        - an optional entry function name
        - an optional compile target (only for D3D11 when source is provided,
          defaults are "vs_4_0" and "ps_4_0")
        - reflection info for each uniform block used by the shader stage:
            - the size of the uniform block in bytes
            - a memory layout hint (native vs std140, only required for GL backends)
            - reflection info for each uniform block member (only required for GL backends):
                - member name
                - member type (SG_UNIFORMTYPE_xxx)
                - if the member is an array, the number of array items
        - reflection info for textures used in the shader stage:
            - the image type (SG_IMAGETYPE_xxx)
            - the image-sample type (SG_IMAGESAMPLETYPE_xxx, default is SG_IMAGESAMPLETYPE_FLOAT)
            - whether the shader expects a multisampled texture
        - reflection info for samplers used in the shader stage:
            - the sampler type (SG_SAMPLERTYPE_xxx)
        - reflection info for each image-sampler-pair used by the shader:
            - the texture slot of the involved texture
            - the sampler slot of the involved sampler
            - for GLSL only: the name of the combined image-sampler object
        - reflection info for each storage-buffer used by the shader:
            - whether the storage buffer is readonly (currently this
              must be true)

    For all GL backends, shader source-code must be provided. For D3D11 and Metal,
    either shader source-code or byte-code can be provided.

    For D3D11, if source code is provided, the d3dcompiler_47.dll will be loaded
    on demand. If this fails, shader creation will fail. When compiling HLSL
    source code, you can provide an optional target string via
    sg_shader_stage_desc.d3d11_target, the default target is "vs_4_0" for the
    vertex shader stage and "ps_4_0" for the pixel shader stage.
*/
typedef struct sg_shader_attr_desc {
    const char* name;           // GLSL vertex attribute name (optional)
    const char* sem_name;       // HLSL semantic name
    int sem_index;              // HLSL semantic index
} sg_shader_attr_desc;

typedef struct sg_shader_uniform_desc {
    const char* name;
    sg_uniform_type type;
    int array_count;
} sg_shader_uniform_desc;

typedef struct sg_shader_uniform_block_desc {
    size_t size;
    sg_uniform_layout layout;
    sg_shader_uniform_desc uniforms[SG_MAX_UB_MEMBERS];
} sg_shader_uniform_block_desc;

typedef struct sg_shader_storage_buffer_desc {
    bool used;
    bool readonly;
} sg_shader_storage_buffer_desc;

typedef struct sg_shader_image_desc {
    bool used;
    bool multisampled;
    sg_image_type image_type;
    sg_image_sample_type sample_type;
} sg_shader_image_desc;

typedef struct sg_shader_sampler_desc {
    bool used;
    sg_sampler_type sampler_type;
} sg_shader_sampler_desc;

typedef struct sg_shader_image_sampler_pair_desc {
    bool used;
    int image_slot;
    int sampler_slot;
    const char* glsl_name;
} sg_shader_image_sampler_pair_desc;

typedef struct sg_shader_stage_desc {
    const char* source;
    sg_range bytecode;
    const char* entry;
    sg_shader_uniform_block_desc uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    sg_shader_storage_buffer_desc storage_buffers[SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
    sg_shader_image_desc images[SG_MAX_SHADERSTAGE_IMAGES];
    sg_shader_sampler_desc samplers[SG_MAX_SHADERSTAGE_SAMPLERS];
    sg_shader_image_sampler_pair_desc image_sampler_pairs[SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS];
} sg_shader_stage_desc;

typedef struct sg_shader_desc {
    uint32_t _start_canary;
    sg_shader_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
    sg_shader_stage_desc vs;
    sg_shader_stage_desc fs;
    const char* label;
    uint32_t _end_canary;
} sg_shader_desc;

/*
    sg_pipeline_desc

    The sg_pipeline_desc struct defines all creation parameters for an
    sg_pipeline object, used as argument to the sg_make_pipeline() function:

    - the vertex layout for all input vertex buffers
    - a shader object
    - the 3D primitive type (points, lines, triangles, ...)
    - the index type (none, 16- or 32-bit)
    - all the fixed-function-pipeline state (depth-, stencil-, blend-state, etc...)

    If the vertex data has no gaps between vertex components, you can omit
    the .layout.buffers[].stride and layout.attrs[].offset items (leave them
    default-initialized to 0), sokol-gfx will then compute the offsets and
    strides from the vertex component formats (.layout.attrs[].format).
    Please note that ALL vertex attribute offsets must be 0 in order for the
    automatic offset computation to kick in.

    The default configuration is as follows:

    .shader:            0 (must be initialized with a valid sg_shader id!)
    .layout:
        .buffers[]:         vertex buffer layouts
            .stride:        0 (if no stride is given it will be computed)
            .step_func      SG_VERTEXSTEP_PER_VERTEX
            .step_rate      1
        .attrs[]:           vertex attribute declarations
            .buffer_index   0 the vertex buffer bind slot
            .offset         0 (offsets can be omitted if the vertex layout has no gaps)
            .format         SG_VERTEXFORMAT_INVALID (must be initialized!)
    .depth:
        .pixel_format:      sg_desc.context.depth_format
        .compare:           SG_COMPAREFUNC_ALWAYS
        .write_enabled:     false
        .bias:              0.0f
        .bias_slope_scale:  0.0f
        .bias_clamp:        0.0f
    .stencil:
        .enabled:           false
        .front/back:
            .compare:       SG_COMPAREFUNC_ALWAYS
            .fail_op:       SG_STENCILOP_KEEP
            .depth_fail_op: SG_STENCILOP_KEEP
            .pass_op:       SG_STENCILOP_KEEP
        .read_mask:         0
        .write_mask:        0
        .ref:               0
    .color_count            1
    .colors[0..color_count]
        .pixel_format       sg_desc.context.color_format
        .write_mask:        SG_COLORMASK_RGBA
        .blend:
            .enabled:           false
            .src_factor_rgb:    SG_BLENDFACTOR_ONE
            .dst_factor_rgb:    SG_BLENDFACTOR_ZERO
            .op_rgb:            SG_BLENDOP_ADD
            .src_factor_alpha:  SG_BLENDFACTOR_ONE
            .dst_factor_alpha:  SG_BLENDFACTOR_ZERO
            .op_alpha:          SG_BLENDOP_ADD
    .primitive_type:            SG_PRIMITIVETYPE_TRIANGLES
    .index_type:                SG_INDEXTYPE_NONE
    .cull_mode:                 SG_CULLMODE_NONE
    .face_winding:              SG_FACEWINDING_CW
    .sample_count:              sg_desc.context.sample_count
    .blend_color:               (sg_color) { 0.0f, 0.0f, 0.0f, 0.0f }
    .alpha_to_coverage_enabled: false
    .label  0       (optional string label for trace hooks)
*/
typedef struct sg_vertex_buffer_layout_state {
    int stride;
    sg_vertex_step step_func;
    int step_rate;
} sg_vertex_buffer_layout_state;

typedef struct sg_vertex_attr_state {
    int buffer_index;
    int offset;
    sg_vertex_format format;
} sg_vertex_attr_state;

typedef struct sg_vertex_layout_state {
    sg_vertex_buffer_layout_state buffers[SG_MAX_VERTEX_BUFFERS];
    sg_vertex_attr_state attrs[SG_MAX_VERTEX_ATTRIBUTES];
} sg_vertex_layout_state;

typedef struct sg_stencil_face_state {
    sg_compare_func compare;
    sg_stencil_op fail_op;
    sg_stencil_op depth_fail_op;
    sg_stencil_op pass_op;
} sg_stencil_face_state;

typedef struct sg_stencil_state {
    bool enabled;
    sg_stencil_face_state front;
    sg_stencil_face_state back;
    uint8_t read_mask;
    uint8_t write_mask;
    uint8_t ref;
} sg_stencil_state;

typedef struct sg_depth_state {
    sg_pixel_format pixel_format;
    sg_compare_func compare;
    bool write_enabled;
    float bias;
    float bias_slope_scale;
    float bias_clamp;
} sg_depth_state;

typedef struct sg_blend_state {
    bool enabled;
    sg_blend_factor src_factor_rgb;
    sg_blend_factor dst_factor_rgb;
    sg_blend_op op_rgb;
    sg_blend_factor src_factor_alpha;
    sg_blend_factor dst_factor_alpha;
    sg_blend_op op_alpha;
} sg_blend_state;

typedef struct sg_color_target_state {
    sg_pixel_format pixel_format;
    sg_color_mask write_mask;
    sg_blend_state blend;
} sg_color_target_state;

typedef struct sg_pipeline_desc {
    uint32_t _start_canary;
    sg_shader shader;
    sg_vertex_layout_state layout;
    sg_depth_state depth;
    sg_stencil_state stencil;
    int color_count;
    sg_color_target_state colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_primitive_type primitive_type;
    sg_index_type index_type;
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    int sample_count;
    sg_color blend_color;
    bool alpha_to_coverage_enabled;
    const char* label;
    uint32_t _end_canary;
} sg_pipeline_desc;

/*
    sg_attachments_desc

    Creation parameters for an sg_attachments object, used as argument to the
    sg_make_attachments() function.

    An attachments object bundles 0..4 color attachments, 0..4 msaa-resolve
    attachments, and none or one depth-stencil attachmente for use
    in a render pass. At least one color attachment or one depth-stencil
    attachment must be provided (no color attachment and a depth-stencil
    attachment is useful for a depth-only render pass).

    Each attachment definition consists of an image object, and two additional indices
    describing which subimage the pass will render into: one mipmap index, and if the image
    is a cubemap, array-texture or 3D-texture, the face-index, array-layer or
    depth-slice.

    All attachments must have the same width and height.

    All color attachments and the depth-stencil attachment must have the
    same sample count.

    If a resolve attachment is set, an MSAA-resolve operation from the
    associated color attachment image into the resolve attachment image will take
    place in the sg_end_pass() function. In this case, the color attachment
    must have a (sample_count>1), and the resolve attachment a
    (sample_count==1). The resolve attachment also must have the same pixel
    format as the color attachment.

    NOTE that MSAA depth-stencil attachments cannot be msaa-resolved!
*/
typedef struct sg_attachment_desc {
    sg_image image;
    int mip_level;
    int slice;      // cube texture: face; array texture: layer; 3D texture: slice
} sg_attachment_desc;

typedef struct sg_attachments_desc {
    uint32_t _start_canary;
    sg_attachment_desc colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_attachment_desc resolves[SG_MAX_COLOR_ATTACHMENTS];
    sg_attachment_desc depth_stencil;
    const char* label;
    uint32_t _end_canary;
} sg_attachments_desc;

/*
    sg_trace_hooks

    Installable callback functions to keep track of the sokol-gfx calls,
    this is useful for debugging, or keeping track of resource creation
    and destruction.

    Trace hooks are installed with sg_install_trace_hooks(), this returns
    another sg_trace_hooks struct with the previous set of
    trace hook function pointers. These should be invoked by the
    new trace hooks to form a proper call chain.
*/
typedef struct sg_trace_hooks {
    void* user_data;
    void (*reset_state_cache)(void* user_data);
    void (*make_buffer)(const sg_buffer_desc* desc, sg_buffer result, void* user_data);
    void (*make_image)(const sg_image_desc* desc, sg_image result, void* user_data);
    void (*make_sampler)(const sg_sampler_desc* desc, sg_sampler result, void* user_data);
    void (*make_shader)(const sg_shader_desc* desc, sg_shader result, void* user_data);
    void (*make_pipeline)(const sg_pipeline_desc* desc, sg_pipeline result, void* user_data);
    void (*make_attachments)(const sg_attachments_desc* desc, sg_attachments result, void* user_data);
    void (*destroy_buffer)(sg_buffer buf, void* user_data);
    void (*destroy_image)(sg_image img, void* user_data);
    void (*destroy_sampler)(sg_sampler smp, void* user_data);
    void (*destroy_shader)(sg_shader shd, void* user_data);
    void (*destroy_pipeline)(sg_pipeline pip, void* user_data);
    void (*destroy_attachments)(sg_attachments atts, void* user_data);
    void (*update_buffer)(sg_buffer buf, const sg_range* data, void* user_data);
    void (*update_image)(sg_image img, const sg_image_data* data, void* user_data);
    void (*append_buffer)(sg_buffer buf, const sg_range* data, int result, void* user_data);
    void (*begin_pass)(const sg_pass* pass, void* user_data);
    void (*apply_viewport)(int x, int y, int width, int height, bool origin_top_left, void* user_data);
    void (*apply_scissor_rect)(int x, int y, int width, int height, bool origin_top_left, void* user_data);
    void (*apply_pipeline)(sg_pipeline pip, void* user_data);
    void (*apply_bindings)(const sg_bindings* bindings, void* user_data);
    void (*apply_uniforms)(sg_shader_stage stage, int ub_index, const sg_range* data, void* user_data);
    void (*draw)(int base_element, int num_elements, int num_instances, void* user_data);
    void (*end_pass)(void* user_data);
    void (*commit)(void* user_data);
    void (*alloc_buffer)(sg_buffer result, void* user_data);
    void (*alloc_image)(sg_image result, void* user_data);
    void (*alloc_sampler)(sg_sampler result, void* user_data);
    void (*alloc_shader)(sg_shader result, void* user_data);
    void (*alloc_pipeline)(sg_pipeline result, void* user_data);
    void (*alloc_attachments)(sg_attachments result, void* user_data);
    void (*dealloc_buffer)(sg_buffer buf_id, void* user_data);
    void (*dealloc_image)(sg_image img_id, void* user_data);
    void (*dealloc_sampler)(sg_sampler smp_id, void* user_data);
    void (*dealloc_shader)(sg_shader shd_id, void* user_data);
    void (*dealloc_pipeline)(sg_pipeline pip_id, void* user_data);
    void (*dealloc_attachments)(sg_attachments atts_id, void* user_data);
    void (*init_buffer)(sg_buffer buf_id, const sg_buffer_desc* desc, void* user_data);
    void (*init_image)(sg_image img_id, const sg_image_desc* desc, void* user_data);
    void (*init_sampler)(sg_sampler smp_id, const sg_sampler_desc* desc, void* user_data);
    void (*init_shader)(sg_shader shd_id, const sg_shader_desc* desc, void* user_data);
    void (*init_pipeline)(sg_pipeline pip_id, const sg_pipeline_desc* desc, void* user_data);
    void (*init_attachments)(sg_attachments atts_id, const sg_attachments_desc* desc, void* user_data);
    void (*uninit_buffer)(sg_buffer buf_id, void* user_data);
    void (*uninit_image)(sg_image img_id, void* user_data);
    void (*uninit_sampler)(sg_sampler smp_id, void* user_data);
    void (*uninit_shader)(sg_shader shd_id, void* user_data);
    void (*uninit_pipeline)(sg_pipeline pip_id, void* user_data);
    void (*uninit_attachments)(sg_attachments atts_id, void* user_data);
    void (*fail_buffer)(sg_buffer buf_id, void* user_data);
    void (*fail_image)(sg_image img_id, void* user_data);
    void (*fail_sampler)(sg_sampler smp_id, void* user_data);
    void (*fail_shader)(sg_shader shd_id, void* user_data);
    void (*fail_pipeline)(sg_pipeline pip_id, void* user_data);
    void (*fail_attachments)(sg_attachments atts_id, void* user_data);
    void (*push_debug_group)(const char* name, void* user_data);
    void (*pop_debug_group)(void* user_data);
} sg_trace_hooks;

/*
    sg_buffer_info
    sg_image_info
    sg_sampler_info
    sg_shader_info
    sg_pipeline_info
    sg_attachments_info

    These structs contain various internal resource attributes which
    might be useful for debug-inspection. Please don't rely on the
    actual content of those structs too much, as they are quite closely
    tied to sokol_gfx.h internals and may change more frequently than
    the other public API elements.

    The *_info structs are used as the return values of the following functions:

    sg_query_buffer_info()
    sg_query_image_info()
    sg_query_sampler_info()
    sg_query_shader_info()
    sg_query_pipeline_info()
    sg_query_pass_info()
*/
typedef struct sg_slot_info {
    sg_resource_state state;    // the current state of this resource slot
    uint32_t res_id;            // type-neutral resource if (e.g. sg_buffer.id)
} sg_slot_info;

typedef struct sg_buffer_info {
    sg_slot_info slot;              // resource pool slot info
    uint32_t update_frame_index;    // frame index of last sg_update_buffer()
    uint32_t append_frame_index;    // frame index of last sg_append_buffer()
    int append_pos;                 // current position in buffer for sg_append_buffer()
    bool append_overflow;           // is buffer in overflow state (due to sg_append_buffer)
    int num_slots;                  // number of renaming-slots for dynamically updated buffers
    int active_slot;                // currently active write-slot for dynamically updated buffers
} sg_buffer_info;

typedef struct sg_image_info {
    sg_slot_info slot;              // resource pool slot info
    uint32_t upd_frame_index;       // frame index of last sg_update_image()
    int num_slots;                  // number of renaming-slots for dynamically updated images
    int active_slot;                // currently active write-slot for dynamically updated images
} sg_image_info;

typedef struct sg_sampler_info {
    sg_slot_info slot;              // resource pool slot info
} sg_sampler_info;

typedef struct sg_shader_info {
    sg_slot_info slot;              // resource pool slot info
} sg_shader_info;

typedef struct sg_pipeline_info {
    sg_slot_info slot;              // resource pool slot info
} sg_pipeline_info;

typedef struct sg_attachments_info {
    sg_slot_info slot;              // resource pool slot info
} sg_attachments_info;

/*
    sg_frame_stats

    Allows to track generic and backend-specific stats about a
    render frame. Obtained by calling sg_query_frame_stats(). The returned
    struct contains information about the *previous* frame.
*/
typedef struct sg_frame_stats_gl {
    uint32_t num_bind_buffer;
    uint32_t num_active_texture;
    uint32_t num_bind_texture;
    uint32_t num_bind_sampler;
    uint32_t num_use_program;
    uint32_t num_render_state;
    uint32_t num_vertex_attrib_pointer;
    uint32_t num_vertex_attrib_divisor;
    uint32_t num_enable_vertex_attrib_array;
    uint32_t num_disable_vertex_attrib_array;
    uint32_t num_uniform;
} sg_frame_stats_gl;

typedef struct sg_frame_stats {
    uint32_t frame_index;   // current frame counter, starts at 0

    uint32_t num_passes;
    uint32_t num_apply_viewport;
    uint32_t num_apply_scissor_rect;
    uint32_t num_apply_pipeline;
    uint32_t num_apply_bindings;
    uint32_t num_apply_uniforms;
    uint32_t num_draw;
    uint32_t num_update_buffer;
    uint32_t num_append_buffer;
    uint32_t num_update_image;

    uint32_t size_apply_uniforms;
    uint32_t size_update_buffer;
    uint32_t size_append_buffer;
    uint32_t size_update_image;

    sg_frame_stats_gl gl;
} sg_frame_stats;

/*
    sg_log_item

    An enum with a unique item for each log message, warning, error
    and validation layer message.
*/
#define _SG_LOG_ITEMS \
    _SG_LOGITEM_XMACRO(OK, "Ok") \
    _SG_LOGITEM_XMACRO(MALLOC_FAILED, "memory allocation failed") \
    _SG_LOGITEM_XMACRO(GL_TEXTURE_FORMAT_NOT_SUPPORTED, "pixel format not supported for texture (gl)") \
    _SG_LOGITEM_XMACRO(GL_3D_TEXTURES_NOT_SUPPORTED, "3d textures not supported (gl)") \
    _SG_LOGITEM_XMACRO(GL_ARRAY_TEXTURES_NOT_SUPPORTED, "array textures not supported (gl)") \
    _SG_LOGITEM_XMACRO(GL_SHADER_COMPILATION_FAILED, "shader compilation failed (gl)") \
    _SG_LOGITEM_XMACRO(GL_SHADER_LINKING_FAILED, "shader linking failed (gl)") \
    _SG_LOGITEM_XMACRO(GL_VERTEX_ATTRIBUTE_NOT_FOUND_IN_SHADER, "vertex attribute not found in shader (gl)") \
    _SG_LOGITEM_XMACRO(GL_TEXTURE_NAME_NOT_FOUND_IN_SHADER, "texture name not found in shader (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_UNDEFINED, "framebuffer completeness check failed with GL_FRAMEBUFFER_UNDEFINED (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_INCOMPLETE_ATTACHMENT, "framebuffer completeness check failed with GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MISSING_ATTACHMENT, "framebuffer completeness check failed with GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_UNSUPPORTED, "framebuffer completeness check failed with GL_FRAMEBUFFER_UNSUPPORTED (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MULTISAMPLE, "framebuffer completeness check failed with GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE (gl)") \
    _SG_LOGITEM_XMACRO(GL_FRAMEBUFFER_STATUS_UNKNOWN, "framebuffer completeness check failed (unknown reason) (gl)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_BUFFER_FAILED, "CreateBuffer() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_BUFFER_SRV_FAILED, "CreateShaderResourceView() failed for storage buffer (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_DEPTH_TEXTURE_UNSUPPORTED_PIXEL_FORMAT, "pixel format not supported for depth-stencil texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_DEPTH_TEXTURE_FAILED, "CreateTexture2D() failed for depth-stencil texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_2D_TEXTURE_UNSUPPORTED_PIXEL_FORMAT, "pixel format not supported for 2d-, cube- or array-texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_2D_TEXTURE_FAILED, "CreateTexture2D() failed for 2d-, cube- or array-texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_2D_SRV_FAILED, "CreateShaderResourceView() failed for 2d-, cube- or array-texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_3D_TEXTURE_UNSUPPORTED_PIXEL_FORMAT, "pixel format not supported for 3D texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_3D_TEXTURE_FAILED, "CreateTexture3D() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_3D_SRV_FAILED, "CreateShaderResourceView() failed for 3d texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_MSAA_TEXTURE_FAILED, "CreateTexture2D() failed for MSAA render target texture (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_SAMPLER_STATE_FAILED, "CreateSamplerState() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_LOAD_D3DCOMPILER_47_DLL_FAILED, "loading d3dcompiler_47.dll failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_SHADER_COMPILATION_FAILED, "shader compilation failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_SHADER_COMPILATION_OUTPUT, "") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_CONSTANT_BUFFER_FAILED, "CreateBuffer() failed for uniform constant buffer (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_INPUT_LAYOUT_FAILED, "CreateInputLayout() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_RASTERIZER_STATE_FAILED, "CreateRasterizerState() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_DEPTH_STENCIL_STATE_FAILED, "CreateDepthStencilState() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_BLEND_STATE_FAILED, "CreateBlendState() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_RTV_FAILED, "CreateRenderTargetView() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_CREATE_DSV_FAILED, "CreateDepthStencilView() failed (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_MAP_FOR_UPDATE_BUFFER_FAILED, "Map() failed when updating buffer (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_MAP_FOR_APPEND_BUFFER_FAILED, "Map() failed when appending to buffer (d3d11)") \
    _SG_LOGITEM_XMACRO(D3D11_MAP_FOR_UPDATE_IMAGE_FAILED, "Map() failed when updating image (d3d11)") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_BUFFER_FAILED, "failed to create buffer object (metal)") \
    _SG_LOGITEM_XMACRO(METAL_TEXTURE_FORMAT_NOT_SUPPORTED, "pixel format not supported for texture (metal)") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_TEXTURE_FAILED, "failed to create texture object (metal)") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_SAMPLER_FAILED, "failed to create sampler object (metal)") \
    _SG_LOGITEM_XMACRO(METAL_SHADER_COMPILATION_FAILED, "shader compilation failed (metal)") \
    _SG_LOGITEM_XMACRO(METAL_SHADER_CREATION_FAILED, "shader creation failed (metal)") \
    _SG_LOGITEM_XMACRO(METAL_SHADER_COMPILATION_OUTPUT, "") \
    _SG_LOGITEM_XMACRO(METAL_VERTEX_SHADER_ENTRY_NOT_FOUND, "vertex shader entry function not found (metal)") \
    _SG_LOGITEM_XMACRO(METAL_FRAGMENT_SHADER_ENTRY_NOT_FOUND, "fragment shader entry not found (metal)") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_RPS_FAILED, "failed to create render pipeline state (metal)") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_RPS_OUTPUT, "") \
    _SG_LOGITEM_XMACRO(METAL_CREATE_DSS_FAILED, "failed to create depth stencil state (metal)") \
    _SG_LOGITEM_XMACRO(WGPU_BINDGROUPS_POOL_EXHAUSTED, "bindgroups pool exhausted (increase sg_desc.bindgroups_cache_size) (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_BINDGROUPSCACHE_SIZE_GREATER_ONE, "sg_desc.wgpu_bindgroups_cache_size must be > 1 (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_BINDGROUPSCACHE_SIZE_POW2, "sg_desc.wgpu_bindgroups_cache_size must be a power of 2 (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_CREATEBINDGROUP_FAILED, "wgpuDeviceCreateBindGroup failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_BUFFER_FAILED, "wgpuDeviceCreateBuffer() failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_TEXTURE_FAILED, "wgpuDeviceCreateTexture() failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_TEXTURE_VIEW_FAILED, "wgpuTextureCreateView() failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_SAMPLER_FAILED, "wgpuDeviceCreateSampler() failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_SHADER_MODULE_FAILED, "wgpuDeviceCreateShaderModule() failed") \
    _SG_LOGITEM_XMACRO(WGPU_SHADER_TOO_MANY_IMAGES, "shader uses too many sampled images on shader stage (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_SHADER_TOO_MANY_SAMPLERS, "shader uses too many samplers on shader stage (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_SHADER_TOO_MANY_STORAGEBUFFERS, "shader uses too many storage buffer bindings on shader stage (wgpu)") \
    _SG_LOGITEM_XMACRO(WGPU_SHADER_CREATE_BINDGROUP_LAYOUT_FAILED, "wgpuDeviceCreateBindGroupLayout() for shader stage failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_PIPELINE_LAYOUT_FAILED, "wgpuDeviceCreatePipelineLayout() failed") \
    _SG_LOGITEM_XMACRO(WGPU_CREATE_RENDER_PIPELINE_FAILED, "wgpuDeviceCreateRenderPipeline() failed") \
    _SG_LOGITEM_XMACRO(WGPU_ATTACHMENTS_CREATE_TEXTURE_VIEW_FAILED, "wgpuTextureCreateView() failed in create attachments") \
    _SG_LOGITEM_XMACRO(IDENTICAL_COMMIT_LISTENER, "attempting to add identical commit listener") \
    _SG_LOGITEM_XMACRO(COMMIT_LISTENER_ARRAY_FULL, "commit listener array full") \
    _SG_LOGITEM_XMACRO(TRACE_HOOKS_NOT_ENABLED, "sg_install_trace_hooks() called, but SG_TRACE_HOOKS is not defined") \
    _SG_LOGITEM_XMACRO(DEALLOC_BUFFER_INVALID_STATE, "sg_dealloc_buffer(): buffer must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(DEALLOC_IMAGE_INVALID_STATE, "sg_dealloc_image(): image must be in alloc state") \
    _SG_LOGITEM_XMACRO(DEALLOC_SAMPLER_INVALID_STATE, "sg_dealloc_sampler(): sampler must be in alloc state") \
    _SG_LOGITEM_XMACRO(DEALLOC_SHADER_INVALID_STATE, "sg_dealloc_shader(): shader must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(DEALLOC_PIPELINE_INVALID_STATE, "sg_dealloc_pipeline(): pipeline must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(DEALLOC_ATTACHMENTS_INVALID_STATE, "sg_dealloc_attachments(): attachments must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_BUFFER_INVALID_STATE, "sg_init_buffer(): buffer must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_IMAGE_INVALID_STATE, "sg_init_image(): image must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_SAMPLER_INVALID_STATE, "sg_init_sampler(): sampler must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_SHADER_INVALID_STATE, "sg_init_shader(): shader must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_PIPELINE_INVALID_STATE, "sg_init_pipeline(): pipeline must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(INIT_ATTACHMENTS_INVALID_STATE, "sg_init_attachments(): pass must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(UNINIT_BUFFER_INVALID_STATE, "sg_uninit_buffer(): buffer must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(UNINIT_IMAGE_INVALID_STATE, "sg_uninit_image(): image must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(UNINIT_SAMPLER_INVALID_STATE, "sg_uninit_sampler(): sampler must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(UNINIT_SHADER_INVALID_STATE, "sg_uninit_shader(): shader must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(UNINIT_PIPELINE_INVALID_STATE, "sg_uninit_pipeline(): pipeline must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(UNINIT_ATTACHMENTS_INVALID_STATE, "sg_uninit_attachments(): attachments must be in VALID or FAILED state") \
    _SG_LOGITEM_XMACRO(FAIL_BUFFER_INVALID_STATE, "sg_fail_buffer(): buffer must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(FAIL_IMAGE_INVALID_STATE, "sg_fail_image(): image must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(FAIL_SAMPLER_INVALID_STATE, "sg_fail_sampler(): sampler must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(FAIL_SHADER_INVALID_STATE, "sg_fail_shader(): shader must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(FAIL_PIPELINE_INVALID_STATE, "sg_fail_pipeline(): pipeline must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(FAIL_ATTACHMENTS_INVALID_STATE, "sg_fail_attachments(): attachments must be in ALLOC state") \
    _SG_LOGITEM_XMACRO(BUFFER_POOL_EXHAUSTED, "buffer pool exhausted") \
    _SG_LOGITEM_XMACRO(IMAGE_POOL_EXHAUSTED, "image pool exhausted") \
    _SG_LOGITEM_XMACRO(SAMPLER_POOL_EXHAUSTED, "sampler pool exhausted") \
    _SG_LOGITEM_XMACRO(SHADER_POOL_EXHAUSTED, "shader pool exhausted") \
    _SG_LOGITEM_XMACRO(PIPELINE_POOL_EXHAUSTED, "pipeline pool exhausted") \
    _SG_LOGITEM_XMACRO(PASS_POOL_EXHAUSTED, "pass pool exhausted") \
    _SG_LOGITEM_XMACRO(BEGINPASS_ATTACHMENT_INVALID, "sg_begin_pass: an attachment was provided that no longer exists") \
    _SG_LOGITEM_XMACRO(DRAW_WITHOUT_BINDINGS, "attempting to draw without resource bindings") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_CANARY, "sg_buffer_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_SIZE, "sg_buffer_desc.size and .data.size cannot both be 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_DATA, "immutable buffers must be initialized with data (sg_buffer_desc.data.ptr and sg_buffer_desc.data.size)") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_DATA_SIZE, "immutable buffer data size differs from buffer size") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_NO_DATA, "dynamic/stream usage buffers cannot be initialized with data") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_STORAGEBUFFER_SUPPORTED, "storage buffers not supported by the backend 3D API (requires OpenGL >= 4.3)") \
    _SG_LOGITEM_XMACRO(VALIDATE_BUFFERDESC_STORAGEBUFFER_SIZE_MULTIPLE_4, "size of storage buffers must be a multiple of 4") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDATA_NODATA, "sg_image_data: no data (.ptr and/or .size is zero)") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDATA_DATA_SIZE, "sg_image_data: data size doesn't match expected surface size") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_CANARY, "sg_image_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_WIDTH, "sg_image_desc.width must be > 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_HEIGHT, "sg_image_desc.height must be > 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_RT_PIXELFORMAT, "invalid pixel format for render-target image") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_NONRT_PIXELFORMAT, "invalid pixel format for non-render-target image") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_MSAA_BUT_NO_RT, "non-render-target images cannot be multisampled") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_NO_MSAA_RT_SUPPORT, "MSAA not supported for this pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_MSAA_NUM_MIPMAPS, "MSAA images must have num_mipmaps == 1") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_MSAA_3D_IMAGE, "3D images cannot have a sample_count > 1") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_DEPTH_3D_IMAGE, "3D images cannot have a depth/stencil image format") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_RT_IMMUTABLE, "render target images must be SG_USAGE_IMMUTABLE") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_RT_NO_DATA, "render target images cannot be initialized with data") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_INJECTED_NO_DATA, "images with injected textures cannot be initialized with data") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_DYNAMIC_NO_DATA, "dynamic/stream images cannot be initialized with data") \
    _SG_LOGITEM_XMACRO(VALIDATE_IMAGEDESC_COMPRESSED_IMMUTABLE, "compressed images must be immutable") \
    _SG_LOGITEM_XMACRO(VALIDATE_SAMPLERDESC_CANARY, "sg_sampler_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_SAMPLERDESC_MINFILTER_NONE, "sg_sampler_desc.min_filter cannot be SG_FILTER_NONE") \
    _SG_LOGITEM_XMACRO(VALIDATE_SAMPLERDESC_MAGFILTER_NONE, "sg_sampler_desc.mag_filter cannot be SG_FILTER_NONE") \
    _SG_LOGITEM_XMACRO(VALIDATE_SAMPLERDESC_ANISTROPIC_REQUIRES_LINEAR_FILTERING, "sg_sampler_desc.max_anisotropy > 1 requires min/mag/mipmap_filter to be SG_FILTER_LINEAR") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_CANARY, "sg_shader_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_SOURCE, "shader source code required") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_BYTECODE, "shader byte code required") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_SOURCE_OR_BYTECODE, "shader source or byte code required") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_BYTECODE_SIZE, "shader byte code length (in bytes) required") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_UBS, "shader uniform blocks must occupy continuous slots") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_UB_MEMBERS, "uniform block members must occupy continuous slots") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_UB_MEMBERS, "GL backend requires uniform block member declarations") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_UB_MEMBER_NAME, "uniform block member name missing") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_UB_SIZE_MISMATCH, "size of uniform block members doesn't match uniform block size") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_UB_ARRAY_COUNT, "uniform array count must be >= 1") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_UB_STD140_ARRAY_TYPE, "uniform arrays only allowed for FLOAT4, INT4, MAT4 in std140 layout") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_STORAGEBUFFERS, "shader stage storage buffers must occupy continuous slots (sg_shader_desc.vs|fs.storage_buffers[])") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_STORAGEBUFFER_READONLY, "shader stage storage buffers must be readonly (sg_shader_desc.vs|fs.storage_buffers[].readonly)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_IMAGES, "shader stage images must occupy continuous slots (sg_shader_desc.vs|fs.images[])") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_SAMPLERS, "shader stage samplers must occupy continuous slots (sg_shader_desc.vs|fs.samplers[])") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_IMAGE_SLOT_OUT_OF_RANGE, "shader stage: image-sampler-pair image slot index is out of range (sg_shader_desc.vs|fs.image_sampler_pairs[].image_slot)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_SAMPLER_SLOT_OUT_OF_RANGE, "shader stage: image-sampler-pair image slot index is out of range (sg_shader_desc.vs|fs.image_sampler_pairs[].sampler_slot)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_NAME_REQUIRED_FOR_GL, "shader stage: image-sampler-pairs must be named in GL (sg_shader_desc.vs|fs.image_sampler_pairs[].name)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_NAME_BUT_NOT_USED, "shader stage: image-sampler-pair has name but .used field not true") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_IMAGE_BUT_NOT_USED, "shader stage: image-sampler-pair has .image_slot != 0 but .used field not true") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_SAMPLER_BUT_NOT_USED, "shader stage: image-sampler-pair .sampler_slot != 0 but .used field not true") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NONFILTERING_SAMPLER_REQUIRED, "shader stage: image sample type UNFILTERABLE_FLOAT, UINT, SINT can only be used with NONFILTERING sampler") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_COMPARISON_SAMPLER_REQUIRED, "shader stage: image sample type DEPTH can only be used with COMPARISON sampler") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_IMAGE_NOT_REFERENCED_BY_IMAGE_SAMPLER_PAIRS, "shader stage: one or more images are note referenced by  (sg_shader_desc.vs|fs.image_sampler_pairs[].image_slot)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_SAMPLER_NOT_REFERENCED_BY_IMAGE_SAMPLER_PAIRS, "shader stage: one or more samplers are not referenced by image-sampler-pairs (sg_shader_desc.vs|fs.image_sampler_pairs[].sampler_slot)") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_NO_CONT_IMAGE_SAMPLER_PAIRS, "shader stage image-sampler-pairs must occupy continuous slots (sg_shader_desc.vs|fs.image_samplers[])") \
    _SG_LOGITEM_XMACRO(VALIDATE_SHADERDESC_ATTR_STRING_TOO_LONG, "vertex attribute name/semantic string too long (max len 16)") \
    _SG_LOGITEM_XMACRO(VALIDATE_PIPELINEDESC_CANARY, "sg_pipeline_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_PIPELINEDESC_SHADER, "sg_pipeline_desc.shader missing or invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_PIPELINEDESC_NO_CONT_ATTRS, "sg_pipeline_desc.layout.attrs is not continuous") \
    _SG_LOGITEM_XMACRO(VALIDATE_PIPELINEDESC_LAYOUT_STRIDE4, "sg_pipeline_desc.layout.buffers[].stride must be multiple of 4") \
    _SG_LOGITEM_XMACRO(VALIDATE_PIPELINEDESC_ATTR_SEMANTICS, "D3D11 missing vertex attribute semantics in shader") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_CANARY, "sg_attachments_desc not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_NO_ATTACHMENTS, "sg_attachments_desc no color or depth-stencil attachments") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_NO_CONT_COLOR_ATTS, "color attachments must occupy continuous slots") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_IMAGE, "pass attachment image is not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_MIPLEVEL, "pass attachment mip level is bigger than image has mipmaps") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_FACE, "pass attachment image is cubemap, but face index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_LAYER, "pass attachment image is array texture, but layer index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_SLICE, "pass attachment image is 3d texture, but slice value is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_IMAGE_NO_RT, "pass attachment image must be have render_target=true") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_COLOR_INV_PIXELFORMAT, "pass color-attachment images must be renderable color pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_INV_PIXELFORMAT, "pass depth-attachment image must be depth or depth-stencil pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_IMAGE_SIZES, "all pass attachments must have the same size") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_IMAGE_SAMPLE_COUNTS, "all pass attachments must have the same sample count") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_COLOR_IMAGE_MSAA, "pass resolve attachments must have a color attachment image with sample count > 1") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE, "pass resolve attachment image not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_SAMPLE_COUNT, "pass resolve attachment image sample count must be 1") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_MIPLEVEL, "pass resolve attachment mip level is bigger than image has mipmaps") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_FACE, "pass resolve attachment is cubemap, but face index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_LAYER, "pass resolve attachment is array texture, but layer index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_SLICE, "pass resolve attachment is 3d texture, but slice value is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_NO_RT, "pass resolve attachment image must have render_target=true") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_SIZES, "pass resolve attachment size must match color attachment image size") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_FORMAT, "pass resolve attachment pixel format must match color attachment pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE, "pass depth attachment image is not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_MIPLEVEL, "pass depth attachment mip level is bigger than image has mipmaps") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_FACE, "pass depth attachment image is cubemap, but face index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_LAYER, "pass depth attachment image is array texture, but layer index is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_SLICE, "pass depth attachment image is 3d texture, but slice value is too big") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_NO_RT, "pass depth attachment image must be have render_target=true") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_SIZES, "pass depth attachment image size must match color attachment image size") \
    _SG_LOGITEM_XMACRO(VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_SAMPLE_COUNT, "pass depth attachment sample count must match color attachment sample count") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_CANARY, "sg_begin_pass: pass struct not initialized") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_ATTACHMENTS_EXISTS, "sg_begin_pass: attachments object no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_ATTACHMENTS_VALID, "sg_begin_pass: attachments object not in resource state VALID") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_COLOR_ATTACHMENT_IMAGE, "sg_begin_pass: one or more color attachment images are not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_RESOLVE_ATTACHMENT_IMAGE, "sg_begin_pass: one or more resolve attachment images are not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_DEPTHSTENCIL_ATTACHMENT_IMAGE, "sg_begin_pass: one or more depth-stencil attachment images are not valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_WIDTH, "sg_begin_pass: expected pass.swapchain.width > 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_WIDTH_NOTSET, "sg_begin_pass: expected pass.swapchain.width == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_HEIGHT, "sg_begin_pass: expected pass.swapchain.height > 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_HEIGHT_NOTSET, "sg_begin_pass: expected pass.swapchain.height == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_SAMPLECOUNT, "sg_begin_pass: expected pass.swapchain.sample_count > 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_SAMPLECOUNT_NOTSET, "sg_begin_pass: expected pass.swapchain.sample_count == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_COLORFORMAT, "sg_begin_pass: expected pass.swapchain.color_format to be valid") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_COLORFORMAT_NOTSET, "sg_begin_pass: expected pass.swapchain.color_format to be unset") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_DEPTHFORMAT_NOTSET, "sg_begin_pass: expected pass.swapchain.depth_format to be unset") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_CURRENTDRAWABLE, "sg_begin_pass: expected pass.swapchain.metal.current_drawable != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_CURRENTDRAWABLE_NOTSET, "sg_begin_pass: expected pass.swapchain.metal.current_drawable == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_DEPTHSTENCILTEXTURE, "sg_begin_pass: expected pass.swapchain.metal.depth_stencil_texture != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_DEPTHSTENCILTEXTURE_NOTSET, "sg_begin_pass: expected pass.swapchain.metal.depth_stencil_texture == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_MSAACOLORTEXTURE, "sg_begin_pass: expected pass.swapchain.metal.msaa_color_texture != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_METAL_EXPECT_MSAACOLORTEXTURE_NOTSET, "sg_begin_pass: expected pass.swapchain.metal.msaa_color_texture == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_RENDERVIEW, "sg_begin_pass: expected pass.swapchain.d3d11.render_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_RENDERVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.d3d11.render_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_RESOLVEVIEW, "sg_begin_pass: expected pass.swapchain.d3d11.resolve_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_RESOLVEVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.d3d11.resolve_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_DEPTHSTENCILVIEW, "sg_begin_pass: expected pass.swapchain.d3d11.depth_stencil_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_D3D11_EXPECT_DEPTHSTENCILVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.d3d11.depth_stencil_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_RENDERVIEW, "sg_begin_pass: expected pass.swapchain.wgpu.render_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_RENDERVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.wgpu.render_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_RESOLVEVIEW, "sg_begin_pass: expected pass.swapchain.wgpu.resolve_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_RESOLVEVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.wgpu.resolve_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_DEPTHSTENCILVIEW, "sg_begin_pass: expected pass.swapchain.wgpu.depth_stencil_view != 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_WGPU_EXPECT_DEPTHSTENCILVIEW_NOTSET, "sg_begin_pass: expected pass.swapchain.wgpu.depth_stencil_view == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_BEGINPASS_SWAPCHAIN_GL_EXPECT_FRAMEBUFFER_NOTSET, "sg_begin_pass: expected pass.swapchain.gl.framebuffer == 0") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_PIPELINE_VALID_ID, "sg_apply_pipeline: invalid pipeline id provided") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_PIPELINE_EXISTS, "sg_apply_pipeline: pipeline object no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_PIPELINE_VALID, "sg_apply_pipeline: pipeline object not in valid state") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_SHADER_EXISTS, "sg_apply_pipeline: shader object no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_SHADER_VALID, "sg_apply_pipeline: shader object not in valid state") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_CURPASS_ATTACHMENTS_EXISTS, "sg_apply_pipeline: current pass attachments no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_CURPASS_ATTACHMENTS_VALID, "sg_apply_pipeline: current pass attachments not in valid state") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_ATT_COUNT, "sg_apply_pipeline: number of pipeline color attachments doesn't match number of pass color attachments") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_COLOR_FORMAT, "sg_apply_pipeline: pipeline color attachment pixel format doesn't match pass color attachment pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_DEPTH_FORMAT, "sg_apply_pipeline: pipeline depth pixel_format doesn't match pass depth attachment pixel format") \
    _SG_LOGITEM_XMACRO(VALIDATE_APIP_SAMPLE_COUNT, "sg_apply_pipeline: pipeline MSAA sample count doesn't match render pass attachment sample count") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_PIPELINE, "sg_apply_bindings: must be called after sg_apply_pipeline") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_PIPELINE_EXISTS, "sg_apply_bindings: currently applied pipeline object no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_PIPELINE_VALID, "sg_apply_bindings: currently applied pipeline object not in valid state") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VBS, "sg_apply_bindings: number of vertex buffers doesn't match number of pipeline vertex layouts") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VB_EXISTS, "sg_apply_bindings: vertex buffer no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VB_TYPE, "sg_apply_bindings: buffer in vertex buffer slot is not a SG_BUFFERTYPE_VERTEXBUFFER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VB_OVERFLOW, "sg_apply_bindings: buffer in vertex buffer slot is overflown") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_NO_IB, "sg_apply_bindings: pipeline object defines indexed rendering, but no index buffer provided") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_IB, "sg_apply_bindings: pipeline object defines non-indexed rendering, but index buffer provided") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_IB_EXISTS, "sg_apply_bindings: index buffer no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_IB_TYPE, "sg_apply_bindings: buffer in index buffer slot is not a SG_BUFFERTYPE_INDEXBUFFER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_IB_OVERFLOW, "sg_apply_bindings: buffer in index buffer slot is overflown") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_IMAGE_BINDING, "sg_apply_bindings: image binding on vertex stage is missing or the image handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_IMG_EXISTS, "sg_apply_bindings: image bound to vertex stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_IMAGE_TYPE_MISMATCH, "sg_apply_bindings: type of image bound to vertex stage doesn't match shader desc") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_IMAGE_MSAA, "sg_apply_bindings: cannot bind image with sample_count>1 to vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_FILTERABLE_IMAGE, "sg_apply_bindings: filterable image expected on vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_DEPTH_IMAGE, "sg_apply_bindings: depth image expected on vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_UNEXPECTED_IMAGE_BINDING, "sg_apply_bindings: unexpected image binding on vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_SAMPLER_BINDING, "sg_apply_bindings: sampler binding on vertex stage is missing or the sampler handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_UNEXPECTED_SAMPLER_COMPARE_NEVER, "sg_apply_bindings: shader expects SG_SAMPLERTYPE_COMPARISON on vertex stage but sampler has SG_COMPAREFUNC_NEVER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_SAMPLER_COMPARE_NEVER, "sg_apply_bindings: shader expects SG_SAMPLERTYPE_FILTERING or SG_SAMPLERTYPE_NONFILTERING on vertex stage but sampler doesn't have SG_COMPAREFUNC_NEVER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_NONFILTERING_SAMPLER, "sg_apply_bindings: shader expected SG_SAMPLERTYPE_NONFILTERING on vertex stage, but sampler has SG_FILTER_LINEAR filters") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_UNEXPECTED_SAMPLER_BINDING, "sg_apply_bindings: unexpected sampler binding on vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_SMP_EXISTS, "sg_apply_bindings: sampler bound to vertex stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_EXPECTED_STORAGEBUFFER_BINDING, "sg_apply_bindings: storage buffer binding on vertex stage is missing or the buffer handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_STORAGEBUFFER_EXISTS, "sg_apply_bindings: storage buffer bound to vertex stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_STORAGEBUFFER_BINDING_BUFFERTYPE, "sg_apply_bindings: buffer bound to vertex stage storage buffer slot is not of type storage buffer") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_VS_UNEXPECTED_STORAGEBUFFER_BINDING, "sg_apply_bindings: unexpected storage buffer binding on vertex stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_IMAGE_BINDING, "sg_apply_bindings: image binding on fragment stage is missing or the image handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_IMG_EXISTS, "sg_apply_bindings: image bound to fragment stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_IMAGE_TYPE_MISMATCH, "sg_apply_bindings: type of image bound to fragment stage doesn't match shader desc") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_IMAGE_MSAA, "sg_apply_bindings: cannot bind image with sample_count>1 to fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_FILTERABLE_IMAGE, "sg_apply_bindings: filterable image expected on fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_DEPTH_IMAGE, "sg_apply_bindings: depth image expected on fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_UNEXPECTED_IMAGE_BINDING, "sg_apply_bindings: unexpected image binding on fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_SAMPLER_BINDING, "sg_apply_bindings: sampler binding on fragment stage is missing or the sampler handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_UNEXPECTED_SAMPLER_COMPARE_NEVER, "sg_apply_bindings: shader expects SG_SAMPLERTYPE_COMPARISON on fragment stage but sampler has SG_COMPAREFUNC_NEVER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_SAMPLER_COMPARE_NEVER, "sg_apply_bindings: shader expects SG_SAMPLERTYPE_FILTERING on fragment stage but sampler doesn't have SG_COMPAREFUNC_NEVER") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_NONFILTERING_SAMPLER, "sg_apply_bindings: shader expected SG_SAMPLERTYPE_NONFILTERING on fragment stage, but sampler has SG_FILTER_LINEAR filters") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_UNEXPECTED_SAMPLER_BINDING, "sg_apply_bindings: unexpected sampler binding on fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_SMP_EXISTS, "sg_apply_bindings: sampler bound to fragment stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_EXPECTED_STORAGEBUFFER_BINDING, "sg_apply_bindings: storage buffer binding on fragment stage is missing or the buffer handle is invalid") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_STORAGEBUFFER_EXISTS, "sg_apply_bindings: storage buffer bound to fragment stage no longer alive") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_STORAGEBUFFER_BINDING_BUFFERTYPE, "sg_apply_bindings: buffer bound to frahment stage storage buffer slot is not of type storage buffer") \
    _SG_LOGITEM_XMACRO(VALIDATE_ABND_FS_UNEXPECTED_STORAGEBUFFER_BINDING, "sg_apply_bindings: unexpected storage buffer binding on fragment stage") \
    _SG_LOGITEM_XMACRO(VALIDATE_AUB_NO_PIPELINE, "sg_apply_uniforms: must be called after sg_apply_pipeline()") \
    _SG_LOGITEM_XMACRO(VALIDATE_AUB_NO_UB_AT_SLOT, "sg_apply_uniforms: no uniform block declaration at this shader stage UB slot") \
    _SG_LOGITEM_XMACRO(VALIDATE_AUB_SIZE, "sg_apply_uniforms: data size doesn't match declared uniform block size") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDATEBUF_USAGE, "sg_update_buffer: cannot update immutable buffer") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDATEBUF_SIZE, "sg_update_buffer: update size is bigger than buffer size") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDATEBUF_ONCE, "sg_update_buffer: only one update allowed per buffer and frame") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDATEBUF_APPEND, "sg_update_buffer: cannot call sg_update_buffer and sg_append_buffer in same frame") \
    _SG_LOGITEM_XMACRO(VALIDATE_APPENDBUF_USAGE, "sg_append_buffer: cannot append to immutable buffer") \
    _SG_LOGITEM_XMACRO(VALIDATE_APPENDBUF_SIZE, "sg_append_buffer: overall appended size is bigger than buffer size") \
    _SG_LOGITEM_XMACRO(VALIDATE_APPENDBUF_UPDATE, "sg_append_buffer: cannot call sg_append_buffer and sg_update_buffer in same frame") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDIMG_USAGE, "sg_update_image: cannot update immutable image") \
    _SG_LOGITEM_XMACRO(VALIDATE_UPDIMG_ONCE, "sg_update_image: only one update allowed per image and frame") \
    _SG_LOGITEM_XMACRO(VALIDATION_FAILED, "validation layer checks failed") \

#define _SG_LOGITEM_XMACRO(item,msg) SG_LOGITEM_##item,
typedef enum sg_log_item {
    _SG_LOG_ITEMS
} sg_log_item;
#undef _SG_LOGITEM_XMACRO

/*
    sg_desc

    The sg_desc struct contains configuration values for sokol_gfx,
    it is used as parameter to the sg_setup() call.

    The default configuration is:

    .buffer_pool_size       128
    .image_pool_size        128
    .sampler_pool_size      64
    .shader_pool_size       32
    .pipeline_pool_size     64
    .pass_pool_size         16
    .uniform_buffer_size    4 MB (4*1024*1024)
    .max_commit_listeners   1024
    .disable_validation     false
    .mtl_force_managed_storage_mode false
    .wgpu_disable_bindgroups_cache  false
    .wgpu_bindgroups_cache_size     1024

    .allocator.alloc_fn     0 (in this case, malloc() will be called)
    .allocator.free_fn      0 (in this case, free() will be called)
    .allocator.user_data    0

    .environment.defaults.color_format: default value depends on selected backend:
        all GL backends:    SG_PIXELFORMAT_RGBA8
        Metal and D3D11:    SG_PIXELFORMAT_BGRA8
        WebGPU:             *no default* (must be queried from WebGPU swapchain object)
    .environment.defaults.depth_format: SG_PIXELFORMAT_DEPTH_STENCIL
    .environment.defaults.sample_count: 1

    Metal specific:
        (NOTE: All Objective-C object references are transferred through
        a bridged (const void*) to sokol_gfx, which will use a unretained
        bridged cast (__bridged id<xxx>) to retrieve the Objective-C
        references back. Since the bridge cast is unretained, the caller
        must hold a strong reference to the Objective-C object for the
        duration of the sokol_gfx call!

        .mtl_force_managed_storage_mode
            when enabled, Metal buffers and texture resources are created in managed storage
            mode, otherwise sokol-gfx will decide whether to create buffers and
            textures in managed or shared storage mode (this is mainly a debugging option)
        .mtl_use_command_buffer_with_retained_references
            when true, the sokol-gfx Metal backend will use Metal command buffers which
            bump the reference count of resource objects as long as they are inflight,
            this is slower than the default command-buffer-with-unretained-references
            method, this may be a workaround when confronted with lifetime validation
            errors from the Metal validation layer until a proper fix has been implemented
        .environment.metal.device
            a pointer to the MTLDevice object

    D3D11 specific:
        .environment.d3d11.device
            a pointer to the ID3D11Device object, this must have been created
            before sg_setup() is called
        .environment.d3d11.device_context
            a pointer to the ID3D11DeviceContext object

    WebGPU specific:
        .wgpu_disable_bindgroups_cache
            When this is true, the WebGPU backend will create and immediately
            release a BindGroup object in the sg_apply_bindings() call, only
            use this for debugging purposes.
        .wgpu_bindgroups_cache_size
            The size of the bindgroups cache for re-using BindGroup objects
            between sg_apply_bindings() calls. The smaller the cache size,
            the more likely are cache slot collisions which will cause
            a BindGroups object to be destroyed and a new one created.
            Use the information returned by sg_query_stats() to check
            if this is a frequent occurrence, and increase the cache size as
            needed (the default is 1024).
            NOTE: wgpu_bindgroups_cache_size must be a power-of-2 number!
        .environment.wgpu.device
            a WGPUDevice handle

    When using sokol_gfx.h and sokol_app.h together, consider using the
    helper function sglue_environment() in the sokol_glue.h header to
    initialize the sg_desc.environment nested struct. sglue_environment() returns
    a completely initialized sg_environment struct with information
    provided by sokol_app.h.
*/
typedef struct sg_environment_defaults {
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
} sg_environment_defaults;

typedef struct sg_environment {
    sg_environment_defaults defaults;
} sg_environment;

/*
    sg_commit_listener

    Used with function sg_add_commit_listener() to add a callback
    which will be called in sg_commit(). This is useful for libraries
    building on top of sokol-gfx to be notified about when a frame
    ends (instead of having to guess, or add a manual 'new-frame'
    function.
*/
typedef struct sg_commit_listener {
    void (*func)(void* user_data);
    void* user_data;
} sg_commit_listener;

/*
    sg_allocator

    Used in sg_desc to provide custom memory-alloc and -free functions
    to sokol_gfx.h. If memory management should be overridden, both the
    alloc_fn and free_fn function must be provided (e.g. it's not valid to
    override one function but not the other).
*/
typedef struct sg_allocator {
    void* (*alloc_fn)(size_t size, void* user_data);
    void (*free_fn)(void* ptr, void* user_data);
    void* user_data;
} sg_allocator;

/*
    sg_logger

    Used in sg_desc to provide a logging function. Please be aware
    that without logging function, sokol-gfx will be completely
    silent, e.g. it will not report errors, warnings and
    validation layer messages. For maximum error verbosity,
    compile in debug mode (e.g. NDEBUG *not* defined) and provide a
    compatible logger function in the sg_setup() call
    (for instance the standard logging function from sokol_log.h).
*/
typedef struct sg_logger {
    void (*func)(
        const char* tag,                // always "sg"
        uint32_t log_level,             // 0=panic, 1=error, 2=warning, 3=info
        uint32_t log_item_id,           // SG_LOGITEM_*
        const char* message_or_null,    // a message string, may be nullptr in release mode
        uint32_t line_nr,               // line number in sokol_gfx.h
        const char* filename_or_null,   // source filename, may be nullptr in release mode
        void* user_data);
    void* user_data;
} sg_logger;

typedef struct sg_desc {
    uint32_t _start_canary;
    int buffer_pool_size;
    int image_pool_size;
    int sampler_pool_size;
    int shader_pool_size;
    int pipeline_pool_size;
    int attachments_pool_size;
    int uniform_buffer_size;
    int max_commit_listeners;
    bool disable_validation;    // disable validation layer even in debug mode, useful for tests
    sg_allocator allocator;
    sg_logger logger; // optional log function override
    sg_environment environment;
    uint32_t _end_canary;
} sg_desc;

// setup and misc functions
SOKOL_GFX_API_DECL void sg_setup(const sg_desc* desc);
SOKOL_GFX_API_DECL void sg_shutdown(void);
SOKOL_GFX_API_DECL bool sg_isvalid(void);
SOKOL_GFX_API_DECL void sg_reset_state_cache(void);
SOKOL_GFX_API_DECL sg_trace_hooks sg_install_trace_hooks(const sg_trace_hooks* trace_hooks);
SOKOL_GFX_API_DECL void sg_push_debug_group(const char* name);
SOKOL_GFX_API_DECL void sg_pop_debug_group(void);
SOKOL_GFX_API_DECL bool sg_add_commit_listener(sg_commit_listener listener);
SOKOL_GFX_API_DECL bool sg_remove_commit_listener(sg_commit_listener listener);

// resource creation, destruction and updating
SOKOL_GFX_API_DECL sg_buffer sg_make_buffer(const sg_buffer_desc* desc);
SOKOL_GFX_API_DECL sg_image sg_make_image(const sg_image_desc* desc);
SOKOL_GFX_API_DECL sg_sampler sg_make_sampler(const sg_sampler_desc* desc);
SOKOL_GFX_API_DECL sg_shader sg_make_shader(const sg_shader_desc* desc);
SOKOL_GFX_API_DECL sg_pipeline sg_make_pipeline(const sg_pipeline_desc* desc);
SOKOL_GFX_API_DECL sg_attachments sg_make_attachments(const sg_attachments_desc* desc);
SOKOL_GFX_API_DECL void sg_destroy_buffer(sg_buffer buf);
SOKOL_GFX_API_DECL void sg_destroy_image(sg_image img);
SOKOL_GFX_API_DECL void sg_destroy_sampler(sg_sampler smp);
SOKOL_GFX_API_DECL void sg_destroy_shader(sg_shader shd);
SOKOL_GFX_API_DECL void sg_destroy_pipeline(sg_pipeline pip);
SOKOL_GFX_API_DECL void sg_destroy_attachments(sg_attachments atts);
SOKOL_GFX_API_DECL void sg_update_buffer(sg_buffer buf, const sg_range* data);
SOKOL_GFX_API_DECL void sg_update_image(sg_image img, const sg_image_data* data);
SOKOL_GFX_API_DECL int sg_append_buffer(sg_buffer buf, const sg_range* data);
SOKOL_GFX_API_DECL bool sg_query_buffer_overflow(sg_buffer buf);
SOKOL_GFX_API_DECL bool sg_query_buffer_will_overflow(sg_buffer buf, size_t size);

// rendering functions
SOKOL_GFX_API_DECL void sg_begin_pass(const sg_pass* pass);
SOKOL_GFX_API_DECL void sg_apply_viewport(int x, int y, int width, int height, bool origin_top_left);
SOKOL_GFX_API_DECL void sg_apply_viewportf(float x, float y, float width, float height, bool origin_top_left);
SOKOL_GFX_API_DECL void sg_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
SOKOL_GFX_API_DECL void sg_apply_scissor_rectf(float x, float y, float width, float height, bool origin_top_left);
SOKOL_GFX_API_DECL void sg_apply_pipeline(sg_pipeline pip);
SOKOL_GFX_API_DECL void sg_apply_bindings(const sg_bindings* bindings);
SOKOL_GFX_API_DECL void sg_apply_uniforms(sg_shader_stage stage, int ub_index, const sg_range* data);
SOKOL_GFX_API_DECL void sg_draw(int base_element, int num_elements, int num_instances);
SOKOL_GFX_API_DECL void sg_end_pass(void);
SOKOL_GFX_API_DECL void sg_commit(void);

// getting information
SOKOL_GFX_API_DECL sg_desc sg_query_desc(void);
SOKOL_GFX_API_DECL sg_backend sg_query_backend(void);
SOKOL_GFX_API_DECL sg_features sg_query_features(void);
SOKOL_GFX_API_DECL sg_limits sg_query_limits(void);
SOKOL_GFX_API_DECL sg_pixelformat_info sg_query_pixelformat(sg_pixel_format fmt);
SOKOL_GFX_API_DECL int sg_query_row_pitch(sg_pixel_format fmt, int width, int row_align_bytes);
SOKOL_GFX_API_DECL int sg_query_surface_pitch(sg_pixel_format fmt, int width, int height, int row_align_bytes);
// get current state of a resource (INITIAL, ALLOC, VALID, FAILED, INVALID)
SOKOL_GFX_API_DECL sg_resource_state sg_query_buffer_state(sg_buffer buf);
SOKOL_GFX_API_DECL sg_resource_state sg_query_image_state(sg_image img);
SOKOL_GFX_API_DECL sg_resource_state sg_query_sampler_state(sg_sampler smp);
SOKOL_GFX_API_DECL sg_resource_state sg_query_shader_state(sg_shader shd);
SOKOL_GFX_API_DECL sg_resource_state sg_query_pipeline_state(sg_pipeline pip);
SOKOL_GFX_API_DECL sg_resource_state sg_query_attachments_state(sg_attachments atts);
// get runtime information about a resource
SOKOL_GFX_API_DECL sg_buffer_info sg_query_buffer_info(sg_buffer buf);
SOKOL_GFX_API_DECL sg_image_info sg_query_image_info(sg_image img);
SOKOL_GFX_API_DECL sg_sampler_info sg_query_sampler_info(sg_sampler smp);
SOKOL_GFX_API_DECL sg_shader_info sg_query_shader_info(sg_shader shd);
SOKOL_GFX_API_DECL sg_pipeline_info sg_query_pipeline_info(sg_pipeline pip);
SOKOL_GFX_API_DECL sg_attachments_info sg_query_attachments_info(sg_attachments atts);
// get desc structs matching a specific resource (NOTE that not all creation attributes may be provided)
SOKOL_GFX_API_DECL sg_buffer_desc sg_query_buffer_desc(sg_buffer buf);
SOKOL_GFX_API_DECL sg_image_desc sg_query_image_desc(sg_image img);
SOKOL_GFX_API_DECL sg_sampler_desc sg_query_sampler_desc(sg_sampler smp);
SOKOL_GFX_API_DECL sg_shader_desc sg_query_shader_desc(sg_shader shd);
SOKOL_GFX_API_DECL sg_pipeline_desc sg_query_pipeline_desc(sg_pipeline pip);
SOKOL_GFX_API_DECL sg_attachments_desc sg_query_attachments_desc(sg_attachments atts);
// get resource creation desc struct with their default values replaced
SOKOL_GFX_API_DECL sg_buffer_desc sg_query_buffer_defaults(const sg_buffer_desc* desc);
SOKOL_GFX_API_DECL sg_image_desc sg_query_image_defaults(const sg_image_desc* desc);
SOKOL_GFX_API_DECL sg_sampler_desc sg_query_sampler_defaults(const sg_sampler_desc* desc);
SOKOL_GFX_API_DECL sg_shader_desc sg_query_shader_defaults(const sg_shader_desc* desc);
SOKOL_GFX_API_DECL sg_pipeline_desc sg_query_pipeline_defaults(const sg_pipeline_desc* desc);
SOKOL_GFX_API_DECL sg_attachments_desc sg_query_attachments_defaults(const sg_attachments_desc* desc);

// separate resource allocation and initialization (for async setup)
SOKOL_GFX_API_DECL sg_buffer sg_alloc_buffer(void);
SOKOL_GFX_API_DECL sg_image sg_alloc_image(void);
SOKOL_GFX_API_DECL sg_sampler sg_alloc_sampler(void);
SOKOL_GFX_API_DECL sg_shader sg_alloc_shader(void);
SOKOL_GFX_API_DECL sg_pipeline sg_alloc_pipeline(void);
SOKOL_GFX_API_DECL sg_attachments sg_alloc_attachments(void);
SOKOL_GFX_API_DECL void sg_dealloc_buffer(sg_buffer buf);
SOKOL_GFX_API_DECL void sg_dealloc_image(sg_image img);
SOKOL_GFX_API_DECL void sg_dealloc_sampler(sg_sampler smp);
SOKOL_GFX_API_DECL void sg_dealloc_shader(sg_shader shd);
SOKOL_GFX_API_DECL void sg_dealloc_pipeline(sg_pipeline pip);
SOKOL_GFX_API_DECL void sg_dealloc_attachments(sg_attachments attachments);
SOKOL_GFX_API_DECL void sg_init_buffer(sg_buffer buf, const sg_buffer_desc* desc);
SOKOL_GFX_API_DECL void sg_init_image(sg_image img, const sg_image_desc* desc);
SOKOL_GFX_API_DECL void sg_init_sampler(sg_sampler smg, const sg_sampler_desc* desc);
SOKOL_GFX_API_DECL void sg_init_shader(sg_shader shd, const sg_shader_desc* desc);
SOKOL_GFX_API_DECL void sg_init_pipeline(sg_pipeline pip, const sg_pipeline_desc* desc);
SOKOL_GFX_API_DECL void sg_init_attachments(sg_attachments attachments, const sg_attachments_desc* desc);
SOKOL_GFX_API_DECL void sg_uninit_buffer(sg_buffer buf);
SOKOL_GFX_API_DECL void sg_uninit_image(sg_image img);
SOKOL_GFX_API_DECL void sg_uninit_sampler(sg_sampler smp);
SOKOL_GFX_API_DECL void sg_uninit_shader(sg_shader shd);
SOKOL_GFX_API_DECL void sg_uninit_pipeline(sg_pipeline pip);
SOKOL_GFX_API_DECL void sg_uninit_attachments(sg_attachments atts);
SOKOL_GFX_API_DECL void sg_fail_buffer(sg_buffer buf);
SOKOL_GFX_API_DECL void sg_fail_image(sg_image img);
SOKOL_GFX_API_DECL void sg_fail_sampler(sg_sampler smp);
SOKOL_GFX_API_DECL void sg_fail_shader(sg_shader shd);
SOKOL_GFX_API_DECL void sg_fail_pipeline(sg_pipeline pip);
SOKOL_GFX_API_DECL void sg_fail_attachments(sg_attachments atts);

// frame stats
SOKOL_GFX_API_DECL void sg_enable_frame_stats(void);
SOKOL_GFX_API_DECL void sg_disable_frame_stats(void);
SOKOL_GFX_API_DECL bool sg_frame_stats_enabled(void);
SOKOL_GFX_API_DECL sg_frame_stats sg_query_frame_stats(void);

/* Backend-specific structs and functions, these may come in handy for mixing
   sokol-gfx rendering with 'native backend' rendering functions.

   This group of functions will be expanded as needed.
*/

typedef struct sg_gl_buffer_info {
    uint32_t buf[SG_NUM_INFLIGHT_FRAMES];
    int active_slot;
} sg_gl_buffer_info;

typedef struct sg_gl_image_info {
    uint32_t tex[SG_NUM_INFLIGHT_FRAMES];
    uint32_t tex_target;
    uint32_t msaa_render_buffer;
    int active_slot;
} sg_gl_image_info;

typedef struct sg_gl_sampler_info {
    uint32_t smp;
} sg_gl_sampler_info;

typedef struct sg_gl_shader_info {
    uint32_t prog;
} sg_gl_shader_info;

typedef struct sg_gl_attachments_info {
    uint32_t framebuffer;
    uint32_t msaa_resolve_framebuffer[SG_MAX_COLOR_ATTACHMENTS];
} sg_gl_attachments_info;

// GL: get internal buffer resource objects
SOKOL_GFX_API_DECL sg_gl_buffer_info sg_gl_query_buffer_info(sg_buffer buf);
// GL: get internal image resource objects
SOKOL_GFX_API_DECL sg_gl_image_info sg_gl_query_image_info(sg_image img);
// GL: get internal sampler resource objects
SOKOL_GFX_API_DECL sg_gl_sampler_info sg_gl_query_sampler_info(sg_sampler smp);
// GL: get internal shader resource objects
SOKOL_GFX_API_DECL sg_gl_shader_info sg_gl_query_shader_info(sg_shader shd);
// GL: get internal pass resource objects
SOKOL_GFX_API_DECL sg_gl_attachments_info sg_gl_query_attachments_info(sg_attachments atts);

#ifdef __cplusplus
} // extern "C"

// reference-based equivalents for c++
inline void sg_setup(const sg_desc& desc) { return sg_setup(&desc); }

inline sg_buffer sg_make_buffer(const sg_buffer_desc& desc) { return sg_make_buffer(&desc); }
inline sg_image sg_make_image(const sg_image_desc& desc) { return sg_make_image(&desc); }
inline sg_sampler sg_make_sampler(const sg_sampler_desc& desc) { return sg_make_sampler(&desc); }
inline sg_shader sg_make_shader(const sg_shader_desc& desc) { return sg_make_shader(&desc); }
inline sg_pipeline sg_make_pipeline(const sg_pipeline_desc& desc) { return sg_make_pipeline(&desc); }
inline sg_attachments sg_make_attachments(const sg_attachments_desc& desc) { return sg_make_attachments(&desc); }
inline void sg_update_image(sg_image img, const sg_image_data& data) { return sg_update_image(img, &data); }

inline void sg_begin_pass(const sg_pass& pass) { return sg_begin_pass(&pass); }
inline void sg_apply_bindings(const sg_bindings& bindings) { return sg_apply_bindings(&bindings); }
inline void sg_apply_uniforms(sg_shader_stage stage, int ub_index, const sg_range& data) { return sg_apply_uniforms(stage, ub_index, &data); }

inline sg_buffer_desc sg_query_buffer_defaults(const sg_buffer_desc& desc) { return sg_query_buffer_defaults(&desc); }
inline sg_image_desc sg_query_image_defaults(const sg_image_desc& desc) { return sg_query_image_defaults(&desc); }
inline sg_sampler_desc sg_query_sampler_defaults(const sg_sampler_desc& desc) { return sg_query_sampler_defaults(&desc); }
inline sg_shader_desc sg_query_shader_defaults(const sg_shader_desc& desc) { return sg_query_shader_defaults(&desc); }
inline sg_pipeline_desc sg_query_pipeline_defaults(const sg_pipeline_desc& desc) { return sg_query_pipeline_defaults(&desc); }
inline sg_attachments_desc sg_query_attachments_defaults(const sg_attachments_desc& desc) { return sg_query_attachments_defaults(&desc); }

inline void sg_init_buffer(sg_buffer buf, const sg_buffer_desc& desc) { return sg_init_buffer(buf, &desc); }
inline void sg_init_image(sg_image img, const sg_image_desc& desc) { return sg_init_image(img, &desc); }
inline void sg_init_sampler(sg_sampler smp, const sg_sampler_desc& desc) { return sg_init_sampler(smp, &desc); }
inline void sg_init_shader(sg_shader shd, const sg_shader_desc& desc) { return sg_init_shader(shd, &desc); }
inline void sg_init_pipeline(sg_pipeline pip, const sg_pipeline_desc& desc) { return sg_init_pipeline(pip, &desc); }
inline void sg_init_attachments(sg_attachments atts, const sg_attachments_desc& desc) { return sg_init_attachments(atts, &desc); }

inline void sg_update_buffer(sg_buffer buf_id, const sg_range& data) { return sg_update_buffer(buf_id, &data); }
inline int sg_append_buffer(sg_buffer buf_id, const sg_range& data) { return sg_append_buffer(buf_id, &data); }
#endif
#endif // SOKOL_GFX_INCLUDED

// ██ ███    ███ ██████  ██      ███████ ███    ███ ███████ ███    ██ ████████  █████  ████████ ██  ██████  ███    ██
// ██ ████  ████ ██   ██ ██      ██      ████  ████ ██      ████   ██    ██    ██   ██    ██    ██ ██    ██ ████   ██
// ██ ██ ████ ██ ██████  ██      █████   ██ ████ ██ █████   ██ ██  ██    ██    ███████    ██    ██ ██    ██ ██ ██  ██
// ██ ██  ██  ██ ██      ██      ██      ██  ██  ██ ██      ██  ██ ██    ██    ██   ██    ██    ██ ██    ██ ██  ██ ██
// ██ ██      ██ ██      ███████ ███████ ██      ██ ███████ ██   ████    ██    ██   ██    ██    ██  ██████  ██   ████
//
// >>implementation
#ifdef SOKOL_GFX_IMPL
#define SOKOL_GFX_IMPL_INCLUDED (1)

#if !(defined(SOKOL_GLCORE)||defined(SOKOL_GLES3)||defined(SOKOL_D3D11)||defined(SOKOL_METAL)||defined(SOKOL_WGPU)||defined(SOKOL_DUMMY_BACKEND))
#error "Please select a backend with SOKOL_GLCORE, SOKOL_GLES3, SOKOL_D3D11, SOKOL_METAL, SOKOL_WGPU or SOKOL_DUMMY_BACKEND"
#endif
#if defined(SOKOL_MALLOC) || defined(SOKOL_CALLOC) || defined(SOKOL_FREE)
#error "SOKOL_MALLOC/CALLOC/FREE macros are no longer supported, please use sg_desc.allocator to override memory allocation functions"
#endif

#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <float.h> // FLT_MAX

#ifndef SOKOL_API_IMPL
    #define SOKOL_API_IMPL
#endif
#ifndef SOKOL_DEBUG
    #ifndef NDEBUG
        #define SOKOL_DEBUG
    #endif
#endif
#ifndef SOKOL_ASSERT
    #include <assert.h>
    #define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_UNREACHABLE
    #define SOKOL_UNREACHABLE SOKOL_ASSERT(false)
#endif

#ifndef _SOKOL_PRIVATE
    #if defined(__GNUC__) || defined(__clang__)
        #define _SOKOL_PRIVATE __attribute__((unused)) static
    #else
        #define _SOKOL_PRIVATE static
    #endif
#endif

#ifndef _SOKOL_UNUSED
    #define _SOKOL_UNUSED(x) (void)(x)
#endif

#if defined(SOKOL_TRACE_HOOKS)
#define _SG_TRACE_ARGS(fn, ...) if (_sg.hooks.fn) { _sg.hooks.fn(__VA_ARGS__, _sg.hooks.user_data); }
#define _SG_TRACE_NOARGS(fn) if (_sg.hooks.fn) { _sg.hooks.fn(_sg.hooks.user_data); }
#else
#define _SG_TRACE_ARGS(fn, ...)
#define _SG_TRACE_NOARGS(fn)
#endif

// default clear values
#ifndef SG_DEFAULT_CLEAR_RED
#define SG_DEFAULT_CLEAR_RED (0.5f)
#endif
#ifndef SG_DEFAULT_CLEAR_GREEN
#define SG_DEFAULT_CLEAR_GREEN (0.5f)
#endif
#ifndef SG_DEFAULT_CLEAR_BLUE
#define SG_DEFAULT_CLEAR_BLUE (0.5f)
#endif
#ifndef SG_DEFAULT_CLEAR_ALPHA
#define SG_DEFAULT_CLEAR_ALPHA (1.0f)
#endif
#ifndef SG_DEFAULT_CLEAR_DEPTH
#define SG_DEFAULT_CLEAR_DEPTH (1.0f)
#endif
#ifndef SG_DEFAULT_CLEAR_STENCIL
#define SG_DEFAULT_CLEAR_STENCIL (0)
#endif

#if defined(SOKOL_GLCORE) || defined(SOKOL_GLES3)
    #define _SOKOL_ANY_GL (1)

    // include platform specific GL headers (or on Win32: use an embedded GL loader)
    #if !defined(SOKOL_EXTERNAL_GL_LOADER)
        #if defined(_WIN32)
            #if defined(SOKOL_GLCORE) && !defined(SOKOL_EXTERNAL_GL_LOADER)
                #ifndef WIN32_LEAN_AND_MEAN
                #define WIN32_LEAN_AND_MEAN
                #endif
                #ifndef NOMINMAX
                #define NOMINMAX
                #endif
                #include <windows.h>
                #define _SOKOL_USE_WIN32_GL_LOADER (1)
                #pragma comment (lib, "kernel32")   // GetProcAddress()
            #endif
        #elif defined(__APPLE__)
            #include <TargetConditionals.h>
            #ifndef GL_SILENCE_DEPRECATION
                #define GL_SILENCE_DEPRECATION
            #endif
            #if defined(TARGET_OS_IPHONE) && !TARGET_OS_IPHONE
                #include <OpenGL/gl3.h>
            #else
                #include <OpenGLES/ES3/gl.h>
                #include <OpenGLES/ES3/glext.h>
            #endif
        #elif defined(__EMSCRIPTEN__) || defined(__ANDROID__)
            #if defined(SOKOL_GLES3)
                #include <GLES3/gl3.h>
            #endif
        #elif defined(__linux__) || defined(__unix__)
            #if defined(SOKOL_GLCORE)
                #define GL_GLEXT_PROTOTYPES
                #include <GL/gl.h>
            #else
                #include <GLES3/gl3.h>
                #include <GLES3/gl3ext.h>
            #endif
        #endif
    #endif

    // optional GL loader definitions (only on Win32)
    #if defined(_SOKOL_USE_WIN32_GL_LOADER)
        #define __gl_h_ 1
        #define __gl32_h_ 1
        #define __gl31_h_ 1
        #define __GL_H__ 1
        #define __glext_h_ 1
        #define __GLEXT_H_ 1
        #define __gltypes_h_ 1
        #define __glcorearb_h_ 1
        #define __gl_glcorearb_h_ 1
        #define GL_APIENTRY APIENTRY

        typedef unsigned int  GLenum;
        typedef unsigned int  GLuint;
        typedef int  GLsizei;
        typedef char  GLchar;
        typedef ptrdiff_t  GLintptr;
        typedef ptrdiff_t  GLsizeiptr;
        typedef double  GLclampd;
        typedef unsigned short  GLushort;
        typedef unsigned char  GLubyte;
        typedef unsigned char  GLboolean;
        typedef uint64_t  GLuint64;
        typedef double  GLdouble;
        typedef unsigned short  GLhalf;
        typedef float  GLclampf;
        typedef unsigned int  GLbitfield;
        typedef signed char  GLbyte;
        typedef short  GLshort;
        typedef void  GLvoid;
        typedef int64_t  GLint64;
        typedef float  GLfloat;
        typedef int  GLint;
        #define GL_INT_2_10_10_10_REV 0x8D9F
        #define GL_R32F 0x822E
        #define GL_PROGRAM_POINT_SIZE 0x8642
        #define GL_DEPTH_ATTACHMENT 0x8D00
        #define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
        #define GL_COLOR_ATTACHMENT2 0x8CE2
        #define GL_COLOR_ATTACHMENT0 0x8CE0
        #define GL_R16F 0x822D
        #define GL_COLOR_ATTACHMENT22 0x8CF6
        #define GL_DRAW_FRAMEBUFFER 0x8CA9
        #define GL_FRAMEBUFFER_COMPLETE 0x8CD5
        #define GL_NUM_EXTENSIONS 0x821D
        #define GL_INFO_LOG_LENGTH 0x8B84
        #define GL_VERTEX_SHADER 0x8B31
        #define GL_INCR 0x1E02
        #define GL_DYNAMIC_DRAW 0x88E8
        #define GL_STATIC_DRAW 0x88E4
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
        #define GL_TEXTURE_CUBE_MAP 0x8513
        #define GL_FUNC_SUBTRACT 0x800A
        #define GL_FUNC_REVERSE_SUBTRACT 0x800B
        #define GL_CONSTANT_COLOR 0x8001
        #define GL_DECR_WRAP 0x8508
        #define GL_R8 0x8229
        #define GL_LINEAR_MIPMAP_LINEAR 0x2703
        #define GL_ELEMENT_ARRAY_BUFFER 0x8893
        #define GL_SHORT 0x1402
        #define GL_DEPTH_TEST 0x0B71
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
        #define GL_LINK_STATUS 0x8B82
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
        #define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
        #define GL_RGBA16F 0x881A
        #define GL_CONSTANT_ALPHA 0x8003
        #define GL_READ_FRAMEBUFFER 0x8CA8
        #define GL_TEXTURE0 0x84C0
        #define GL_TEXTURE_MIN_LOD 0x813A
        #define GL_CLAMP_TO_EDGE 0x812F
        #define GL_UNSIGNED_SHORT_5_6_5 0x8363
        #define GL_TEXTURE_WRAP_R 0x8072
        #define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
        #define GL_NEAREST_MIPMAP_NEAREST 0x2700
        #define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
        #define GL_SRC_ALPHA_SATURATE 0x0308
        #define GL_STREAM_DRAW 0x88E0
        #define GL_ONE 1
        #define GL_NEAREST_MIPMAP_LINEAR 0x2702
        #define GL_RGB10_A2 0x8059
        #define GL_RGBA8 0x8058
        #define GL_SRGB8_ALPHA8 0x8C43
        #define GL_COLOR_ATTACHMENT1 0x8CE1
        #define GL_RGBA4 0x8056
        #define GL_RGB8 0x8051
        #define GL_ARRAY_BUFFER 0x8892
        #define GL_STENCIL 0x1802
        #define GL_TEXTURE_2D 0x0DE1
        #define GL_DEPTH 0x1801
        #define GL_FRONT 0x0404
        #define GL_STENCIL_BUFFER_BIT 0x00000400
        #define GL_REPEAT 0x2901
        #define GL_RGBA 0x1908
        #define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
        #define GL_DECR 0x1E03
        #define GL_FRAGMENT_SHADER 0x8B30
        #define GL_FLOAT 0x1406
        #define GL_TEXTURE_MAX_LOD 0x813B
        #define GL_DEPTH_COMPONENT 0x1902
        #define GL_ONE_MINUS_DST_ALPHA 0x0305
        #define GL_COLOR 0x1800
        #define GL_TEXTURE_2D_ARRAY 0x8C1A
        #define GL_TRIANGLES 0x0004
        #define GL_UNSIGNED_BYTE 0x1401
        #define GL_TEXTURE_MAG_FILTER 0x2800
        #define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
        #define GL_NONE 0
        #define GL_SRC_COLOR 0x0300
        #define GL_BYTE 0x1400
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
        #define GL_LINE_STRIP 0x0003
        #define GL_TEXTURE_3D 0x806F
        #define GL_CW 0x0900
        #define GL_LINEAR 0x2601
        #define GL_RENDERBUFFER 0x8D41
        #define GL_GEQUAL 0x0206
        #define GL_COLOR_BUFFER_BIT 0x00004000
        #define GL_RGBA32F 0x8814
        #define GL_BLEND 0x0BE2
        #define GL_ONE_MINUS_SRC_ALPHA 0x0303
        #define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
        #define GL_TEXTURE_WRAP_T 0x2803
        #define GL_TEXTURE_WRAP_S 0x2802
        #define GL_TEXTURE_MIN_FILTER 0x2801
        #define GL_LINEAR_MIPMAP_NEAREST 0x2701
        #define GL_EXTENSIONS 0x1F03
        #define GL_NO_ERROR 0
        #define GL_REPLACE 0x1E01
        #define GL_KEEP 0x1E00
        #define GL_CCW 0x0901
        #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
        #define GL_RGB 0x1907
        #define GL_TRIANGLE_STRIP 0x0005
        #define GL_FALSE 0
        #define GL_ZERO 0
        #define GL_CULL_FACE 0x0B44
        #define GL_INVERT 0x150A
        #define GL_INT 0x1404
        #define GL_UNSIGNED_INT 0x1405
        #define GL_UNSIGNED_SHORT 0x1403
        #define GL_NEAREST 0x2600
        #define GL_SCISSOR_TEST 0x0C11
        #define GL_LEQUAL 0x0203
        #define GL_STENCIL_TEST 0x0B90
        #define GL_DITHER 0x0BD0
        #define GL_DEPTH_COMPONENT32F 0x8CAC
        #define GL_EQUAL 0x0202
        #define GL_FRAMEBUFFER 0x8D40
        #define GL_RGB5 0x8050
        #define GL_LINES 0x0001
        #define GL_DEPTH_BUFFER_BIT 0x00000100
        #define GL_SRC_ALPHA 0x0302
        #define GL_INCR_WRAP 0x8507
        #define GL_LESS 0x0201
        #define GL_MULTISAMPLE 0x809D
        #define GL_FRAMEBUFFER_BINDING 0x8CA6
        #define GL_BACK 0x0405
        #define GL_ALWAYS 0x0207
        #define GL_FUNC_ADD 0x8006
        #define GL_ONE_MINUS_DST_COLOR 0x0307
        #define GL_NOTEQUAL 0x0205
        #define GL_DST_COLOR 0x0306
        #define GL_COMPILE_STATUS 0x8B81
        #define GL_RED 0x1903
        #define GL_COLOR_ATTACHMENT3 0x8CE3
        #define GL_DST_ALPHA 0x0304
        #define GL_RGB5_A1 0x8057
        #define GL_GREATER 0x0204
        #define GL_POLYGON_OFFSET_FILL 0x8037
        #define GL_TRUE 1
        #define GL_NEVER 0x0200
        #define GL_POINTS 0x0000
        #define GL_ONE_MINUS_SRC_COLOR 0x0301
        #define GL_MIRRORED_REPEAT 0x8370
        #define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
        #define GL_R11F_G11F_B10F 0x8C3A
        #define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
        #define GL_RGB9_E5 0x8C3D
        #define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
        #define GL_RGBA32UI 0x8D70
        #define GL_RGB32UI 0x8D71
        #define GL_RGBA16UI 0x8D76
        #define GL_RGB16UI 0x8D77
        #define GL_RGBA8UI 0x8D7C
        #define GL_RGB8UI 0x8D7D
        #define GL_RGBA32I 0x8D82
        #define GL_RGB32I 0x8D83
        #define GL_RGBA16I 0x8D88
        #define GL_RGB16I 0x8D89
        #define GL_RGBA8I 0x8D8E
        #define GL_RGB8I 0x8D8F
        #define GL_RED_INTEGER 0x8D94
        #define GL_RG 0x8227
        #define GL_RG_INTEGER 0x8228
        #define GL_R8 0x8229
        #define GL_R16 0x822A
        #define GL_RG8 0x822B
        #define GL_RG16 0x822C
        #define GL_R16F 0x822D
        #define GL_R32F 0x822E
        #define GL_RG16F 0x822F
        #define GL_RG32F 0x8230
        #define GL_R8I 0x8231
        #define GL_R8UI 0x8232
        #define GL_R16I 0x8233
        #define GL_R16UI 0x8234
        #define GL_R32I 0x8235
        #define GL_R32UI 0x8236
        #define GL_RG8I 0x8237
        #define GL_RG8UI 0x8238
        #define GL_RG16I 0x8239
        #define GL_RG16UI 0x823A
        #define GL_RG32I 0x823B
        #define GL_RG32UI 0x823C
        #define GL_RGBA_INTEGER 0x8D99
        #define GL_R8_SNORM 0x8F94
        #define GL_RG8_SNORM 0x8F95
        #define GL_RGB8_SNORM 0x8F96
        #define GL_RGBA8_SNORM 0x8F97
        #define GL_R16_SNORM 0x8F98
        #define GL_RG16_SNORM 0x8F99
        #define GL_RGB16_SNORM 0x8F9A
        #define GL_RGBA16_SNORM 0x8F9B
        #define GL_RGBA16 0x805B
        #define GL_MAX_TEXTURE_SIZE 0x0D33
        #define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
        #define GL_MAX_3D_TEXTURE_SIZE 0x8073
        #define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
        #define GL_MAX_VERTEX_ATTRIBS 0x8869
        #define GL_CLAMP_TO_BORDER 0x812D
        #define GL_TEXTURE_BORDER_COLOR 0x1004
        #define GL_CURRENT_PROGRAM 0x8B8D
        #define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
        #define GL_UNPACK_ALIGNMENT 0x0CF5
        #define GL_FRAMEBUFFER_SRGB 0x8DB9
        #define GL_TEXTURE_COMPARE_MODE 0x884C
        #define GL_TEXTURE_COMPARE_FUNC 0x884D
        #define GL_COMPARE_REF_TO_TEXTURE 0x884E
        #define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
        #define GL_TEXTURE_MAX_LEVEL 0x813D
        #define GL_FRAMEBUFFER_UNDEFINED 0x8219
        #define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
        #define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
        #define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
        #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
        #define GL_MAJOR_VERSION 0x821B
        #define GL_MINOR_VERSION 0x821C
    #endif

    #ifndef GL_UNSIGNED_INT_2_10_10_10_REV
    #define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
    #endif
    #ifndef GL_UNSIGNED_INT_24_8
    #define GL_UNSIGNED_INT_24_8 0x84FA
    #endif
    #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
    #endif
    #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
    #endif
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
    #endif
    #ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
    #endif
    #ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
    #endif
    #ifndef GL_COMPRESSED_RED_RGTC1
    #define GL_COMPRESSED_RED_RGTC1 0x8DBB
    #endif
    #ifndef GL_COMPRESSED_SIGNED_RED_RGTC1
    #define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
    #endif
    #ifndef GL_COMPRESSED_RED_GREEN_RGTC2
    #define GL_COMPRESSED_RED_GREEN_RGTC2 0x8DBD
    #endif
    #ifndef GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2
    #define GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2 0x8DBE
    #endif
    #ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
    #define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB 0x8E8C
    #endif
    #ifndef GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB
    #define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB 0x8E8D
    #endif
    #ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    #define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB 0x8E8E
    #endif
    #ifndef GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    #define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB 0x8E8F
    #endif
    #ifndef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
    #endif
    #ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
    #endif
    #ifndef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
    #endif
    #ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
    #endif
    #ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2 0x9274
    #endif
    #ifndef GL_COMPRESSED_SRGB8_ETC2
    #define GL_COMPRESSED_SRGB8_ETC2 0x9275
    #endif
    #ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
    #endif
    #ifndef GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
    #define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
    #endif
    #ifndef GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
    #define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
    #endif
    #ifndef GL_COMPRESSED_R11_EAC
    #define GL_COMPRESSED_R11_EAC 0x9270
    #endif
    #ifndef GL_COMPRESSED_SIGNED_R11_EAC
    #define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
    #endif
    #ifndef GL_COMPRESSED_RG11_EAC
    #define GL_COMPRESSED_RG11_EAC 0x9272
    #endif
    #ifndef GL_COMPRESSED_SIGNED_RG11_EAC
    #define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
    #endif
    #ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
    #endif
    #ifndef GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
    #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
    #endif
    #ifndef GL_DEPTH24_STENCIL8
    #define GL_DEPTH24_STENCIL8 0x88F0
    #endif
    #ifndef GL_HALF_FLOAT
    #define GL_HALF_FLOAT 0x140B
    #endif
    #ifndef GL_DEPTH_STENCIL
    #define GL_DEPTH_STENCIL 0x84F9
    #endif
    #ifndef GL_LUMINANCE
    #define GL_LUMINANCE 0x1909
    #endif
    #ifndef _SG_GL_CHECK_ERROR
    #define _SG_GL_CHECK_ERROR() { SOKOL_ASSERT(glGetError() == GL_NO_ERROR); }
    #endif
#endif

#if defined(SOKOL_GLES3)
    // on WebGL2, GL_FRAMEBUFFER_UNDEFINED technically doesn't exist (it is defined
    // in the Emscripten headers, but may not exist in other WebGL2 shims)
    // see: https://github.com/floooh/sokol/pull/933
    #ifndef GL_FRAMEBUFFER_UNDEFINED
    #define GL_FRAMEBUFFER_UNDEFINED 0x8219
    #endif
#endif

// make some GL constants generally available to simplify compilation,
// use of those constants will be filtered by runtime flags
#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

// ███████ ████████ ██████  ██    ██  ██████ ████████ ███████
// ██         ██    ██   ██ ██    ██ ██         ██    ██
// ███████    ██    ██████  ██    ██ ██         ██    ███████
//      ██    ██    ██   ██ ██    ██ ██         ██         ██
// ███████    ██    ██   ██  ██████   ██████    ██    ███████
//
// >>structs
// resource pool slots
typedef struct {
    uint32_t id;
    sg_resource_state state;
} _sg_slot_t;

// resource pool housekeeping struct
typedef struct {
    int size;
    int queue_top;
    uint32_t* gen_ctrs;
    int* free_queue;
} _sg_pool_t;

_SOKOL_PRIVATE void _sg_init_pool(_sg_pool_t* pool, int num);
_SOKOL_PRIVATE void _sg_discard_pool(_sg_pool_t* pool);
_SOKOL_PRIVATE int _sg_pool_alloc_index(_sg_pool_t* pool);
_SOKOL_PRIVATE void _sg_pool_free_index(_sg_pool_t* pool, int slot_index);
_SOKOL_PRIVATE void _sg_reset_slot(_sg_slot_t* slot);
_SOKOL_PRIVATE uint32_t _sg_slot_alloc(_sg_pool_t* pool, _sg_slot_t* slot, int slot_index);
_SOKOL_PRIVATE int _sg_slot_index(uint32_t id);

// constants
enum {
    _SG_STRING_SIZE = 32,
    _SG_SLOT_SHIFT = 16,
    _SG_SLOT_MASK = (1<<_SG_SLOT_SHIFT)-1,
    _SG_MAX_POOL_SIZE = (1<<_SG_SLOT_SHIFT),
    _SG_DEFAULT_BUFFER_POOL_SIZE = 128,
    _SG_DEFAULT_IMAGE_POOL_SIZE = 128,
    _SG_DEFAULT_SAMPLER_POOL_SIZE = 64,
    _SG_DEFAULT_SHADER_POOL_SIZE = 32,
    _SG_DEFAULT_PIPELINE_POOL_SIZE = 64,
    _SG_DEFAULT_ATTACHMENTS_POOL_SIZE = 16,
    _SG_DEFAULT_UB_SIZE = 4 * 1024 * 1024,
    _SG_DEFAULT_MAX_COMMIT_LISTENERS = 1024,
    _SG_DEFAULT_WGPU_BINDGROUP_CACHE_SIZE = 1024,
};

// fixed-size string
typedef struct {
    char buf[_SG_STRING_SIZE];
} _sg_str_t;

// helper macros
#define _sg_def(val, def) (((val) == 0) ? (def) : (val))
#define _sg_def_flt(val, def) (((val) == 0.0f) ? (def) : (val))
#define _sg_min(a,b) (((a)<(b))?(a):(b))
#define _sg_max(a,b) (((a)>(b))?(a):(b))
#define _sg_clamp(v,v0,v1) (((v)<(v0))?(v0):(((v)>(v1))?(v1):(v)))
#define _sg_fequal(val,cmp,delta) ((((val)-(cmp))> -(delta))&&(((val)-(cmp))<(delta)))
#define _sg_ispow2(val) ((val&(val-1))==0)
#define _sg_stats_add(key,val) {if(_sg.stats_enabled){ _sg.stats.key+=val;}}

_SOKOL_PRIVATE void* _sg_malloc_clear(size_t size);
_SOKOL_PRIVATE void _sg_free(void* ptr);
_SOKOL_PRIVATE void _sg_clear(void* ptr, size_t size);

typedef struct {
    int size;
    int append_pos;
    bool append_overflow;
    uint32_t update_frame_index;
    uint32_t append_frame_index;
    int num_slots;
    int active_slot;
    sg_buffer_type type;
    sg_usage usage;
} _sg_buffer_common_t;

_SOKOL_PRIVATE void _sg_buffer_common_init(_sg_buffer_common_t* cmn, const sg_buffer_desc* desc) {
    cmn->size = (int)desc->size;
    cmn->append_pos = 0;
    cmn->append_overflow = false;
    cmn->update_frame_index = 0;
    cmn->append_frame_index = 0;
    cmn->num_slots = (desc->usage == SG_USAGE_IMMUTABLE) ? 1 : SG_NUM_INFLIGHT_FRAMES;
    cmn->active_slot = 0;
    cmn->type = desc->type;
    cmn->usage = desc->usage;
}

typedef struct {
    uint32_t upd_frame_index;
    int num_slots;
    int active_slot;
    sg_image_type type;
    bool render_target;
    int width;
    int height;
    int num_slices;
    int num_mipmaps;
    sg_usage usage;
    sg_pixel_format pixel_format;
    int sample_count;
} _sg_image_common_t;

_SOKOL_PRIVATE void _sg_image_common_init(_sg_image_common_t* cmn, const sg_image_desc* desc) {
    cmn->upd_frame_index = 0;
    cmn->num_slots = (desc->usage == SG_USAGE_IMMUTABLE) ? 1 : SG_NUM_INFLIGHT_FRAMES;
    cmn->active_slot = 0;
    cmn->type = desc->type;
    cmn->render_target = desc->render_target;
    cmn->width = desc->width;
    cmn->height = desc->height;
    cmn->num_slices = desc->num_slices;
    cmn->num_mipmaps = desc->num_mipmaps;
    cmn->usage = desc->usage;
    cmn->pixel_format = desc->pixel_format;
    cmn->sample_count = desc->sample_count;
}

typedef struct {
    sg_filter min_filter;
    sg_filter mag_filter;
    sg_filter mipmap_filter;
    sg_wrap wrap_u;
    sg_wrap wrap_v;
    sg_wrap wrap_w;
    float min_lod;
    float max_lod;
    sg_border_color border_color;
    sg_compare_func compare;
    uint32_t max_anisotropy;
} _sg_sampler_common_t;

_SOKOL_PRIVATE void _sg_sampler_common_init(_sg_sampler_common_t* cmn, const sg_sampler_desc* desc) {
    cmn->min_filter = desc->min_filter;
    cmn->mag_filter = desc->mag_filter;
    cmn->mipmap_filter = desc->mipmap_filter;
    cmn->wrap_u = desc->wrap_u;
    cmn->wrap_v = desc->wrap_v;
    cmn->wrap_w = desc->wrap_w;
    cmn->min_lod = desc->min_lod;
    cmn->max_lod = desc->max_lod;
    cmn->border_color = desc->border_color;
    cmn->compare = desc->compare;
    cmn->max_anisotropy = desc->max_anisotropy;
}

typedef struct {
    size_t size;
} _sg_shader_uniform_block_t;

typedef struct {
    bool used;
    bool readonly;
} _sg_shader_storage_buffer_t;

typedef struct {
    sg_image_type image_type;
    sg_image_sample_type sample_type;
    bool multisampled;
} _sg_shader_image_t;

typedef struct {
    sg_sampler_type sampler_type;
} _sg_shader_sampler_t;

// combined image sampler mappings, only needed on GL
typedef struct {
    int image_slot;
    int sampler_slot;
} _sg_shader_image_sampler_t;

typedef struct {
    int num_uniform_blocks;
    int num_storage_buffers;
    int num_images;
    int num_samplers;
    int num_image_samplers;
    _sg_shader_uniform_block_t uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    _sg_shader_storage_buffer_t storage_buffers[SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
    _sg_shader_image_t images[SG_MAX_SHADERSTAGE_IMAGES];
    _sg_shader_sampler_t samplers[SG_MAX_SHADERSTAGE_SAMPLERS];
    _sg_shader_image_sampler_t image_samplers[SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS];
} _sg_shader_stage_t;

typedef struct {
    _sg_shader_stage_t stage[SG_NUM_SHADER_STAGES];
} _sg_shader_common_t;

_SOKOL_PRIVATE void _sg_shader_common_init(_sg_shader_common_t* cmn, const sg_shader_desc* desc) {
    for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
        const sg_shader_stage_desc* stage_desc = (stage_index == SG_SHADERSTAGE_VS) ? &desc->vs : &desc->fs;
        _sg_shader_stage_t* stage = &cmn->stage[stage_index];
        SOKOL_ASSERT(stage->num_uniform_blocks == 0);
        for (int ub_index = 0; ub_index < SG_MAX_SHADERSTAGE_UBS; ub_index++) {
            const sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
            if (0 == ub_desc->size) {
                break;
            }
            stage->uniform_blocks[ub_index].size = ub_desc->size;
            stage->num_uniform_blocks++;
        }
        SOKOL_ASSERT(stage->num_images == 0);
        for (int img_index = 0; img_index < SG_MAX_SHADERSTAGE_IMAGES; img_index++) {
            const sg_shader_image_desc* img_desc = &stage_desc->images[img_index];
            if (!img_desc->used) {
                break;
            }
            stage->images[img_index].multisampled = img_desc->multisampled;
            stage->images[img_index].image_type = img_desc->image_type;
            stage->images[img_index].sample_type = img_desc->sample_type;
            stage->num_images++;
        }
        SOKOL_ASSERT(stage->num_samplers == 0);
        for (int smp_index = 0; smp_index < SG_MAX_SHADERSTAGE_SAMPLERS; smp_index++) {
            const sg_shader_sampler_desc* smp_desc = &stage_desc->samplers[smp_index];
            if (!smp_desc->used) {
                break;
            }
            stage->samplers[smp_index].sampler_type = smp_desc->sampler_type;
            stage->num_samplers++;
        }
        SOKOL_ASSERT(stage->num_image_samplers == 0);
        for (int img_smp_index = 0; img_smp_index < SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS; img_smp_index++) {
            const sg_shader_image_sampler_pair_desc* img_smp_desc = &stage_desc->image_sampler_pairs[img_smp_index];
            if (!img_smp_desc->used) {
                break;
            }
            SOKOL_ASSERT((img_smp_desc->image_slot >= 0) && (img_smp_desc->image_slot < stage->num_images));
            stage->image_samplers[img_smp_index].image_slot = img_smp_desc->image_slot;
            SOKOL_ASSERT((img_smp_desc->sampler_slot >= 0) && (img_smp_desc->sampler_slot < stage->num_samplers));
            stage->image_samplers[img_smp_index].sampler_slot = img_smp_desc->sampler_slot;
            stage->num_image_samplers++;
        }
        SOKOL_ASSERT(stage->num_storage_buffers == 0);
        for (int sbuf_index = 0; sbuf_index < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; sbuf_index++) {
            const sg_shader_storage_buffer_desc* sbuf_desc = &stage_desc->storage_buffers[sbuf_index];
            if (!sbuf_desc->used) {
                break;
            }
            stage->storage_buffers[sbuf_index].used = sbuf_desc->used;
            stage->storage_buffers[sbuf_index].readonly = sbuf_desc->readonly;
            stage->num_storage_buffers++;
        }
    }
}

typedef struct {
    bool vertex_buffer_layout_active[SG_MAX_VERTEX_BUFFERS];
    bool use_instanced_draw;
    sg_shader shader_id;
    sg_vertex_layout_state layout;
    sg_depth_state depth;
    sg_stencil_state stencil;
    int color_count;
    sg_color_target_state colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_primitive_type primitive_type;
    sg_index_type index_type;
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    int sample_count;
    sg_color blend_color;
    bool alpha_to_coverage_enabled;
} _sg_pipeline_common_t;

_SOKOL_PRIVATE void _sg_pipeline_common_init(_sg_pipeline_common_t* cmn, const sg_pipeline_desc* desc) {
    SOKOL_ASSERT((desc->color_count >= 0) && (desc->color_count <= SG_MAX_COLOR_ATTACHMENTS));
    for (int i = 0; i < SG_MAX_VERTEX_BUFFERS; i++) {
        cmn->vertex_buffer_layout_active[i] = false;
    }
    cmn->use_instanced_draw = false;
    cmn->shader_id = desc->shader;
    cmn->layout = desc->layout;
    cmn->depth = desc->depth;
    cmn->stencil = desc->stencil;
    cmn->color_count = desc->color_count;
    for (int i = 0; i < desc->color_count; i++) {
        cmn->colors[i] = desc->colors[i];
    }
    cmn->primitive_type = desc->primitive_type;
    cmn->index_type = desc->index_type;
    cmn->cull_mode = desc->cull_mode;
    cmn->face_winding = desc->face_winding;
    cmn->sample_count = desc->sample_count;
    cmn->blend_color = desc->blend_color;
    cmn->alpha_to_coverage_enabled = desc->alpha_to_coverage_enabled;
}

typedef struct {
    sg_image image_id;
    int mip_level;
    int slice;
} _sg_attachment_common_t;

typedef struct {
    int width;
    int height;
    int num_colors;
    _sg_attachment_common_t colors[SG_MAX_COLOR_ATTACHMENTS];
    _sg_attachment_common_t resolves[SG_MAX_COLOR_ATTACHMENTS];
    _sg_attachment_common_t depth_stencil;
} _sg_attachments_common_t;

_SOKOL_PRIVATE void _sg_attachment_common_init(_sg_attachment_common_t* cmn, const sg_attachment_desc* desc) {
    cmn->image_id = desc->image;
    cmn->mip_level = desc->mip_level;
    cmn->slice = desc->slice;
}

_SOKOL_PRIVATE void _sg_attachments_common_init(_sg_attachments_common_t* cmn, const sg_attachments_desc* desc, int width, int height) {
    SOKOL_ASSERT((width > 0) && (height > 0));
    cmn->width = width;
    cmn->height = height;
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        if (desc->colors[i].image.id != SG_INVALID_ID) {
            cmn->num_colors++;
            _sg_attachment_common_init(&cmn->colors[i], &desc->colors[i]);
            _sg_attachment_common_init(&cmn->resolves[i], &desc->resolves[i]);
        }
    }
    if (desc->depth_stencil.image.id != SG_INVALID_ID) {
        _sg_attachment_common_init(&cmn->depth_stencil, &desc->depth_stencil);
    }
}

#if defined(SOKOL_DUMMY_BACKEND)
typedef struct {
    _sg_slot_t slot;
    _sg_buffer_common_t cmn;
} _sg_dummy_buffer_t;
typedef _sg_dummy_buffer_t _sg_buffer_t;

typedef struct {
    _sg_slot_t slot;
    _sg_image_common_t cmn;
} _sg_dummy_image_t;
typedef _sg_dummy_image_t _sg_image_t;

typedef struct {
    _sg_slot_t slot;
    _sg_sampler_common_t cmn;
} _sg_dummy_sampler_t;
typedef _sg_dummy_sampler_t _sg_sampler_t;

typedef struct {
    _sg_slot_t slot;
    _sg_shader_common_t cmn;
} _sg_dummy_shader_t;
typedef _sg_dummy_shader_t _sg_shader_t;

typedef struct {
    _sg_slot_t slot;
    _sg_shader_t* shader;
    _sg_pipeline_common_t cmn;
} _sg_dummy_pipeline_t;
typedef _sg_dummy_pipeline_t _sg_pipeline_t;

typedef struct {
    _sg_image_t* image;
} _sg_dummy_attachment_t;

typedef struct {
    _sg_slot_t slot;
    _sg_attachments_common_t cmn;
    struct {
        _sg_dummy_attachment_t colors[SG_MAX_COLOR_ATTACHMENTS];
        _sg_dummy_attachment_t resolves[SG_MAX_COLOR_ATTACHMENTS];
        _sg_dummy_attachment_t depth_stencil;
    } dmy;
} _sg_dummy_attachments_t;
typedef _sg_dummy_attachments_t _sg_attachments_t;

#elif defined(_SOKOL_ANY_GL)

#define _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE (SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS * SG_NUM_SHADER_STAGES)
#define _SG_GL_STORAGEBUFFER_STAGE_INDEX_PITCH (SG_MAX_SHADERSTAGE_STORAGEBUFFERS)

typedef struct {
    _sg_slot_t slot;
    _sg_buffer_common_t cmn;
    struct {
        GLuint buf[SG_NUM_INFLIGHT_FRAMES];
        bool injected;  // if true, external buffers were injected with sg_buffer_desc.gl_buffers
    } gl;
} _sg_gl_buffer_t;
typedef _sg_gl_buffer_t _sg_buffer_t;

typedef struct {
    _sg_slot_t slot;
    _sg_image_common_t cmn;
    struct {
        GLenum target;
        GLuint msaa_render_buffer;
        GLuint tex[SG_NUM_INFLIGHT_FRAMES];
        bool injected;  // if true, external textures were injected with sg_image_desc.gl_textures
    } gl;
} _sg_gl_image_t;
typedef _sg_gl_image_t _sg_image_t;

typedef struct {
    _sg_slot_t slot;
    _sg_sampler_common_t cmn;
    struct {
        GLuint smp;
        bool injected;  // true if external sampler was injects in sg_sampler_desc.gl_sampler
    } gl;
} _sg_gl_sampler_t;
typedef _sg_gl_sampler_t _sg_sampler_t;

typedef struct {
    GLint gl_loc;
    sg_uniform_type type;
    uint16_t count;
    uint16_t offset;
} _sg_gl_uniform_t;

typedef struct {
    int num_uniforms;
    _sg_gl_uniform_t uniforms[SG_MAX_UB_MEMBERS];
} _sg_gl_uniform_block_t;

typedef struct {
    int gl_tex_slot;
} _sg_gl_shader_image_sampler_t;

typedef struct {
    _sg_str_t name;
} _sg_gl_shader_attr_t;

typedef struct {
    _sg_gl_uniform_block_t uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    _sg_gl_shader_image_sampler_t image_samplers[SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS];
} _sg_gl_shader_stage_t;

typedef struct {
    _sg_slot_t slot;
    _sg_shader_common_t cmn;
    struct {
        GLuint prog;
        _sg_gl_shader_attr_t attrs[SG_MAX_VERTEX_ATTRIBUTES];
        _sg_gl_shader_stage_t stage[SG_NUM_SHADER_STAGES];
    } gl;
} _sg_gl_shader_t;
typedef _sg_gl_shader_t _sg_shader_t;

typedef struct {
    int8_t vb_index;        // -1 if attr is not enabled
    int8_t divisor;         // -1 if not initialized
    uint8_t stride;
    uint8_t size;
    uint8_t normalized;
    int offset;
    GLenum type;
} _sg_gl_attr_t;

typedef struct {
    _sg_slot_t slot;
    _sg_pipeline_common_t cmn;
    _sg_shader_t* shader;
    struct {
        _sg_gl_attr_t attrs[SG_MAX_VERTEX_ATTRIBUTES];
        sg_depth_state depth;
        sg_stencil_state stencil;
        sg_primitive_type primitive_type;
        sg_blend_state blend;
        sg_color_mask color_write_mask[SG_MAX_COLOR_ATTACHMENTS];
        sg_cull_mode cull_mode;
        sg_face_winding face_winding;
        int sample_count;
        bool alpha_to_coverage_enabled;
    } gl;
} _sg_gl_pipeline_t;
typedef _sg_gl_pipeline_t _sg_pipeline_t;

typedef struct {
    _sg_image_t* image;
} _sg_gl_attachment_t;

typedef struct {
    _sg_slot_t slot;
    _sg_attachments_common_t cmn;
    struct {
        GLuint fb;
        _sg_gl_attachment_t colors[SG_MAX_COLOR_ATTACHMENTS];
        _sg_gl_attachment_t resolves[SG_MAX_COLOR_ATTACHMENTS];
        _sg_gl_attachment_t depth_stencil;
        GLuint msaa_resolve_framebuffer[SG_MAX_COLOR_ATTACHMENTS];
    } gl;
} _sg_gl_attachments_t;
typedef _sg_gl_attachments_t _sg_attachments_t;

typedef struct {
    _sg_gl_attr_t gl_attr;
    GLuint gl_vbuf;
} _sg_gl_cache_attr_t;

typedef struct {
    GLenum target;
    GLuint texture;
    GLuint sampler;
} _sg_gl_cache_texture_sampler_bind_slot;

typedef struct {
    sg_depth_state depth;
    sg_stencil_state stencil;
    sg_blend_state blend;
    sg_color_mask color_write_mask[SG_MAX_COLOR_ATTACHMENTS];
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    bool polygon_offset_enabled;
    int sample_count;
    sg_color blend_color;
    bool alpha_to_coverage_enabled;
    _sg_gl_cache_attr_t attrs[SG_MAX_VERTEX_ATTRIBUTES];
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint storage_buffer;  // general bind point
    GLuint stage_storage_buffers[SG_NUM_SHADER_STAGES][SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
    GLuint stored_vertex_buffer;
    GLuint stored_index_buffer;
    GLuint stored_storage_buffer;
    GLuint prog;
    _sg_gl_cache_texture_sampler_bind_slot texture_samplers[_SG_GL_TEXTURE_SAMPLER_CACHE_SIZE];
    _sg_gl_cache_texture_sampler_bind_slot stored_texture_sampler;
    int cur_ib_offset;
    GLenum cur_primitive_type;
    GLenum cur_index_type;
    GLenum cur_active_texture;
    _sg_pipeline_t* cur_pipeline;
    sg_pipeline cur_pipeline_id;
} _sg_gl_state_cache_t;

typedef struct {
    bool valid;
    GLuint vao;
    _sg_gl_state_cache_t cache;
    bool ext_anisotropic;
    GLint max_anisotropy;
    sg_store_action color_store_actions[SG_MAX_COLOR_ATTACHMENTS];
    sg_store_action depth_store_action;
    sg_store_action stencil_store_action;
    #if _SOKOL_USE_WIN32_GL_LOADER
    HINSTANCE opengl32_dll;
    #endif
} _sg_gl_backend_t;

#endif

// POOL STRUCTS

// this *MUST* remain 0
#define _SG_INVALID_SLOT_INDEX (0)

typedef struct {
    _sg_pool_t buffer_pool;
    _sg_pool_t image_pool;
    _sg_pool_t sampler_pool;
    _sg_pool_t shader_pool;
    _sg_pool_t pipeline_pool;
    _sg_pool_t attachments_pool;
    _sg_buffer_t* buffers;
    _sg_image_t* images;
    _sg_sampler_t* samplers;
    _sg_shader_t* shaders;
    _sg_pipeline_t* pipelines;
    _sg_attachments_t* attachments;
} _sg_pools_t;

typedef struct {
    int num;        // number of allocated commit listener items
    int upper;      // the current upper index (no valid items past this point)
    sg_commit_listener* items;
} _sg_commit_listeners_t;

// resolved resource bindings struct
typedef struct {
    _sg_pipeline_t* pip;
    int num_vbs;
    int num_vs_imgs;
    int num_vs_smps;
    int num_vs_sbufs;
    int num_fs_imgs;
    int num_fs_smps;
    int num_fs_sbufs;
    int vb_offsets[SG_MAX_VERTEX_BUFFERS];
    int ib_offset;
    _sg_buffer_t* vbs[SG_MAX_VERTEX_BUFFERS];
    _sg_buffer_t* ib;
    _sg_image_t* vs_imgs[SG_MAX_SHADERSTAGE_IMAGES];
    _sg_sampler_t* vs_smps[SG_MAX_SHADERSTAGE_SAMPLERS];
    _sg_buffer_t* vs_sbufs[SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
    _sg_image_t* fs_imgs[SG_MAX_SHADERSTAGE_IMAGES];
    _sg_sampler_t* fs_smps[SG_MAX_SHADERSTAGE_SAMPLERS];
    _sg_buffer_t* fs_sbufs[SG_MAX_SHADERSTAGE_STORAGEBUFFERS];
} _sg_bindings_t;

typedef struct {
    bool sample;
    bool filter;
    bool render;
    bool blend;
    bool msaa;
    bool depth;
} _sg_pixelformat_info_t;

typedef struct {
    bool valid;
    sg_desc desc;       // original desc with default values patched in
    uint32_t frame_index;
    struct {
        bool valid;
        bool in_pass;
        sg_attachments atts_id;     // SG_INVALID_ID in a swapchain pass
        _sg_attachments_t* atts;    // 0 in a swapchain pass
        int width;
        int height;
        struct {
            sg_pixel_format color_fmt;
            sg_pixel_format depth_fmt;
            int sample_count;
        } swapchain;
    } cur_pass;
    sg_pipeline cur_pipeline;
    bool next_draw_valid;
    #if defined(SOKOL_DEBUG)
    sg_log_item validate_error;
    #endif
    _sg_pools_t pools;
    sg_backend backend;
    sg_features features;
    sg_limits limits;
    _sg_pixelformat_info_t formats[_SG_PIXELFORMAT_NUM];
    bool stats_enabled;
    sg_frame_stats stats;
    sg_frame_stats prev_stats;
    _sg_gl_backend_t gl;
    #if defined(SOKOL_TRACE_HOOKS)
    sg_trace_hooks hooks;
    #endif
    _sg_commit_listeners_t commit_listeners;
} _sg_state_t;
static _sg_state_t _sg;

// ██       ██████   ██████   ██████  ██ ███    ██  ██████
// ██      ██    ██ ██       ██       ██ ████   ██ ██
// ██      ██    ██ ██   ███ ██   ███ ██ ██ ██  ██ ██   ███
// ██      ██    ██ ██    ██ ██    ██ ██ ██  ██ ██ ██    ██
// ███████  ██████   ██████   ██████  ██ ██   ████  ██████
//
// >>logging
#if defined(SOKOL_DEBUG)
#define _SG_LOGITEM_XMACRO(item,msg) #item ": " msg,
static const char* _sg_log_messages[] = {
    _SG_LOG_ITEMS
};
#undef _SG_LOGITEM_XMACRO
#endif // SOKOL_DEBUG

#define _SG_PANIC(code) _sg_log(SG_LOGITEM_ ##code, 0, 0, __LINE__)
#define _SG_ERROR(code) _sg_log(SG_LOGITEM_ ##code, 1, 0, __LINE__)
#define _SG_WARN(code) _sg_log(SG_LOGITEM_ ##code, 2, 0, __LINE__)
#define _SG_INFO(code) _sg_log(SG_LOGITEM_ ##code, 3, 0, __LINE__)
#define _SG_LOGMSG(code,msg) _sg_log(SG_LOGITEM_ ##code, 3, msg, __LINE__)
#define _SG_VALIDATE(cond,code) if (!(cond)){ _sg.validate_error = SG_LOGITEM_ ##code; _sg_log(SG_LOGITEM_ ##code, 1, 0, __LINE__); }

static void _sg_log(sg_log_item log_item, uint32_t log_level, const char* msg, uint32_t line_nr) {
    if (_sg.desc.logger.func) {
        const char* filename = 0;
        #if defined(SOKOL_DEBUG)
            filename = __FILE__;
            if (0 == msg) {
                msg = _sg_log_messages[log_item];
            }
        #endif
        _sg.desc.logger.func("sg", log_level, log_item, msg, line_nr, filename, _sg.desc.logger.user_data);
    } else {
        // for log level PANIC it would be 'undefined behaviour' to continue
        if (log_level == 0) {
            abort();
        }
    }
}

// ███    ███ ███████ ███    ███  ██████  ██████  ██    ██
// ████  ████ ██      ████  ████ ██    ██ ██   ██  ██  ██
// ██ ████ ██ █████   ██ ████ ██ ██    ██ ██████    ████
// ██  ██  ██ ██      ██  ██  ██ ██    ██ ██   ██    ██
// ██      ██ ███████ ██      ██  ██████  ██   ██    ██
//
// >>memory

// a helper macro to clear a struct with potentially ARC'ed ObjC references
#if defined(SOKOL_METAL)
    #if defined(__cplusplus)
        #define _SG_CLEAR_ARC_STRUCT(type, item) { item = type(); }
    #else
        #define _SG_CLEAR_ARC_STRUCT(type, item) { item = (type) { 0 }; }
    #endif
#else
    #define _SG_CLEAR_ARC_STRUCT(type, item) { _sg_clear(&item, sizeof(item)); }
#endif

_SOKOL_PRIVATE void _sg_clear(void* ptr, size_t size) {
    SOKOL_ASSERT(ptr && (size > 0));
    memset(ptr, 0, size);
}

_SOKOL_PRIVATE void* _sg_malloc(size_t size) {
    SOKOL_ASSERT(size > 0);
    void* ptr;
    if (_sg.desc.allocator.alloc_fn) {
        ptr = _sg.desc.allocator.alloc_fn(size, _sg.desc.allocator.user_data);
    } else {
        ptr = malloc(size);
    }
    if (0 == ptr) {
        _SG_PANIC(MALLOC_FAILED);
    }
    return ptr;
}

_SOKOL_PRIVATE void* _sg_malloc_clear(size_t size) {
    void* ptr = _sg_malloc(size);
    _sg_clear(ptr, size);
    return ptr;
}

_SOKOL_PRIVATE void _sg_free(void* ptr) {
    if (_sg.desc.allocator.free_fn) {
        _sg.desc.allocator.free_fn(ptr, _sg.desc.allocator.user_data);
    } else {
        free(ptr);
    }
}

_SOKOL_PRIVATE bool _sg_strempty(const _sg_str_t* str) {
    return 0 == str->buf[0];
}

_SOKOL_PRIVATE const char* _sg_strptr(const _sg_str_t* str) {
    return &str->buf[0];
}

_SOKOL_PRIVATE void _sg_strcpy(_sg_str_t* dst, const char* src) {
    SOKOL_ASSERT(dst);
    if (src) {
        #if defined(_MSC_VER)
        strncpy_s(dst->buf, _SG_STRING_SIZE, src, (_SG_STRING_SIZE-1));
        #else
        strncpy(dst->buf, src, _SG_STRING_SIZE);
        #endif
        dst->buf[_SG_STRING_SIZE-1] = 0;
    } else {
        _sg_clear(dst->buf, _SG_STRING_SIZE);
    }
}

// ██   ██ ███████ ██      ██████  ███████ ██████  ███████
// ██   ██ ██      ██      ██   ██ ██      ██   ██ ██
// ███████ █████   ██      ██████  █████   ██████  ███████
// ██   ██ ██      ██      ██      ██      ██   ██      ██
// ██   ██ ███████ ███████ ██      ███████ ██   ██ ███████
//
// >>helpers
_SOKOL_PRIVATE uint32_t _sg_align_u32(uint32_t val, uint32_t align) {
    SOKOL_ASSERT((align > 0) && ((align & (align - 1)) == 0));
    return (val + (align - 1)) & ~(align - 1);
}

typedef struct { int x, y, w, h; } _sg_recti_t;

_SOKOL_PRIVATE _sg_recti_t _sg_clipi(int x, int y, int w, int h, int clip_width, int clip_height) {
    x = _sg_min(_sg_max(0, x), clip_width-1);
    y = _sg_min(_sg_max(0, y), clip_height-1);
    if ((x + w) > clip_width) {
        w = clip_width - x;
    }
    if ((y + h) > clip_height) {
        h = clip_height - y;
    }
    w = _sg_max(w, 1);
    h = _sg_max(h, 1);
    const _sg_recti_t res = { x, y, w, h };
    return res;
}

_SOKOL_PRIVATE int _sg_vertexformat_bytesize(sg_vertex_format fmt) {
    switch (fmt) {
        case SG_VERTEXFORMAT_FLOAT:     return 4;
        case SG_VERTEXFORMAT_FLOAT2:    return 8;
        case SG_VERTEXFORMAT_FLOAT3:    return 12;
        case SG_VERTEXFORMAT_FLOAT4:    return 16;
        case SG_VERTEXFORMAT_BYTE4:     return 4;
        case SG_VERTEXFORMAT_BYTE4N:    return 4;
        case SG_VERTEXFORMAT_UBYTE4:    return 4;
        case SG_VERTEXFORMAT_UBYTE4N:   return 4;
        case SG_VERTEXFORMAT_SHORT2:    return 4;
        case SG_VERTEXFORMAT_SHORT2N:   return 4;
        case SG_VERTEXFORMAT_USHORT2N:  return 4;
        case SG_VERTEXFORMAT_SHORT4:    return 8;
        case SG_VERTEXFORMAT_SHORT4N:   return 8;
        case SG_VERTEXFORMAT_USHORT4N:  return 8;
        case SG_VERTEXFORMAT_UINT10_N2: return 4;
        case SG_VERTEXFORMAT_HALF2:     return 4;
        case SG_VERTEXFORMAT_HALF4:     return 8;
        case SG_VERTEXFORMAT_UINT:      return 4;
        case SG_VERTEXFORMAT_INVALID:   return 0;
        default:
            SOKOL_UNREACHABLE;
            return -1;
    }
}

_SOKOL_PRIVATE uint32_t _sg_uniform_alignment(sg_uniform_type type, int array_count, sg_uniform_layout ub_layout) {
    if (ub_layout == SG_UNIFORMLAYOUT_NATIVE) {
        return 1;
    } else {
        SOKOL_ASSERT(array_count > 0);
        if (array_count == 1) {
            switch (type) {
                case SG_UNIFORMTYPE_FLOAT:
                case SG_UNIFORMTYPE_INT:
                    return 4;
                case SG_UNIFORMTYPE_FLOAT2:
                case SG_UNIFORMTYPE_INT2:
                    return 8;
                case SG_UNIFORMTYPE_FLOAT3:
                case SG_UNIFORMTYPE_FLOAT4:
                case SG_UNIFORMTYPE_INT3:
                case SG_UNIFORMTYPE_INT4:
                    return 16;
                case SG_UNIFORMTYPE_MAT4:
                    return 16;
                default:
                    SOKOL_UNREACHABLE;
                    return 1;
            }
        } else {
            return 16;
        }
    }
}

_SOKOL_PRIVATE uint32_t _sg_uniform_size(sg_uniform_type type, int array_count, sg_uniform_layout ub_layout) {
    SOKOL_ASSERT(array_count > 0);
    if (array_count == 1) {
        switch (type) {
            case SG_UNIFORMTYPE_FLOAT:
            case SG_UNIFORMTYPE_INT:
                return 4;
            case SG_UNIFORMTYPE_FLOAT2:
            case SG_UNIFORMTYPE_INT2:
                return 8;
            case SG_UNIFORMTYPE_FLOAT3:
            case SG_UNIFORMTYPE_INT3:
                return 12;
            case SG_UNIFORMTYPE_FLOAT4:
            case SG_UNIFORMTYPE_INT4:
                return 16;
            case SG_UNIFORMTYPE_MAT4:
                return 64;
            default:
                SOKOL_UNREACHABLE;
                return 0;
        }
    } else {
        if (ub_layout == SG_UNIFORMLAYOUT_NATIVE) {
            switch (type) {
                case SG_UNIFORMTYPE_FLOAT:
                case SG_UNIFORMTYPE_INT:
                    return 4 * (uint32_t)array_count;
                case SG_UNIFORMTYPE_FLOAT2:
                case SG_UNIFORMTYPE_INT2:
                    return 8 * (uint32_t)array_count;
                case SG_UNIFORMTYPE_FLOAT3:
                case SG_UNIFORMTYPE_INT3:
                    return 12 * (uint32_t)array_count;
                case SG_UNIFORMTYPE_FLOAT4:
                case SG_UNIFORMTYPE_INT4:
                    return 16 * (uint32_t)array_count;
                case SG_UNIFORMTYPE_MAT4:
                    return 64 * (uint32_t)array_count;
                default:
                    SOKOL_UNREACHABLE;
                    return 0;
            }
        } else {
            switch (type) {
                case SG_UNIFORMTYPE_FLOAT:
                case SG_UNIFORMTYPE_FLOAT2:
                case SG_UNIFORMTYPE_FLOAT3:
                case SG_UNIFORMTYPE_FLOAT4:
                case SG_UNIFORMTYPE_INT:
                case SG_UNIFORMTYPE_INT2:
                case SG_UNIFORMTYPE_INT3:
                case SG_UNIFORMTYPE_INT4:
                    return 16 * (uint32_t)array_count;
                case SG_UNIFORMTYPE_MAT4:
                    return 64 * (uint32_t)array_count;
                default:
                    SOKOL_UNREACHABLE;
                    return 0;
            }
        }
    }
}

_SOKOL_PRIVATE bool _sg_is_compressed_pixel_format(sg_pixel_format fmt) {
    switch (fmt) {
        case SG_PIXELFORMAT_BC1_RGBA:
        case SG_PIXELFORMAT_BC2_RGBA:
        case SG_PIXELFORMAT_BC3_RGBA:
        case SG_PIXELFORMAT_BC3_SRGBA:
        case SG_PIXELFORMAT_BC4_R:
        case SG_PIXELFORMAT_BC4_RSN:
        case SG_PIXELFORMAT_BC5_RG:
        case SG_PIXELFORMAT_BC5_RGSN:
        case SG_PIXELFORMAT_BC6H_RGBF:
        case SG_PIXELFORMAT_BC6H_RGBUF:
        case SG_PIXELFORMAT_BC7_RGBA:
        case SG_PIXELFORMAT_BC7_SRGBA:
        case SG_PIXELFORMAT_PVRTC_RGB_2BPP:
        case SG_PIXELFORMAT_PVRTC_RGB_4BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_2BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_4BPP:
        case SG_PIXELFORMAT_ETC2_RGB8:
        case SG_PIXELFORMAT_ETC2_SRGB8:
        case SG_PIXELFORMAT_ETC2_RGB8A1:
        case SG_PIXELFORMAT_ETC2_RGBA8:
        case SG_PIXELFORMAT_ETC2_SRGB8A8:
        case SG_PIXELFORMAT_EAC_R11:
        case SG_PIXELFORMAT_EAC_R11SN:
        case SG_PIXELFORMAT_EAC_RG11:
        case SG_PIXELFORMAT_EAC_RG11SN:
        case SG_PIXELFORMAT_ASTC_4x4_RGBA:
        case SG_PIXELFORMAT_ASTC_4x4_SRGBA:
            return true;
        default:
            return false;
    }
}

_SOKOL_PRIVATE bool _sg_is_valid_rendertarget_color_format(sg_pixel_format fmt) {
    const int fmt_index = (int) fmt;
    SOKOL_ASSERT((fmt_index >= 0) && (fmt_index < _SG_PIXELFORMAT_NUM));
    return _sg.formats[fmt_index].render && !_sg.formats[fmt_index].depth;
}

_SOKOL_PRIVATE bool _sg_is_valid_rendertarget_depth_format(sg_pixel_format fmt) {
    const int fmt_index = (int) fmt;
    SOKOL_ASSERT((fmt_index >= 0) && (fmt_index < _SG_PIXELFORMAT_NUM));
    return _sg.formats[fmt_index].render && _sg.formats[fmt_index].depth;
}

_SOKOL_PRIVATE bool _sg_is_depth_or_depth_stencil_format(sg_pixel_format fmt) {
    return (SG_PIXELFORMAT_DEPTH == fmt) || (SG_PIXELFORMAT_DEPTH_STENCIL == fmt);
}

_SOKOL_PRIVATE bool _sg_is_depth_stencil_format(sg_pixel_format fmt) {
    return (SG_PIXELFORMAT_DEPTH_STENCIL == fmt);
}

_SOKOL_PRIVATE int _sg_pixelformat_bytesize(sg_pixel_format fmt) {
    switch (fmt) {
        case SG_PIXELFORMAT_R8:
        case SG_PIXELFORMAT_R8SN:
        case SG_PIXELFORMAT_R8UI:
        case SG_PIXELFORMAT_R8SI:
            return 1;
        case SG_PIXELFORMAT_R16:
        case SG_PIXELFORMAT_R16SN:
        case SG_PIXELFORMAT_R16UI:
        case SG_PIXELFORMAT_R16SI:
        case SG_PIXELFORMAT_R16F:
        case SG_PIXELFORMAT_RG8:
        case SG_PIXELFORMAT_RG8SN:
        case SG_PIXELFORMAT_RG8UI:
        case SG_PIXELFORMAT_RG8SI:
            return 2;
        case SG_PIXELFORMAT_R32UI:
        case SG_PIXELFORMAT_R32SI:
        case SG_PIXELFORMAT_R32F:
        case SG_PIXELFORMAT_RG16:
        case SG_PIXELFORMAT_RG16SN:
        case SG_PIXELFORMAT_RG16UI:
        case SG_PIXELFORMAT_RG16SI:
        case SG_PIXELFORMAT_RG16F:
        case SG_PIXELFORMAT_RGBA8:
        case SG_PIXELFORMAT_SRGB8A8:
        case SG_PIXELFORMAT_RGBA8SN:
        case SG_PIXELFORMAT_RGBA8UI:
        case SG_PIXELFORMAT_RGBA8SI:
        case SG_PIXELFORMAT_BGRA8:
        case SG_PIXELFORMAT_RGB10A2:
        case SG_PIXELFORMAT_RG11B10F:
        case SG_PIXELFORMAT_RGB9E5:
            return 4;
        case SG_PIXELFORMAT_RG32UI:
        case SG_PIXELFORMAT_RG32SI:
        case SG_PIXELFORMAT_RG32F:
        case SG_PIXELFORMAT_RGBA16:
        case SG_PIXELFORMAT_RGBA16SN:
        case SG_PIXELFORMAT_RGBA16UI:
        case SG_PIXELFORMAT_RGBA16SI:
        case SG_PIXELFORMAT_RGBA16F:
            return 8;
        case SG_PIXELFORMAT_RGBA32UI:
        case SG_PIXELFORMAT_RGBA32SI:
        case SG_PIXELFORMAT_RGBA32F:
            return 16;
        case SG_PIXELFORMAT_DEPTH:
        case SG_PIXELFORMAT_DEPTH_STENCIL:
            return 4;
        default:
            SOKOL_UNREACHABLE;
            return 0;
    }
}

_SOKOL_PRIVATE int _sg_roundup(int val, int round_to) {
    return (val+(round_to-1)) & ~(round_to-1);
}

_SOKOL_PRIVATE uint32_t _sg_roundup_u32(uint32_t val, uint32_t round_to) {
    return (val+(round_to-1)) & ~(round_to-1);
}

_SOKOL_PRIVATE uint64_t _sg_roundup_u64(uint64_t val, uint64_t round_to) {
    return (val+(round_to-1)) & ~(round_to-1);
}

_SOKOL_PRIVATE bool _sg_multiple_u64(uint64_t val, uint64_t of) {
    return (val & (of-1)) == 0;
}

/* return row pitch for an image

    see ComputePitch in https://github.com/microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexUtil.cpp

    For the special PVRTC pitch computation, see:
    GL extension requirement (https://www.khronos.org/registry/OpenGL/extensions/IMG/IMG_texture_compression_pvrtc.txt)

    Quote:

    6) How is the imageSize argument calculated for the CompressedTexImage2D
       and CompressedTexSubImage2D functions.

       Resolution: For PVRTC 4BPP formats the imageSize is calculated as:
          ( max(width, 8) * max(height, 8) * 4 + 7) / 8
       For PVRTC 2BPP formats the imageSize is calculated as:
          ( max(width, 16) * max(height, 8) * 2 + 7) / 8
*/
_SOKOL_PRIVATE int _sg_row_pitch(sg_pixel_format fmt, int width, int row_align) {
    int pitch;
    switch (fmt) {
        case SG_PIXELFORMAT_BC1_RGBA:
        case SG_PIXELFORMAT_BC4_R:
        case SG_PIXELFORMAT_BC4_RSN:
        case SG_PIXELFORMAT_ETC2_RGB8:
        case SG_PIXELFORMAT_ETC2_SRGB8:
        case SG_PIXELFORMAT_ETC2_RGB8A1:
        case SG_PIXELFORMAT_EAC_R11:
        case SG_PIXELFORMAT_EAC_R11SN:
            pitch = ((width + 3) / 4) * 8;
            pitch = pitch < 8 ? 8 : pitch;
            break;
        case SG_PIXELFORMAT_BC2_RGBA:
        case SG_PIXELFORMAT_BC3_RGBA:
        case SG_PIXELFORMAT_BC3_SRGBA:
        case SG_PIXELFORMAT_BC5_RG:
        case SG_PIXELFORMAT_BC5_RGSN:
        case SG_PIXELFORMAT_BC6H_RGBF:
        case SG_PIXELFORMAT_BC6H_RGBUF:
        case SG_PIXELFORMAT_BC7_RGBA:
        case SG_PIXELFORMAT_BC7_SRGBA:
        case SG_PIXELFORMAT_ETC2_RGBA8:
        case SG_PIXELFORMAT_ETC2_SRGB8A8:
        case SG_PIXELFORMAT_EAC_RG11:
        case SG_PIXELFORMAT_EAC_RG11SN:
        case SG_PIXELFORMAT_ASTC_4x4_RGBA:
        case SG_PIXELFORMAT_ASTC_4x4_SRGBA:
            pitch = ((width + 3) / 4) * 16;
            pitch = pitch < 16 ? 16 : pitch;
            break;
        case SG_PIXELFORMAT_PVRTC_RGB_4BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_4BPP:
            pitch = (_sg_max(width, 8) * 4 + 7) / 8;
            break;
        case SG_PIXELFORMAT_PVRTC_RGB_2BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_2BPP:
            pitch = (_sg_max(width, 16) * 2 + 7) / 8;
            break;
        default:
            pitch = width * _sg_pixelformat_bytesize(fmt);
            break;
    }
    pitch = _sg_roundup(pitch, row_align);
    return pitch;
}

// compute the number of rows in a surface depending on pixel format
_SOKOL_PRIVATE int _sg_num_rows(sg_pixel_format fmt, int height) {
    int num_rows;
    switch (fmt) {
        case SG_PIXELFORMAT_BC1_RGBA:
        case SG_PIXELFORMAT_BC4_R:
        case SG_PIXELFORMAT_BC4_RSN:
        case SG_PIXELFORMAT_ETC2_RGB8:
        case SG_PIXELFORMAT_ETC2_SRGB8:
        case SG_PIXELFORMAT_ETC2_RGB8A1:
        case SG_PIXELFORMAT_ETC2_RGBA8:
        case SG_PIXELFORMAT_ETC2_SRGB8A8:
        case SG_PIXELFORMAT_EAC_R11:
        case SG_PIXELFORMAT_EAC_R11SN:
        case SG_PIXELFORMAT_EAC_RG11:
        case SG_PIXELFORMAT_EAC_RG11SN:
        case SG_PIXELFORMAT_BC2_RGBA:
        case SG_PIXELFORMAT_BC3_RGBA:
        case SG_PIXELFORMAT_BC3_SRGBA:
        case SG_PIXELFORMAT_BC5_RG:
        case SG_PIXELFORMAT_BC5_RGSN:
        case SG_PIXELFORMAT_BC6H_RGBF:
        case SG_PIXELFORMAT_BC6H_RGBUF:
        case SG_PIXELFORMAT_BC7_RGBA:
        case SG_PIXELFORMAT_BC7_SRGBA:
        case SG_PIXELFORMAT_ASTC_4x4_RGBA:
        case SG_PIXELFORMAT_ASTC_4x4_SRGBA:
            num_rows = ((height + 3) / 4);
            break;
        case SG_PIXELFORMAT_PVRTC_RGB_4BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_4BPP:
        case SG_PIXELFORMAT_PVRTC_RGB_2BPP:
        case SG_PIXELFORMAT_PVRTC_RGBA_2BPP:
            /* NOTE: this is most likely not correct because it ignores any
                PVCRTC block size, but multiplied with _sg_row_pitch()
                it gives the correct surface pitch.

                See: https://www.khronos.org/registry/OpenGL/extensions/IMG/IMG_texture_compression_pvrtc.txt
            */
            num_rows = ((_sg_max(height, 8) + 7) / 8) * 8;
            break;
        default:
            num_rows = height;
            break;
    }
    if (num_rows < 1) {
        num_rows = 1;
    }
    return num_rows;
}

// return size of a mipmap level
_SOKOL_PRIVATE int _sg_miplevel_dim(int base_dim, int mip_level) {
    return _sg_max(base_dim >> mip_level, 1);
}

/* return pitch of a 2D subimage / texture slice
    see ComputePitch in https://github.com/microsoft/DirectXTex/blob/master/DirectXTex/DirectXTexUtil.cpp
*/
_SOKOL_PRIVATE int _sg_surface_pitch(sg_pixel_format fmt, int width, int height, int row_align) {
    int num_rows = _sg_num_rows(fmt, height);
    return num_rows * _sg_row_pitch(fmt, width, row_align);
}

// capability table pixel format helper functions
_SOKOL_PRIVATE void _sg_pixelformat_all(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->filter = true;
    pfi->blend = true;
    pfi->render = true;
    pfi->msaa = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_s(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sf(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->filter = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sr(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->render = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sfr(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->filter = true;
    pfi->render = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_srmd(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->render = true;
    pfi->msaa = true;
    pfi->depth = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_srm(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->render = true;
    pfi->msaa = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sfrm(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->filter = true;
    pfi->render = true;
    pfi->msaa = true;
}
_SOKOL_PRIVATE void _sg_pixelformat_sbrm(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->blend = true;
    pfi->render = true;
    pfi->msaa = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sbr(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->blend = true;
    pfi->render = true;
}

_SOKOL_PRIVATE void _sg_pixelformat_sfbr(_sg_pixelformat_info_t* pfi) {
    pfi->sample = true;
    pfi->filter = true;
    pfi->blend = true;
    pfi->render = true;
}

_SOKOL_PRIVATE sg_pass_action _sg_pass_action_defaults(const sg_pass_action* action) {
    SOKOL_ASSERT(action);
    sg_pass_action res = *action;
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        if (res.colors[i].load_action == _SG_LOADACTION_DEFAULT) {
            res.colors[i].load_action = SG_LOADACTION_CLEAR;
            res.colors[i].clear_value.r = SG_DEFAULT_CLEAR_RED;
            res.colors[i].clear_value.g = SG_DEFAULT_CLEAR_GREEN;
            res.colors[i].clear_value.b = SG_DEFAULT_CLEAR_BLUE;
            res.colors[i].clear_value.a = SG_DEFAULT_CLEAR_ALPHA;
        }
        if (res.colors[i].store_action == _SG_STOREACTION_DEFAULT) {
            res.colors[i].store_action = SG_STOREACTION_STORE;
        }
    }
    if (res.depth.load_action == _SG_LOADACTION_DEFAULT) {
        res.depth.load_action = SG_LOADACTION_CLEAR;
        res.depth.clear_value = SG_DEFAULT_CLEAR_DEPTH;
    }
    if (res.depth.store_action == _SG_STOREACTION_DEFAULT) {
        res.depth.store_action = SG_STOREACTION_DONTCARE;
    }
    if (res.stencil.load_action == _SG_LOADACTION_DEFAULT) {
        res.stencil.load_action = SG_LOADACTION_CLEAR;
        res.stencil.clear_value = SG_DEFAULT_CLEAR_STENCIL;
    }
    if (res.stencil.store_action == _SG_STOREACTION_DEFAULT) {
        res.stencil.store_action = SG_STOREACTION_DONTCARE;
    }
    return res;
}

#if defined(_SOKOL_ANY_GL)

//-- type translation ----------------------------------------------------------
_SOKOL_PRIVATE GLenum _sg_gl_buffer_target(sg_buffer_type t) {
    switch (t) {
        case SG_BUFFERTYPE_VERTEXBUFFER:    return GL_ARRAY_BUFFER;
        case SG_BUFFERTYPE_INDEXBUFFER:     return GL_ELEMENT_ARRAY_BUFFER;
        case SG_BUFFERTYPE_STORAGEBUFFER:   return GL_SHADER_STORAGE_BUFFER;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_texture_target(sg_image_type t) {
    switch (t) {
        case SG_IMAGETYPE_2D:   return GL_TEXTURE_2D;
        case SG_IMAGETYPE_CUBE: return GL_TEXTURE_CUBE_MAP;
        case SG_IMAGETYPE_3D:       return GL_TEXTURE_3D;
        case SG_IMAGETYPE_ARRAY:    return GL_TEXTURE_2D_ARRAY;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_usage(sg_usage u) {
    switch (u) {
        case SG_USAGE_IMMUTABLE:    return GL_STATIC_DRAW;
        case SG_USAGE_DYNAMIC:      return GL_DYNAMIC_DRAW;
        case SG_USAGE_STREAM:       return GL_STREAM_DRAW;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_shader_stage(sg_shader_stage stage) {
    switch (stage) {
        case SG_SHADERSTAGE_VS:     return GL_VERTEX_SHADER;
        case SG_SHADERSTAGE_FS:     return GL_FRAGMENT_SHADER;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLint _sg_gl_vertexformat_size(sg_vertex_format fmt) {
    switch (fmt) {
        case SG_VERTEXFORMAT_FLOAT:     return 1;
        case SG_VERTEXFORMAT_FLOAT2:    return 2;
        case SG_VERTEXFORMAT_FLOAT3:    return 3;
        case SG_VERTEXFORMAT_FLOAT4:    return 4;
        case SG_VERTEXFORMAT_BYTE4:     return 4;
        case SG_VERTEXFORMAT_BYTE4N:    return 4;
        case SG_VERTEXFORMAT_UBYTE4:    return 4;
        case SG_VERTEXFORMAT_UBYTE4N:   return 4;
        case SG_VERTEXFORMAT_SHORT2:    return 2;
        case SG_VERTEXFORMAT_SHORT2N:   return 2;
        case SG_VERTEXFORMAT_USHORT2N:  return 2;
        case SG_VERTEXFORMAT_SHORT4:    return 4;
        case SG_VERTEXFORMAT_SHORT4N:   return 4;
        case SG_VERTEXFORMAT_USHORT4N:  return 4;
        case SG_VERTEXFORMAT_UINT10_N2: return 4;
        case SG_VERTEXFORMAT_HALF2:     return 2;
        case SG_VERTEXFORMAT_HALF4:     return 4;
        case SG_VERTEXFORMAT_UINT:      return 1;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_vertexformat_type(sg_vertex_format fmt) {
    switch (fmt) {
        case SG_VERTEXFORMAT_FLOAT:
        case SG_VERTEXFORMAT_FLOAT2:
        case SG_VERTEXFORMAT_FLOAT3:
        case SG_VERTEXFORMAT_FLOAT4:
            return GL_FLOAT;
        case SG_VERTEXFORMAT_BYTE4:
        case SG_VERTEXFORMAT_BYTE4N:
            return GL_BYTE;
        case SG_VERTEXFORMAT_UBYTE4:
        case SG_VERTEXFORMAT_UBYTE4N:
            return GL_UNSIGNED_BYTE;
        case SG_VERTEXFORMAT_SHORT2:
        case SG_VERTEXFORMAT_SHORT2N:
        case SG_VERTEXFORMAT_SHORT4:
        case SG_VERTEXFORMAT_SHORT4N:
            return GL_SHORT;
        case SG_VERTEXFORMAT_USHORT2N:
        case SG_VERTEXFORMAT_USHORT4N:
            return GL_UNSIGNED_SHORT;
        case SG_VERTEXFORMAT_UINT10_N2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case SG_VERTEXFORMAT_HALF2:
        case SG_VERTEXFORMAT_HALF4:
            return GL_HALF_FLOAT;
        case SG_VERTEXFORMAT_UINT:
            return GL_UNSIGNED_INT;
        default:
            SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLboolean _sg_gl_vertexformat_normalized(sg_vertex_format fmt) {
    switch (fmt) {
        case SG_VERTEXFORMAT_BYTE4N:
        case SG_VERTEXFORMAT_UBYTE4N:
        case SG_VERTEXFORMAT_SHORT2N:
        case SG_VERTEXFORMAT_USHORT2N:
        case SG_VERTEXFORMAT_SHORT4N:
        case SG_VERTEXFORMAT_USHORT4N:
        case SG_VERTEXFORMAT_UINT10_N2:
            return GL_TRUE;
        default:
            return GL_FALSE;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_primitive_type(sg_primitive_type t) {
    switch (t) {
        case SG_PRIMITIVETYPE_POINTS:           return GL_POINTS;
        case SG_PRIMITIVETYPE_LINES:            return GL_LINES;
        case SG_PRIMITIVETYPE_LINE_STRIP:       return GL_LINE_STRIP;
        case SG_PRIMITIVETYPE_TRIANGLES:        return GL_TRIANGLES;
        case SG_PRIMITIVETYPE_TRIANGLE_STRIP:   return GL_TRIANGLE_STRIP;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_index_type(sg_index_type t) {
    switch (t) {
        case SG_INDEXTYPE_NONE:     return 0;
        case SG_INDEXTYPE_UINT16:   return GL_UNSIGNED_SHORT;
        case SG_INDEXTYPE_UINT32:   return GL_UNSIGNED_INT;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_compare_func(sg_compare_func cmp) {
    switch (cmp) {
        case SG_COMPAREFUNC_NEVER:          return GL_NEVER;
        case SG_COMPAREFUNC_LESS:           return GL_LESS;
        case SG_COMPAREFUNC_EQUAL:          return GL_EQUAL;
        case SG_COMPAREFUNC_LESS_EQUAL:     return GL_LEQUAL;
        case SG_COMPAREFUNC_GREATER:        return GL_GREATER;
        case SG_COMPAREFUNC_NOT_EQUAL:      return GL_NOTEQUAL;
        case SG_COMPAREFUNC_GREATER_EQUAL:  return GL_GEQUAL;
        case SG_COMPAREFUNC_ALWAYS:         return GL_ALWAYS;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_stencil_op(sg_stencil_op op) {
    switch (op) {
        case SG_STENCILOP_KEEP:         return GL_KEEP;
        case SG_STENCILOP_ZERO:         return GL_ZERO;
        case SG_STENCILOP_REPLACE:      return GL_REPLACE;
        case SG_STENCILOP_INCR_CLAMP:   return GL_INCR;
        case SG_STENCILOP_DECR_CLAMP:   return GL_DECR;
        case SG_STENCILOP_INVERT:       return GL_INVERT;
        case SG_STENCILOP_INCR_WRAP:    return GL_INCR_WRAP;
        case SG_STENCILOP_DECR_WRAP:    return GL_DECR_WRAP;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_blend_factor(sg_blend_factor f) {
    switch (f) {
        case SG_BLENDFACTOR_ZERO:                   return GL_ZERO;
        case SG_BLENDFACTOR_ONE:                    return GL_ONE;
        case SG_BLENDFACTOR_SRC_COLOR:              return GL_SRC_COLOR;
        case SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR:    return GL_ONE_MINUS_SRC_COLOR;
        case SG_BLENDFACTOR_SRC_ALPHA:              return GL_SRC_ALPHA;
        case SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:    return GL_ONE_MINUS_SRC_ALPHA;
        case SG_BLENDFACTOR_DST_COLOR:              return GL_DST_COLOR;
        case SG_BLENDFACTOR_ONE_MINUS_DST_COLOR:    return GL_ONE_MINUS_DST_COLOR;
        case SG_BLENDFACTOR_DST_ALPHA:              return GL_DST_ALPHA;
        case SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA:    return GL_ONE_MINUS_DST_ALPHA;
        case SG_BLENDFACTOR_SRC_ALPHA_SATURATED:    return GL_SRC_ALPHA_SATURATE;
        case SG_BLENDFACTOR_BLEND_COLOR:            return GL_CONSTANT_COLOR;
        case SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR:  return GL_ONE_MINUS_CONSTANT_COLOR;
        case SG_BLENDFACTOR_BLEND_ALPHA:            return GL_CONSTANT_ALPHA;
        case SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA:  return GL_ONE_MINUS_CONSTANT_ALPHA;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_blend_op(sg_blend_op op) {
    switch (op) {
        case SG_BLENDOP_ADD:                return GL_FUNC_ADD;
        case SG_BLENDOP_SUBTRACT:           return GL_FUNC_SUBTRACT;
        case SG_BLENDOP_REVERSE_SUBTRACT:   return GL_FUNC_REVERSE_SUBTRACT;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_min_filter(sg_filter min_f, sg_filter mipmap_f) {
    if (min_f == SG_FILTER_NEAREST) {
        switch (mipmap_f) {
            case SG_FILTER_NONE:    return GL_NEAREST;
            case SG_FILTER_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
            case SG_FILTER_LINEAR:  return GL_NEAREST_MIPMAP_LINEAR;
            default: SOKOL_UNREACHABLE; return (GLenum)0;
        }
    } else if (min_f == SG_FILTER_LINEAR) {
        switch (mipmap_f) {
            case SG_FILTER_NONE:    return GL_LINEAR;
            case SG_FILTER_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
            case SG_FILTER_LINEAR:  return GL_LINEAR_MIPMAP_LINEAR;
            default: SOKOL_UNREACHABLE; return (GLenum)0;
        }
    } else {
        SOKOL_UNREACHABLE; return (GLenum)0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_mag_filter(sg_filter mag_f) {
    if (mag_f == SG_FILTER_NEAREST) {
        return GL_NEAREST;
    } else {
        return GL_LINEAR;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_wrap(sg_wrap w) {
    switch (w) {
        case SG_WRAP_CLAMP_TO_EDGE:     return GL_CLAMP_TO_EDGE;
        #if defined(SOKOL_GLCORE)
        case SG_WRAP_CLAMP_TO_BORDER:   return GL_CLAMP_TO_BORDER;
        #else
        case SG_WRAP_CLAMP_TO_BORDER:   return GL_CLAMP_TO_EDGE;
        #endif
        case SG_WRAP_REPEAT:            return GL_REPEAT;
        case SG_WRAP_MIRRORED_REPEAT:   return GL_MIRRORED_REPEAT;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_teximage_type(sg_pixel_format fmt) {
    switch (fmt) {
        case SG_PIXELFORMAT_R8:
        case SG_PIXELFORMAT_R8UI:
        case SG_PIXELFORMAT_RG8:
        case SG_PIXELFORMAT_RG8UI:
        case SG_PIXELFORMAT_RGBA8:
        case SG_PIXELFORMAT_SRGB8A8:
        case SG_PIXELFORMAT_RGBA8UI:
        case SG_PIXELFORMAT_BGRA8:
            return GL_UNSIGNED_BYTE;
        case SG_PIXELFORMAT_R8SN:
        case SG_PIXELFORMAT_R8SI:
        case SG_PIXELFORMAT_RG8SN:
        case SG_PIXELFORMAT_RG8SI:
        case SG_PIXELFORMAT_RGBA8SN:
        case SG_PIXELFORMAT_RGBA8SI:
            return GL_BYTE;
        case SG_PIXELFORMAT_R16:
        case SG_PIXELFORMAT_R16UI:
        case SG_PIXELFORMAT_RG16:
        case SG_PIXELFORMAT_RG16UI:
        case SG_PIXELFORMAT_RGBA16:
        case SG_PIXELFORMAT_RGBA16UI:
            return GL_UNSIGNED_SHORT;
        case SG_PIXELFORMAT_R16SN:
        case SG_PIXELFORMAT_R16SI:
        case SG_PIXELFORMAT_RG16SN:
        case SG_PIXELFORMAT_RG16SI:
        case SG_PIXELFORMAT_RGBA16SN:
        case SG_PIXELFORMAT_RGBA16SI:
            return GL_SHORT;
        case SG_PIXELFORMAT_R16F:
        case SG_PIXELFORMAT_RG16F:
        case SG_PIXELFORMAT_RGBA16F:
            return GL_HALF_FLOAT;
        case SG_PIXELFORMAT_R32UI:
        case SG_PIXELFORMAT_RG32UI:
        case SG_PIXELFORMAT_RGBA32UI:
            return GL_UNSIGNED_INT;
        case SG_PIXELFORMAT_R32SI:
        case SG_PIXELFORMAT_RG32SI:
        case SG_PIXELFORMAT_RGBA32SI:
            return GL_INT;
        case SG_PIXELFORMAT_R32F:
        case SG_PIXELFORMAT_RG32F:
        case SG_PIXELFORMAT_RGBA32F:
            return GL_FLOAT;
        case SG_PIXELFORMAT_RGB10A2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case SG_PIXELFORMAT_RG11B10F:
            return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case SG_PIXELFORMAT_RGB9E5:
            return GL_UNSIGNED_INT_5_9_9_9_REV;
        case SG_PIXELFORMAT_DEPTH:
            return GL_FLOAT;
        case SG_PIXELFORMAT_DEPTH_STENCIL:
            return GL_UNSIGNED_INT_24_8;
        default:
            SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_teximage_format(sg_pixel_format fmt) {
    switch (fmt) {
        case SG_PIXELFORMAT_R8:
        case SG_PIXELFORMAT_R8SN:
        case SG_PIXELFORMAT_R16:
        case SG_PIXELFORMAT_R16SN:
        case SG_PIXELFORMAT_R16F:
        case SG_PIXELFORMAT_R32F:
            return GL_RED;
        case SG_PIXELFORMAT_R8UI:
        case SG_PIXELFORMAT_R8SI:
        case SG_PIXELFORMAT_R16UI:
        case SG_PIXELFORMAT_R16SI:
        case SG_PIXELFORMAT_R32UI:
        case SG_PIXELFORMAT_R32SI:
            return GL_RED_INTEGER;
        case SG_PIXELFORMAT_RG8:
        case SG_PIXELFORMAT_RG8SN:
        case SG_PIXELFORMAT_RG16:
        case SG_PIXELFORMAT_RG16SN:
        case SG_PIXELFORMAT_RG16F:
        case SG_PIXELFORMAT_RG32F:
            return GL_RG;
        case SG_PIXELFORMAT_RG8UI:
        case SG_PIXELFORMAT_RG8SI:
        case SG_PIXELFORMAT_RG16UI:
        case SG_PIXELFORMAT_RG16SI:
        case SG_PIXELFORMAT_RG32UI:
        case SG_PIXELFORMAT_RG32SI:
            return GL_RG_INTEGER;
        case SG_PIXELFORMAT_RGBA8:
        case SG_PIXELFORMAT_SRGB8A8:
        case SG_PIXELFORMAT_RGBA8SN:
        case SG_PIXELFORMAT_RGBA16:
        case SG_PIXELFORMAT_RGBA16SN:
        case SG_PIXELFORMAT_RGBA16F:
        case SG_PIXELFORMAT_RGBA32F:
        case SG_PIXELFORMAT_RGB10A2:
            return GL_RGBA;
        case SG_PIXELFORMAT_RGBA8UI:
        case SG_PIXELFORMAT_RGBA8SI:
        case SG_PIXELFORMAT_RGBA16UI:
        case SG_PIXELFORMAT_RGBA16SI:
        case SG_PIXELFORMAT_RGBA32UI:
        case SG_PIXELFORMAT_RGBA32SI:
            return GL_RGBA_INTEGER;
        case SG_PIXELFORMAT_RG11B10F:
        case SG_PIXELFORMAT_RGB9E5:
            return GL_RGB;
        case SG_PIXELFORMAT_DEPTH:
            return GL_DEPTH_COMPONENT;
        case SG_PIXELFORMAT_DEPTH_STENCIL:
            return GL_DEPTH_STENCIL;
        case SG_PIXELFORMAT_BC1_RGBA:
            return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case SG_PIXELFORMAT_BC2_RGBA:
            return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case SG_PIXELFORMAT_BC3_RGBA:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case SG_PIXELFORMAT_BC3_SRGBA:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        case SG_PIXELFORMAT_BC4_R:
            return GL_COMPRESSED_RED_RGTC1;
        case SG_PIXELFORMAT_BC4_RSN:
            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case SG_PIXELFORMAT_BC5_RG:
            return GL_COMPRESSED_RED_GREEN_RGTC2;
        case SG_PIXELFORMAT_BC5_RGSN:
            return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
        case SG_PIXELFORMAT_BC6H_RGBF:
            return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
        case SG_PIXELFORMAT_BC6H_RGBUF:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
        case SG_PIXELFORMAT_BC7_RGBA:
            return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
        case SG_PIXELFORMAT_BC7_SRGBA:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
        case SG_PIXELFORMAT_PVRTC_RGB_2BPP:
            return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGB_4BPP:
            return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGBA_2BPP:
            return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGBA_4BPP:
            return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        case SG_PIXELFORMAT_ETC2_RGB8:
            return GL_COMPRESSED_RGB8_ETC2;
        case SG_PIXELFORMAT_ETC2_SRGB8:
            return GL_COMPRESSED_SRGB8_ETC2;
        case SG_PIXELFORMAT_ETC2_RGB8A1:
            return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        case SG_PIXELFORMAT_ETC2_RGBA8:
            return GL_COMPRESSED_RGBA8_ETC2_EAC;
        case SG_PIXELFORMAT_ETC2_SRGB8A8:
            return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
        case SG_PIXELFORMAT_EAC_R11:
            return GL_COMPRESSED_R11_EAC;
        case SG_PIXELFORMAT_EAC_R11SN:
            return GL_COMPRESSED_SIGNED_R11_EAC;
        case SG_PIXELFORMAT_EAC_RG11:
            return GL_COMPRESSED_RG11_EAC;
        case SG_PIXELFORMAT_EAC_RG11SN:
            return GL_COMPRESSED_SIGNED_RG11_EAC;
        case SG_PIXELFORMAT_ASTC_4x4_RGBA:
            return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        case SG_PIXELFORMAT_ASTC_4x4_SRGBA:
            return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
        default:
            SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_teximage_internal_format(sg_pixel_format fmt) {
    switch (fmt) {
        case SG_PIXELFORMAT_R8:         return GL_R8;
        case SG_PIXELFORMAT_R8SN:       return GL_R8_SNORM;
        case SG_PIXELFORMAT_R8UI:       return GL_R8UI;
        case SG_PIXELFORMAT_R8SI:       return GL_R8I;
        #if !defined(SOKOL_GLES3)
            case SG_PIXELFORMAT_R16:        return GL_R16;
            case SG_PIXELFORMAT_R16SN:      return GL_R16_SNORM;
        #endif
        case SG_PIXELFORMAT_R16UI:      return GL_R16UI;
        case SG_PIXELFORMAT_R16SI:      return GL_R16I;
        case SG_PIXELFORMAT_R16F:       return GL_R16F;
        case SG_PIXELFORMAT_RG8:        return GL_RG8;
        case SG_PIXELFORMAT_RG8SN:      return GL_RG8_SNORM;
        case SG_PIXELFORMAT_RG8UI:      return GL_RG8UI;
        case SG_PIXELFORMAT_RG8SI:      return GL_RG8I;
        case SG_PIXELFORMAT_R32UI:      return GL_R32UI;
        case SG_PIXELFORMAT_R32SI:      return GL_R32I;
        case SG_PIXELFORMAT_R32F:       return GL_R32F;
        #if !defined(SOKOL_GLES3)
            case SG_PIXELFORMAT_RG16:       return GL_RG16;
            case SG_PIXELFORMAT_RG16SN:     return GL_RG16_SNORM;
        #endif
        case SG_PIXELFORMAT_RG16UI:     return GL_RG16UI;
        case SG_PIXELFORMAT_RG16SI:     return GL_RG16I;
        case SG_PIXELFORMAT_RG16F:      return GL_RG16F;
        case SG_PIXELFORMAT_RGBA8:      return GL_RGBA8;
        case SG_PIXELFORMAT_SRGB8A8:    return GL_SRGB8_ALPHA8;
        case SG_PIXELFORMAT_RGBA8SN:    return GL_RGBA8_SNORM;
        case SG_PIXELFORMAT_RGBA8UI:    return GL_RGBA8UI;
        case SG_PIXELFORMAT_RGBA8SI:    return GL_RGBA8I;
        case SG_PIXELFORMAT_RGB10A2:    return GL_RGB10_A2;
        case SG_PIXELFORMAT_RG11B10F:   return GL_R11F_G11F_B10F;
        case SG_PIXELFORMAT_RGB9E5:     return GL_RGB9_E5;
        case SG_PIXELFORMAT_RG32UI:     return GL_RG32UI;
        case SG_PIXELFORMAT_RG32SI:     return GL_RG32I;
        case SG_PIXELFORMAT_RG32F:      return GL_RG32F;
        #if !defined(SOKOL_GLES3)
            case SG_PIXELFORMAT_RGBA16:     return GL_RGBA16;
            case SG_PIXELFORMAT_RGBA16SN:   return GL_RGBA16_SNORM;
        #endif
        case SG_PIXELFORMAT_RGBA16UI:   return GL_RGBA16UI;
        case SG_PIXELFORMAT_RGBA16SI:   return GL_RGBA16I;
        case SG_PIXELFORMAT_RGBA16F:    return GL_RGBA16F;
        case SG_PIXELFORMAT_RGBA32UI:   return GL_RGBA32UI;
        case SG_PIXELFORMAT_RGBA32SI:   return GL_RGBA32I;
        case SG_PIXELFORMAT_RGBA32F:    return GL_RGBA32F;
        case SG_PIXELFORMAT_DEPTH:      return GL_DEPTH_COMPONENT32F;
        case SG_PIXELFORMAT_DEPTH_STENCIL:      return GL_DEPTH24_STENCIL8;
        case SG_PIXELFORMAT_BC1_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case SG_PIXELFORMAT_BC2_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case SG_PIXELFORMAT_BC3_RGBA:           return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case SG_PIXELFORMAT_BC3_SRGBA:          return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        case SG_PIXELFORMAT_BC4_R:              return GL_COMPRESSED_RED_RGTC1;
        case SG_PIXELFORMAT_BC4_RSN:            return GL_COMPRESSED_SIGNED_RED_RGTC1;
        case SG_PIXELFORMAT_BC5_RG:             return GL_COMPRESSED_RED_GREEN_RGTC2;
        case SG_PIXELFORMAT_BC5_RGSN:           return GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
        case SG_PIXELFORMAT_BC6H_RGBF:          return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
        case SG_PIXELFORMAT_BC6H_RGBUF:         return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
        case SG_PIXELFORMAT_BC7_RGBA:           return GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
        case SG_PIXELFORMAT_BC7_SRGBA:          return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
        case SG_PIXELFORMAT_PVRTC_RGB_2BPP:     return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGB_4BPP:     return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGBA_2BPP:    return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        case SG_PIXELFORMAT_PVRTC_RGBA_4BPP:    return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        case SG_PIXELFORMAT_ETC2_RGB8:          return GL_COMPRESSED_RGB8_ETC2;
        case SG_PIXELFORMAT_ETC2_SRGB8:         return GL_COMPRESSED_SRGB8_ETC2;
        case SG_PIXELFORMAT_ETC2_RGB8A1:        return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
        case SG_PIXELFORMAT_ETC2_RGBA8:         return GL_COMPRESSED_RGBA8_ETC2_EAC;
        case SG_PIXELFORMAT_ETC2_SRGB8A8:       return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
        case SG_PIXELFORMAT_EAC_R11:            return GL_COMPRESSED_R11_EAC;
        case SG_PIXELFORMAT_EAC_R11SN:          return GL_COMPRESSED_SIGNED_R11_EAC;
        case SG_PIXELFORMAT_EAC_RG11:           return GL_COMPRESSED_RG11_EAC;
        case SG_PIXELFORMAT_EAC_RG11SN:         return GL_COMPRESSED_SIGNED_RG11_EAC;
        case SG_PIXELFORMAT_ASTC_4x4_RGBA:      return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        case SG_PIXELFORMAT_ASTC_4x4_SRGBA:     return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_cubeface_target(int face_index) {
    switch (face_index) {
        case 0: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case 1: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case 2: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case 3: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case 4: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case 5: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default: SOKOL_UNREACHABLE; return 0;
    }
}

// see: https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
_SOKOL_PRIVATE void _sg_gl_init_pixelformats(bool has_bgra) {
    _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_R8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_R8SN]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_R8UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_R8SI]);
    #if !defined(SOKOL_GLES3)
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_R16]);
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_R16SN]);
    #endif
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_R16UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_R16SI]);
    _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RG8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RG8SN]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG8UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG8SI]);
    _sg_pixelformat_sr(&_sg.formats[SG_PIXELFORMAT_R32UI]);
    _sg_pixelformat_sr(&_sg.formats[SG_PIXELFORMAT_R32SI]);
    #if !defined(SOKOL_GLES3)
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RG16]);
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RG16SN]);
    #endif
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG16UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG16SI]);
    _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGBA8]);
    _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_SRGB8A8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RGBA8SN]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA8UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA8SI]);
    if (has_bgra) {
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_BGRA8]);
    }
    _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGB10A2]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RG11B10F]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RGB9E5]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG32UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RG32SI]);
    #if !defined(SOKOL_GLES3)
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGBA16]);
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGBA16SN]);
    #endif
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA16UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA16SI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA32UI]);
    _sg_pixelformat_srm(&_sg.formats[SG_PIXELFORMAT_RGBA32SI]);
    _sg_pixelformat_srmd(&_sg.formats[SG_PIXELFORMAT_DEPTH]);
    _sg_pixelformat_srmd(&_sg.formats[SG_PIXELFORMAT_DEPTH_STENCIL]);
}

// FIXME: OES_half_float_blend
_SOKOL_PRIVATE void _sg_gl_init_pixelformats_half_float(bool has_colorbuffer_half_float) {
    if (has_colorbuffer_half_float) {
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_R16F]);
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RG16F]);
        _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGBA16F]);
    } else {
        _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_R16F]);
        _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RG16F]);
        _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RGBA16F]);
    }
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_float(bool has_colorbuffer_float, bool has_texture_float_linear, bool has_float_blend) {
    if (has_texture_float_linear) {
        if (has_colorbuffer_float) {
            if (has_float_blend) {
                _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_R32F]);
                _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RG32F]);
                _sg_pixelformat_all(&_sg.formats[SG_PIXELFORMAT_RGBA32F]);
            } else {
                _sg_pixelformat_sfrm(&_sg.formats[SG_PIXELFORMAT_R32F]);
                _sg_pixelformat_sfrm(&_sg.formats[SG_PIXELFORMAT_RG32F]);
                _sg_pixelformat_sfrm(&_sg.formats[SG_PIXELFORMAT_RGBA32F]);
            }
        } else {
            _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_R32F]);
            _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RG32F]);
            _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_RGBA32F]);
        }
    } else {
        if (has_colorbuffer_float) {
            _sg_pixelformat_sbrm(&_sg.formats[SG_PIXELFORMAT_R32F]);
            _sg_pixelformat_sbrm(&_sg.formats[SG_PIXELFORMAT_RG32F]);
            _sg_pixelformat_sbrm(&_sg.formats[SG_PIXELFORMAT_RGBA32F]);
        } else {
            _sg_pixelformat_s(&_sg.formats[SG_PIXELFORMAT_R32F]);
            _sg_pixelformat_s(&_sg.formats[SG_PIXELFORMAT_RG32F]);
            _sg_pixelformat_s(&_sg.formats[SG_PIXELFORMAT_RGBA32F]);
        }
    }
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_s3tc(void) {
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC1_RGBA]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC2_RGBA]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC3_RGBA]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC3_SRGBA]);
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_rgtc(void) {
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC4_R]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC4_RSN]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC5_RG]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC5_RGSN]);
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_bptc(void) {
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC6H_RGBF]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC6H_RGBUF]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC7_RGBA]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_BC7_SRGBA]);
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_pvrtc(void) {
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_PVRTC_RGB_2BPP]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_PVRTC_RGB_4BPP]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_PVRTC_RGBA_2BPP]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_PVRTC_RGBA_4BPP]);
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_etc2(void) {
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ETC2_RGB8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ETC2_SRGB8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ETC2_RGB8A1]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ETC2_RGBA8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ETC2_SRGB8A8]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_EAC_R11]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_EAC_R11SN]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_EAC_RG11]);
    _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_EAC_RG11SN]);
}

_SOKOL_PRIVATE void _sg_gl_init_pixelformats_astc(void) {
     _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ASTC_4x4_RGBA]);
     _sg_pixelformat_sf(&_sg.formats[SG_PIXELFORMAT_ASTC_4x4_SRGBA]);
 }

_SOKOL_PRIVATE void _sg_gl_init_limits(void) {
    _SG_GL_CHECK_ERROR();
    GLint gl_int;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.max_image_size_2d = gl_int;
    _sg.limits.max_image_size_array = gl_int;
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.max_image_size_cube = gl_int;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_int);
    _SG_GL_CHECK_ERROR();
    if (gl_int > SG_MAX_VERTEX_ATTRIBUTES) {
        gl_int = SG_MAX_VERTEX_ATTRIBUTES;
    }
    _sg.limits.max_vertex_attrs = gl_int;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.gl_max_vertex_uniform_components = gl_int;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.max_image_size_3d = gl_int;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.max_image_array_layers = gl_int;
    if (_sg.gl.ext_anisotropic) {
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_int);
        _SG_GL_CHECK_ERROR();
        _sg.gl.max_anisotropy = gl_int;
    } else {
        _sg.gl.max_anisotropy = 1;
    }
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_int);
    _SG_GL_CHECK_ERROR();
    _sg.limits.gl_max_combined_texture_image_units = gl_int;
}

#if defined(SOKOL_GLCORE)
_SOKOL_PRIVATE void _sg_gl_init_caps_glcore(void) {
    _sg.backend = SG_BACKEND_GLCORE;

    GLint major_version = 0;
    GLint minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    const int version = major_version * 100 + minor_version * 10;
    _sg.features.origin_top_left = false;
    _sg.features.image_clamp_to_border = true;
    _sg.features.mrt_independent_blend_state = false;
    _sg.features.mrt_independent_write_mask = true;
    _sg.features.storage_buffer = version >= 430;

    // scan extensions
    bool has_s3tc = false;  // BC1..BC3
    bool has_rgtc = false;  // BC4 and BC5
    bool has_bptc = false;  // BC6H and BC7
    bool has_pvrtc = false;
    bool has_etc2 = false;
    bool has_astc = false;
    GLint num_ext = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
    for (int i = 0; i < num_ext; i++) {
        const char* ext = (const char*) glGetStringi(GL_EXTENSIONS, (GLuint)i);
        if (ext) {
            if (strstr(ext, "_texture_compression_s3tc")) {
                has_s3tc = true;
            } else if (strstr(ext, "_texture_compression_rgtc")) {
                has_rgtc = true;
            } else if (strstr(ext, "_texture_compression_bptc")) {
                has_bptc = true;
            } else if (strstr(ext, "_texture_compression_pvrtc")) {
                has_pvrtc = true;
            } else if (strstr(ext, "_ES3_compatibility")) {
                has_etc2 = true;
            } else if (strstr(ext, "_texture_filter_anisotropic")) {
                _sg.gl.ext_anisotropic = true;
            } else if (strstr(ext, "_texture_compression_astc_ldr")) {
                has_astc = true;
            }
        }
    }

    // limits
    _sg_gl_init_limits();

    // pixel formats
    const bool has_bgra = false;    // not a bug
    const bool has_colorbuffer_float = true;
    const bool has_colorbuffer_half_float = true;
    const bool has_texture_float_linear = true; // FIXME???
    const bool has_float_blend = true;
    _sg_gl_init_pixelformats(has_bgra);
    _sg_gl_init_pixelformats_float(has_colorbuffer_float, has_texture_float_linear, has_float_blend);
    _sg_gl_init_pixelformats_half_float(has_colorbuffer_half_float);
    if (has_s3tc) {
        _sg_gl_init_pixelformats_s3tc();
    }
    if (has_rgtc) {
        _sg_gl_init_pixelformats_rgtc();
    }
    if (has_bptc) {
        _sg_gl_init_pixelformats_bptc();
    }
    if (has_pvrtc) {
        _sg_gl_init_pixelformats_pvrtc();
    }
    if (has_etc2) {
        _sg_gl_init_pixelformats_etc2();
    }
    if (has_astc) {
        _sg_gl_init_pixelformats_astc();
    }
}
#endif

//-- state cache implementation ------------------------------------------------
_SOKOL_PRIVATE GLuint _sg_gl_storagebuffer_bind_index(int stage, int slot) {
    SOKOL_ASSERT((stage >= 0) && (stage < SG_NUM_SHADER_STAGES));
    SOKOL_ASSERT((slot >= 0) && (slot < SG_MAX_SHADERSTAGE_STORAGEBUFFERS));
    return (GLuint) (stage * _SG_GL_STORAGEBUFFER_STAGE_INDEX_PITCH + slot);
}

_SOKOL_PRIVATE void _sg_gl_cache_clear_buffer_bindings(bool force) {
    if (force || (_sg.gl.cache.vertex_buffer != 0)) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        _sg.gl.cache.vertex_buffer = 0;
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    if (force || (_sg.gl.cache.index_buffer != 0)) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        _sg.gl.cache.index_buffer = 0;
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    if (force || (_sg.gl.cache.storage_buffer != 0)) {
        if (_sg.features.storage_buffer) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
        _sg.gl.cache.storage_buffer = 0;
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    for (int stage = 0; stage < SG_NUM_SHADER_STAGES; stage++) {
        for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++) {
            if (force || (_sg.gl.cache.stage_storage_buffers[stage][i] != 0)) {
                const GLuint bind_index = _sg_gl_storagebuffer_bind_index(stage, i);
                if (_sg.features.storage_buffer) {
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_index, 0);
                }
                _sg.gl.cache.stage_storage_buffers[stage][i] = 0;
                _sg_stats_add(gl.num_bind_buffer, 1);
            }
        }
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_bind_buffer(GLenum target, GLuint buffer) {
    SOKOL_ASSERT((GL_ARRAY_BUFFER == target) || (GL_ELEMENT_ARRAY_BUFFER == target) || (GL_SHADER_STORAGE_BUFFER == target));
    if (target == GL_ARRAY_BUFFER) {
        if (_sg.gl.cache.vertex_buffer != buffer) {
            _sg.gl.cache.vertex_buffer = buffer;
            glBindBuffer(target, buffer);
            _sg_stats_add(gl.num_bind_buffer, 1);
        }
    } else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        if (_sg.gl.cache.index_buffer != buffer) {
            _sg.gl.cache.index_buffer = buffer;
            glBindBuffer(target, buffer);
            _sg_stats_add(gl.num_bind_buffer, 1);
        }
    } else if (target == GL_SHADER_STORAGE_BUFFER) {
        if (_sg.gl.cache.storage_buffer != buffer) {
            _sg.gl.cache.storage_buffer = buffer;
            if (_sg.features.storage_buffer) {
                glBindBuffer(target, buffer);
            }
            _sg_stats_add(gl.num_bind_buffer, 1);
        }
    } else {
        SOKOL_UNREACHABLE;
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_bind_storage_buffer(int stage, int slot, GLuint buffer) {
    SOKOL_ASSERT((stage >= 0) && (stage < SG_NUM_SHADER_STAGES));
    SOKOL_ASSERT((slot >= 0) && (slot < SG_MAX_SHADERSTAGE_STORAGEBUFFERS));
    if (_sg.gl.cache.stage_storage_buffers[stage][slot] != buffer) {
        _sg.gl.cache.stage_storage_buffers[stage][slot] = buffer;
        _sg.gl.cache.storage_buffer = buffer; // not a bug
        GLuint bind_index = _sg_gl_storagebuffer_bind_index(stage, slot);
        if (_sg.features.storage_buffer) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_index, buffer);
        }
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_store_buffer_binding(GLenum target) {
    if (target == GL_ARRAY_BUFFER) {
        _sg.gl.cache.stored_vertex_buffer = _sg.gl.cache.vertex_buffer;
    } else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        _sg.gl.cache.stored_index_buffer = _sg.gl.cache.index_buffer;
    } else if (target == GL_SHADER_STORAGE_BUFFER) {
        _sg.gl.cache.stored_storage_buffer = _sg.gl.cache.storage_buffer;
    } else {
        SOKOL_UNREACHABLE;
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_restore_buffer_binding(GLenum target) {
    if (target == GL_ARRAY_BUFFER) {
        if (_sg.gl.cache.stored_vertex_buffer != 0) {
            // we only care about restoring valid ids
            _sg_gl_cache_bind_buffer(target, _sg.gl.cache.stored_vertex_buffer);
            _sg.gl.cache.stored_vertex_buffer = 0;
        }
    } else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        if (_sg.gl.cache.stored_index_buffer != 0) {
            // we only care about restoring valid ids
            _sg_gl_cache_bind_buffer(target, _sg.gl.cache.stored_index_buffer);
            _sg.gl.cache.stored_index_buffer = 0;
        }
    } else if (target == GL_SHADER_STORAGE_BUFFER) {
        if (_sg.gl.cache.stored_storage_buffer != 0) {
            // we only care about restoring valid ids
            _sg_gl_cache_bind_buffer(target, _sg.gl.cache.stored_storage_buffer);
            _sg.gl.cache.stored_storage_buffer = 0;
        }
    } else {
        SOKOL_UNREACHABLE;
    }
}

// called when _sg_gl_discard_buffer()
_SOKOL_PRIVATE void _sg_gl_cache_invalidate_buffer(GLuint buf) {
    if (buf == _sg.gl.cache.vertex_buffer) {
        _sg.gl.cache.vertex_buffer = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    if (buf == _sg.gl.cache.index_buffer) {
        _sg.gl.cache.index_buffer = 0;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    if (buf == _sg.gl.cache.storage_buffer) {
        _sg.gl.cache.storage_buffer = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        _sg_stats_add(gl.num_bind_buffer, 1);
    }
    for (int stage = 0; stage < SG_NUM_SHADER_STAGES; stage++) {
        for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++) {
            if (buf == _sg.gl.cache.stage_storage_buffers[stage][i]) {
                _sg.gl.cache.stage_storage_buffers[stage][i] = 0;
                _sg.gl.cache.storage_buffer = 0; // not a bug!
                const GLuint bind_index = _sg_gl_storagebuffer_bind_index(stage, i);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_index, 0);
                _sg_stats_add(gl.num_bind_buffer, 1);
            }
        }
    }
    if (buf == _sg.gl.cache.stored_vertex_buffer) {
        _sg.gl.cache.stored_vertex_buffer = 0;
    }
    if (buf == _sg.gl.cache.stored_index_buffer) {
        _sg.gl.cache.stored_index_buffer = 0;
    }
    if (buf == _sg.gl.cache.stored_storage_buffer) {
        _sg.gl.cache.stored_storage_buffer = 0;
    }
    for (int i = 0; i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
        if (buf == _sg.gl.cache.attrs[i].gl_vbuf) {
            _sg.gl.cache.attrs[i].gl_vbuf = 0;
        }
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_active_texture(GLenum texture) {
    _SG_GL_CHECK_ERROR();
    if (_sg.gl.cache.cur_active_texture != texture) {
        _sg.gl.cache.cur_active_texture = texture;
        glActiveTexture(texture);
        _sg_stats_add(gl.num_active_texture, 1);
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_cache_clear_texture_sampler_bindings(bool force) {
    _SG_GL_CHECK_ERROR();
    for (int i = 0; (i < _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE) && (i < _sg.limits.gl_max_combined_texture_image_units); i++) {
        if (force || (_sg.gl.cache.texture_samplers[i].texture != 0)) {
            GLenum gl_texture_unit = (GLenum) (GL_TEXTURE0 + i);
            glActiveTexture(gl_texture_unit);
            _sg_stats_add(gl.num_active_texture, 1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            _sg_stats_add(gl.num_bind_texture, 4);
            glBindSampler((GLuint)i, 0);
            _sg_stats_add(gl.num_bind_sampler, 1);
            _sg.gl.cache.texture_samplers[i].target = 0;
            _sg.gl.cache.texture_samplers[i].texture = 0;
            _sg.gl.cache.texture_samplers[i].sampler = 0;
            _sg.gl.cache.cur_active_texture = gl_texture_unit;
        }
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_cache_bind_texture_sampler(int slot_index, GLenum target, GLuint texture, GLuint sampler) {
    /* it's valid to call this function with target=0 and/or texture=0
       target=0 will unbind the previous binding, texture=0 will clear
       the new binding
    */
    SOKOL_ASSERT((slot_index >= 0) && (slot_index < _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE));
    if (slot_index >= _sg.limits.gl_max_combined_texture_image_units) {
        return;
    }
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_texture_sampler_bind_slot* slot = &_sg.gl.cache.texture_samplers[slot_index];
    if ((slot->target != target) || (slot->texture != texture) || (slot->sampler != sampler)) {
        _sg_gl_cache_active_texture((GLenum)(GL_TEXTURE0 + slot_index));
        // if the target has changed, clear the previous binding on that target
        if ((target != slot->target) && (slot->target != 0)) {
            glBindTexture(slot->target, 0);
            _SG_GL_CHECK_ERROR();
            _sg_stats_add(gl.num_bind_texture, 1);
        }
        // apply new binding (can be 0 to unbind)
        if (target != 0) {
            glBindTexture(target, texture);
            _SG_GL_CHECK_ERROR();
            _sg_stats_add(gl.num_bind_texture, 1);
        }
        // apply new sampler (can be 0 to unbind)
        glBindSampler((GLuint)slot_index, sampler);
        _SG_GL_CHECK_ERROR();
        _sg_stats_add(gl.num_bind_sampler, 1);

        slot->target = target;
        slot->texture = texture;
        slot->sampler = sampler;
    }
}

_SOKOL_PRIVATE void _sg_gl_cache_store_texture_sampler_binding(int slot_index) {
    SOKOL_ASSERT((slot_index >= 0) && (slot_index < _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE));
    _sg.gl.cache.stored_texture_sampler = _sg.gl.cache.texture_samplers[slot_index];
}

_SOKOL_PRIVATE void _sg_gl_cache_restore_texture_sampler_binding(int slot_index) {
    SOKOL_ASSERT((slot_index >= 0) && (slot_index < _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE));
    _sg_gl_cache_texture_sampler_bind_slot* slot = &_sg.gl.cache.stored_texture_sampler;
    if (slot->texture != 0) {
        // we only care about restoring valid ids
        SOKOL_ASSERT(slot->target != 0);
        _sg_gl_cache_bind_texture_sampler(slot_index, slot->target, slot->texture, slot->sampler);
        slot->target = 0;
        slot->texture = 0;
        slot->sampler = 0;
    }
}

// called from _sg_gl_discard_texture() and _sg_gl_discard_sampler()
_SOKOL_PRIVATE void _sg_gl_cache_invalidate_texture_sampler(GLuint tex, GLuint smp) {
    _SG_GL_CHECK_ERROR();
    for (int i = 0; i < _SG_GL_TEXTURE_SAMPLER_CACHE_SIZE; i++) {
        _sg_gl_cache_texture_sampler_bind_slot* slot = &_sg.gl.cache.texture_samplers[i];
        if ((0 != slot->target) && ((tex == slot->texture) || (smp == slot->sampler))) {
            _sg_gl_cache_active_texture((GLenum)(GL_TEXTURE0 + i));
            glBindTexture(slot->target, 0);
            _SG_GL_CHECK_ERROR();
            _sg_stats_add(gl.num_bind_texture, 1);
            glBindSampler((GLuint)i, 0);
            _SG_GL_CHECK_ERROR();
            _sg_stats_add(gl.num_bind_sampler, 1);
            slot->target = 0;
            slot->texture = 0;
            slot->sampler = 0;
        }
    }
    if ((tex == _sg.gl.cache.stored_texture_sampler.texture) || (smp == _sg.gl.cache.stored_texture_sampler.sampler)) {
        _sg.gl.cache.stored_texture_sampler.target = 0;
        _sg.gl.cache.stored_texture_sampler.texture = 0;
        _sg.gl.cache.stored_texture_sampler.sampler = 0;
    }
}

// called from _sg_gl_discard_shader()
_SOKOL_PRIVATE void _sg_gl_cache_invalidate_program(GLuint prog) {
    if (prog == _sg.gl.cache.prog) {
        _sg.gl.cache.prog = 0;
        glUseProgram(0);
        _sg_stats_add(gl.num_use_program, 1);
    }
}

// called from _sg_gl_discard_pipeline()
_SOKOL_PRIVATE void _sg_gl_cache_invalidate_pipeline(_sg_pipeline_t* pip) {
    if (pip == _sg.gl.cache.cur_pipeline) {
        _sg.gl.cache.cur_pipeline = 0;
        _sg.gl.cache.cur_pipeline_id.id = SG_INVALID_ID;
    }
}

_SOKOL_PRIVATE void _sg_gl_reset_state_cache(void) {
    _SG_GL_CHECK_ERROR();
    glBindVertexArray(_sg.gl.vao);
    _SG_GL_CHECK_ERROR();
    _sg_clear(&_sg.gl.cache, sizeof(_sg.gl.cache));
    _sg_gl_cache_clear_buffer_bindings(true);
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_clear_texture_sampler_bindings(true);
    _SG_GL_CHECK_ERROR();
    for (int i = 0; i < _sg.limits.max_vertex_attrs; i++) {
        _sg_gl_attr_t* attr = &_sg.gl.cache.attrs[i].gl_attr;
        attr->vb_index = -1;
        attr->divisor = -1;
        glDisableVertexAttribArray((GLuint)i);
        _SG_GL_CHECK_ERROR();
        _sg_stats_add(gl.num_disable_vertex_attrib_array, 1);
    }
    _sg.gl.cache.cur_primitive_type = GL_TRIANGLES;

    // shader program
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&_sg.gl.cache.prog);
    _SG_GL_CHECK_ERROR();

    // depth and stencil state
    _sg.gl.cache.depth.compare = SG_COMPAREFUNC_ALWAYS;
    _sg.gl.cache.stencil.front.compare = SG_COMPAREFUNC_ALWAYS;
    _sg.gl.cache.stencil.front.fail_op = SG_STENCILOP_KEEP;
    _sg.gl.cache.stencil.front.depth_fail_op = SG_STENCILOP_KEEP;
    _sg.gl.cache.stencil.front.pass_op = SG_STENCILOP_KEEP;
    _sg.gl.cache.stencil.back.compare = SG_COMPAREFUNC_ALWAYS;
    _sg.gl.cache.stencil.back.fail_op = SG_STENCILOP_KEEP;
    _sg.gl.cache.stencil.back.depth_fail_op = SG_STENCILOP_KEEP;
    _sg.gl.cache.stencil.back.pass_op = SG_STENCILOP_KEEP;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_FALSE);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0);
    _sg_stats_add(gl.num_render_state, 7);

    // blend state
    _sg.gl.cache.blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    _sg.gl.cache.blend.dst_factor_rgb = SG_BLENDFACTOR_ZERO;
    _sg.gl.cache.blend.op_rgb = SG_BLENDOP_ADD;
    _sg.gl.cache.blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    _sg.gl.cache.blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
    _sg.gl.cache.blend.op_alpha = SG_BLENDOP_ADD;
    glDisable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
    _sg_stats_add(gl.num_render_state, 4);

    // standalone state
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        _sg.gl.cache.color_write_mask[i] = SG_COLORMASK_RGBA;
    }
    _sg.gl.cache.cull_mode = SG_CULLMODE_NONE;
    _sg.gl.cache.face_winding = SG_FACEWINDING_CW;
    _sg.gl.cache.sample_count = 1;
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glPolygonOffset(0.0f, 0.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    _sg_stats_add(gl.num_render_state, 10);
    #if defined(SOKOL_GLCORE)
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        _sg_stats_add(gl.num_render_state, 2);
    #endif
}

_SOKOL_PRIVATE void _sg_gl_setup_backend(const sg_desc* desc) {
    _SOKOL_UNUSED(desc);

    // assumes that _sg.gl is already zero-initialized
    _sg.gl.valid = true;

    #if defined(_SOKOL_USE_WIN32_GL_LOADER)
    _sg_gl_load_opengl();
    #endif

    // clear initial GL error state
    #if defined(SOKOL_DEBUG)
        while (glGetError() != GL_NO_ERROR);
    #endif
    #if defined(SOKOL_GLCORE)
        _sg_gl_init_caps_glcore();
    #elif defined(SOKOL_GLES3)
        _sg_gl_init_caps_gles3();
    #endif

    glGenVertexArrays(1, &_sg.gl.vao);
    glBindVertexArray(_sg.gl.vao);
    _SG_GL_CHECK_ERROR();
    // incoming texture data is generally expected to be packed tightly
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    #if defined(SOKOL_GLCORE)
        // enable seamless cubemap sampling (only desktop GL)
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    #endif
    _sg_gl_reset_state_cache();
}

_SOKOL_PRIVATE void _sg_gl_discard_backend(void) {
    SOKOL_ASSERT(_sg.gl.valid);
    if (_sg.gl.vao) {
        glDeleteVertexArrays(1, &_sg.gl.vao);
    }
    #if defined(_SOKOL_USE_WIN32_GL_LOADER)
    _sg_gl_unload_opengl();
    #endif
    _sg.gl.valid = false;
}

//-- GL backend resource creation and destruction ------------------------------
_SOKOL_PRIVATE sg_resource_state _sg_gl_create_buffer(_sg_buffer_t* buf, const sg_buffer_desc* desc) {
    SOKOL_ASSERT(buf && desc);
    _SG_GL_CHECK_ERROR();
    buf->gl.injected = (0 != desc->gl_buffers[0]);
    const GLenum gl_target = _sg_gl_buffer_target(buf->cmn.type);
    const GLenum gl_usage  = _sg_gl_usage(buf->cmn.usage);
    for (int slot = 0; slot < buf->cmn.num_slots; slot++) {
        GLuint gl_buf = 0;
        if (buf->gl.injected) {
            SOKOL_ASSERT(desc->gl_buffers[slot]);
            gl_buf = desc->gl_buffers[slot];
        } else {
            glGenBuffers(1, &gl_buf);
            SOKOL_ASSERT(gl_buf);
            _sg_gl_cache_store_buffer_binding(gl_target);
            _sg_gl_cache_bind_buffer(gl_target, gl_buf);
            glBufferData(gl_target, buf->cmn.size, 0, gl_usage);
            if (buf->cmn.usage == SG_USAGE_IMMUTABLE) {
                SOKOL_ASSERT(desc->data.ptr);
                glBufferSubData(gl_target, 0, buf->cmn.size, desc->data.ptr);
            }
            _sg_gl_cache_restore_buffer_binding(gl_target);
        }
        buf->gl.buf[slot] = gl_buf;
    }
    _SG_GL_CHECK_ERROR();
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_buffer(_sg_buffer_t* buf) {
    SOKOL_ASSERT(buf);
    _SG_GL_CHECK_ERROR();
    for (int slot = 0; slot < buf->cmn.num_slots; slot++) {
        if (buf->gl.buf[slot]) {
            _sg_gl_cache_invalidate_buffer(buf->gl.buf[slot]);
            if (!buf->gl.injected) {
                glDeleteBuffers(1, &buf->gl.buf[slot]);
            }
        }
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE bool _sg_gl_supported_texture_format(sg_pixel_format fmt) {
    const int fmt_index = (int) fmt;
    SOKOL_ASSERT((fmt_index > SG_PIXELFORMAT_NONE) && (fmt_index < _SG_PIXELFORMAT_NUM));
    return _sg.formats[fmt_index].sample;
}

_SOKOL_PRIVATE sg_resource_state _sg_gl_create_image(_sg_image_t* img, const sg_image_desc* desc) {
    SOKOL_ASSERT(img && desc);
    _SG_GL_CHECK_ERROR();
    img->gl.injected = (0 != desc->gl_textures[0]);

    // check if texture format is support
    if (!_sg_gl_supported_texture_format(img->cmn.pixel_format)) {
        _SG_ERROR(GL_TEXTURE_FORMAT_NOT_SUPPORTED);
        return SG_RESOURCESTATE_FAILED;
    }
    const GLenum gl_internal_format = _sg_gl_teximage_internal_format(img->cmn.pixel_format);

    // if this is a MSAA render target, a render buffer object will be created instead of a regulat texture
    // (since GLES3 has no multisampled texture objects)
    if (img->cmn.render_target && (img->cmn.sample_count > 1)) {
        glGenRenderbuffers(1, &img->gl.msaa_render_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, img->gl.msaa_render_buffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, img->cmn.sample_count, gl_internal_format, img->cmn.width, img->cmn.height);
    } else if (img->gl.injected) {
        img->gl.target = _sg_gl_texture_target(img->cmn.type);
        // inject externally GL textures
        for (int slot = 0; slot < img->cmn.num_slots; slot++) {
            SOKOL_ASSERT(desc->gl_textures[slot]);
            img->gl.tex[slot] = desc->gl_textures[slot];
        }
        if (desc->gl_texture_target) {
            img->gl.target = (GLenum)desc->gl_texture_target;
        }
    } else {
        // create our own GL texture(s)
        img->gl.target = _sg_gl_texture_target(img->cmn.type);
        const GLenum gl_format = _sg_gl_teximage_format(img->cmn.pixel_format);
        const bool is_compressed = _sg_is_compressed_pixel_format(img->cmn.pixel_format);
        for (int slot = 0; slot < img->cmn.num_slots; slot++) {
            glGenTextures(1, &img->gl.tex[slot]);
            SOKOL_ASSERT(img->gl.tex[slot]);
            _sg_gl_cache_store_texture_sampler_binding(0);
            _sg_gl_cache_bind_texture_sampler(0, img->gl.target, img->gl.tex[slot], 0);
            glTexParameteri(img->gl.target, GL_TEXTURE_MAX_LEVEL, img->cmn.num_mipmaps - 1);
            const int num_faces = img->cmn.type == SG_IMAGETYPE_CUBE ? 6 : 1;
            int data_index = 0;
            for (int face_index = 0; face_index < num_faces; face_index++) {
                for (int mip_index = 0; mip_index < img->cmn.num_mipmaps; mip_index++, data_index++) {
                    GLenum gl_img_target = img->gl.target;
                    if (SG_IMAGETYPE_CUBE == img->cmn.type) {
                        gl_img_target = _sg_gl_cubeface_target(face_index);
                    }
                    const GLvoid* data_ptr = desc->data.subimage[face_index][mip_index].ptr;
                    const int mip_width = _sg_miplevel_dim(img->cmn.width, mip_index);
                    const int mip_height = _sg_miplevel_dim(img->cmn.height, mip_index);
                    if ((SG_IMAGETYPE_2D == img->cmn.type) || (SG_IMAGETYPE_CUBE == img->cmn.type)) {
                        if (is_compressed) {
                            const GLsizei data_size = (GLsizei) desc->data.subimage[face_index][mip_index].size;
                            glCompressedTexImage2D(gl_img_target, mip_index, gl_internal_format,
                                mip_width, mip_height, 0, data_size, data_ptr);
                        } else {
                            const GLenum gl_type = _sg_gl_teximage_type(img->cmn.pixel_format);
                            glTexImage2D(gl_img_target, mip_index, (GLint)gl_internal_format,
                                mip_width, mip_height, 0, gl_format, gl_type, data_ptr);
                        }
                    } else if ((SG_IMAGETYPE_3D == img->cmn.type) || (SG_IMAGETYPE_ARRAY == img->cmn.type)) {
                        int mip_depth = img->cmn.num_slices;
                        if (SG_IMAGETYPE_3D == img->cmn.type) {
                            mip_depth = _sg_miplevel_dim(mip_depth, mip_index);
                        }
                        if (is_compressed) {
                            const GLsizei data_size = (GLsizei) desc->data.subimage[face_index][mip_index].size;
                            glCompressedTexImage3D(gl_img_target, mip_index, gl_internal_format,
                                mip_width, mip_height, mip_depth, 0, data_size, data_ptr);
                        } else {
                            const GLenum gl_type = _sg_gl_teximage_type(img->cmn.pixel_format);
                            glTexImage3D(gl_img_target, mip_index, (GLint)gl_internal_format,
                                mip_width, mip_height, mip_depth, 0, gl_format, gl_type, data_ptr);
                        }
                    }
                }
            }
            _sg_gl_cache_restore_texture_sampler_binding(0);
        }
    }
    _SG_GL_CHECK_ERROR();
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_image(_sg_image_t* img) {
    SOKOL_ASSERT(img);
    _SG_GL_CHECK_ERROR();
    for (int slot = 0; slot < img->cmn.num_slots; slot++) {
        if (img->gl.tex[slot]) {
            _sg_gl_cache_invalidate_texture_sampler(img->gl.tex[slot], 0);
            if (!img->gl.injected) {
                glDeleteTextures(1, &img->gl.tex[slot]);
            }
        }
    }
    if (img->gl.msaa_render_buffer) {
        glDeleteRenderbuffers(1, &img->gl.msaa_render_buffer);
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE sg_resource_state _sg_gl_create_sampler(_sg_sampler_t* smp, const sg_sampler_desc* desc) {
    SOKOL_ASSERT(smp && desc);
    _SG_GL_CHECK_ERROR();
    smp->gl.injected = (0 != desc->gl_sampler);
    if (smp->gl.injected) {
        smp->gl.smp = (GLuint) desc->gl_sampler;
    } else {
        glGenSamplers(1, &smp->gl.smp);
        SOKOL_ASSERT(smp->gl.smp);

        const GLenum gl_min_filter = _sg_gl_min_filter(smp->cmn.min_filter, smp->cmn.mipmap_filter);
        const GLenum gl_mag_filter = _sg_gl_mag_filter(smp->cmn.mag_filter);
        glSamplerParameteri(smp->gl.smp, GL_TEXTURE_MIN_FILTER, (GLint)gl_min_filter);
        glSamplerParameteri(smp->gl.smp, GL_TEXTURE_MAG_FILTER, (GLint)gl_mag_filter);
        // GL spec has strange defaults for mipmap min/max lod: -1000 to +1000
        const float min_lod = _sg_clamp(desc->min_lod, 0.0f, 1000.0f);
        const float max_lod = _sg_clamp(desc->max_lod, 0.0f, 1000.0f);
        glSamplerParameterf(smp->gl.smp, GL_TEXTURE_MIN_LOD, min_lod);
        glSamplerParameterf(smp->gl.smp, GL_TEXTURE_MAX_LOD, max_lod);
        glSamplerParameteri(smp->gl.smp, GL_TEXTURE_WRAP_S, (GLint)_sg_gl_wrap(smp->cmn.wrap_u));
        glSamplerParameteri(smp->gl.smp, GL_TEXTURE_WRAP_T, (GLint)_sg_gl_wrap(smp->cmn.wrap_v));
        glSamplerParameteri(smp->gl.smp, GL_TEXTURE_WRAP_R, (GLint)_sg_gl_wrap(smp->cmn.wrap_w));
        #if defined(SOKOL_GLCORE)
        float border[4];
        switch (smp->cmn.border_color) {
            case SG_BORDERCOLOR_TRANSPARENT_BLACK:
                border[0] = 0.0f; border[1] = 0.0f; border[2] = 0.0f; border[3] = 0.0f;
                break;
            case SG_BORDERCOLOR_OPAQUE_WHITE:
                border[0] = 1.0f; border[1] = 1.0f; border[2] = 1.0f; border[3] = 1.0f;
                break;
            default:
                border[0] = 0.0f; border[1] = 0.0f; border[2] = 0.0f; border[3] = 1.0f;
                break;
        }
        glSamplerParameterfv(smp->gl.smp, GL_TEXTURE_BORDER_COLOR, border);
        #endif
        if (smp->cmn.compare != SG_COMPAREFUNC_NEVER) {
            glSamplerParameteri(smp->gl.smp, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(smp->gl.smp, GL_TEXTURE_COMPARE_FUNC, (GLint)_sg_gl_compare_func(smp->cmn.compare));
        } else {
            glSamplerParameteri(smp->gl.smp, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        }
        if (_sg.gl.ext_anisotropic && (smp->cmn.max_anisotropy > 1)) {
            GLint max_aniso = (GLint) smp->cmn.max_anisotropy;
            if (max_aniso > _sg.gl.max_anisotropy) {
                max_aniso = _sg.gl.max_anisotropy;
            }
            glSamplerParameteri(smp->gl.smp, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
        }
    }
    _SG_GL_CHECK_ERROR();
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_sampler(_sg_sampler_t* smp) {
    SOKOL_ASSERT(smp);
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_invalidate_texture_sampler(0, smp->gl.smp);
    if (!smp->gl.injected) {
        glDeleteSamplers(1, &smp->gl.smp);
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE GLuint _sg_gl_compile_shader(sg_shader_stage stage, const char* src) {
    SOKOL_ASSERT(src);
    _SG_GL_CHECK_ERROR();
    GLuint gl_shd = glCreateShader(_sg_gl_shader_stage(stage));
    glShaderSource(gl_shd, 1, &src, 0);
    glCompileShader(gl_shd);
    GLint compile_status = 0;
    glGetShaderiv(gl_shd, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status) {
        // compilation failed, log error and delete shader
        GLint log_len = 0;
        glGetShaderiv(gl_shd, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            GLchar* log_buf = (GLchar*) _sg_malloc((size_t)log_len);
            glGetShaderInfoLog(gl_shd, log_len, &log_len, log_buf);
            _SG_ERROR(GL_SHADER_COMPILATION_FAILED);
            _SG_LOGMSG(GL_SHADER_COMPILATION_FAILED, log_buf);
            _sg_free(log_buf);
        }
        glDeleteShader(gl_shd);
        gl_shd = 0;
    }
    _SG_GL_CHECK_ERROR();
    return gl_shd;
}

_SOKOL_PRIVATE sg_resource_state _sg_gl_create_shader(_sg_shader_t* shd, const sg_shader_desc* desc) {
    SOKOL_ASSERT(shd && desc);
    SOKOL_ASSERT(!shd->gl.prog);
    _SG_GL_CHECK_ERROR();

    // copy the optional vertex attribute names over
    for (int i = 0; i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
        _sg_strcpy(&shd->gl.attrs[i].name, desc->attrs[i].name);
    }

    GLuint gl_vs = _sg_gl_compile_shader(SG_SHADERSTAGE_VS, desc->vs.source);
    GLuint gl_fs = _sg_gl_compile_shader(SG_SHADERSTAGE_FS, desc->fs.source);
    if (!(gl_vs && gl_fs)) {
        return SG_RESOURCESTATE_FAILED;
    }
    GLuint gl_prog = glCreateProgram();
    glAttachShader(gl_prog, gl_vs);
    glAttachShader(gl_prog, gl_fs);
    glLinkProgram(gl_prog);
    glDeleteShader(gl_vs);
    glDeleteShader(gl_fs);
    _SG_GL_CHECK_ERROR();

    GLint link_status;
    glGetProgramiv(gl_prog, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        GLint log_len = 0;
        glGetProgramiv(gl_prog, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            GLchar* log_buf = (GLchar*) _sg_malloc((size_t)log_len);
            glGetProgramInfoLog(gl_prog, log_len, &log_len, log_buf);
            _SG_ERROR(GL_SHADER_LINKING_FAILED);
            _SG_LOGMSG(GL_SHADER_LINKING_FAILED, log_buf);
            _sg_free(log_buf);
        }
        glDeleteProgram(gl_prog);
        return SG_RESOURCESTATE_FAILED;
    }
    shd->gl.prog = gl_prog;

    // resolve uniforms
    _SG_GL_CHECK_ERROR();
    for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
        const sg_shader_stage_desc* stage_desc = (stage_index == SG_SHADERSTAGE_VS)? &desc->vs : &desc->fs;
        const _sg_shader_stage_t* stage = &shd->cmn.stage[stage_index];
        _sg_gl_shader_stage_t* gl_stage = &shd->gl.stage[stage_index];
        for (int ub_index = 0; ub_index < stage->num_uniform_blocks; ub_index++) {
            const sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
            SOKOL_ASSERT(ub_desc->size > 0);
            _sg_gl_uniform_block_t* ub = &gl_stage->uniform_blocks[ub_index];
            SOKOL_ASSERT(ub->num_uniforms == 0);
            uint32_t cur_uniform_offset = 0;
            for (int u_index = 0; u_index < SG_MAX_UB_MEMBERS; u_index++) {
                const sg_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
                if (u_desc->type == SG_UNIFORMTYPE_INVALID) {
                    break;
                }
                const uint32_t u_align = _sg_uniform_alignment(u_desc->type, u_desc->array_count, ub_desc->layout);
                const uint32_t u_size = _sg_uniform_size(u_desc->type, u_desc->array_count, ub_desc->layout);
                cur_uniform_offset = _sg_align_u32(cur_uniform_offset, u_align);
                _sg_gl_uniform_t* u = &ub->uniforms[u_index];
                u->type = u_desc->type;
                u->count = (uint16_t) u_desc->array_count;
                u->offset = (uint16_t) cur_uniform_offset;
                cur_uniform_offset += u_size;
                if (u_desc->name) {
                    u->gl_loc = glGetUniformLocation(gl_prog, u_desc->name);
                } else {
                    u->gl_loc = u_index;
                }
                ub->num_uniforms++;
            }
            if (ub_desc->layout == SG_UNIFORMLAYOUT_STD140) {
                cur_uniform_offset = _sg_align_u32(cur_uniform_offset, 16);
            }
            SOKOL_ASSERT(ub_desc->size == (size_t)cur_uniform_offset);
            _SOKOL_UNUSED(cur_uniform_offset);
        }
    }

    // resolve combined image samplers
    _SG_GL_CHECK_ERROR();
    GLuint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&cur_prog);
    glUseProgram(gl_prog);
    int gl_tex_slot = 0;
    for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
        const sg_shader_stage_desc* stage_desc = (stage_index == SG_SHADERSTAGE_VS)? &desc->vs : &desc->fs;
        const _sg_shader_stage_t* stage = &shd->cmn.stage[stage_index];
        _sg_gl_shader_stage_t* gl_stage = &shd->gl.stage[stage_index];
        for (int img_smp_index = 0; img_smp_index < stage->num_image_samplers; img_smp_index++) {
            const sg_shader_image_sampler_pair_desc* img_smp_desc = &stage_desc->image_sampler_pairs[img_smp_index];
            _sg_gl_shader_image_sampler_t* gl_img_smp = &gl_stage->image_samplers[img_smp_index];
            SOKOL_ASSERT(img_smp_desc->glsl_name);
            GLint gl_loc = glGetUniformLocation(gl_prog, img_smp_desc->glsl_name);
            if (gl_loc != -1) {
                gl_img_smp->gl_tex_slot = gl_tex_slot++;
                glUniform1i(gl_loc, gl_img_smp->gl_tex_slot);
            } else {
                gl_img_smp->gl_tex_slot = -1;
                _SG_ERROR(GL_TEXTURE_NAME_NOT_FOUND_IN_SHADER);
                _SG_LOGMSG(GL_TEXTURE_NAME_NOT_FOUND_IN_SHADER, img_smp_desc->glsl_name);
            }
        }
    }
    // it's legal to call glUseProgram with 0
    glUseProgram(cur_prog);
    _SG_GL_CHECK_ERROR();
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_shader(_sg_shader_t* shd) {
    SOKOL_ASSERT(shd);
    _SG_GL_CHECK_ERROR();
    if (shd->gl.prog) {
        _sg_gl_cache_invalidate_program(shd->gl.prog);
        glDeleteProgram(shd->gl.prog);
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE sg_resource_state _sg_gl_create_pipeline(_sg_pipeline_t* pip, _sg_shader_t* shd, const sg_pipeline_desc* desc) {
    SOKOL_ASSERT(pip && shd && desc);
    SOKOL_ASSERT((pip->shader == 0) && (pip->cmn.shader_id.id != SG_INVALID_ID));
    SOKOL_ASSERT(desc->shader.id == shd->slot.id);
    SOKOL_ASSERT(shd->gl.prog);
    pip->shader = shd;
    pip->gl.primitive_type = desc->primitive_type;
    pip->gl.depth = desc->depth;
    pip->gl.stencil = desc->stencil;
    // FIXME: blend color and write mask per draw-buffer-attachment (requires GL4)
    pip->gl.blend = desc->colors[0].blend;
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        pip->gl.color_write_mask[i] = desc->colors[i].write_mask;
    }
    pip->gl.cull_mode = desc->cull_mode;
    pip->gl.face_winding = desc->face_winding;
    pip->gl.sample_count = desc->sample_count;
    pip->gl.alpha_to_coverage_enabled = desc->alpha_to_coverage_enabled;

    // resolve vertex attributes
    for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
        pip->gl.attrs[attr_index].vb_index = -1;
    }
    for (int attr_index = 0; attr_index < _sg.limits.max_vertex_attrs; attr_index++) {
        const sg_vertex_attr_state* a_state = &desc->layout.attrs[attr_index];
        if (a_state->format == SG_VERTEXFORMAT_INVALID) {
            break;
        }
        SOKOL_ASSERT(a_state->buffer_index < SG_MAX_VERTEX_BUFFERS);
        const sg_vertex_buffer_layout_state* l_state = &desc->layout.buffers[a_state->buffer_index];
        const sg_vertex_step step_func = l_state->step_func;
        const int step_rate = l_state->step_rate;
        GLint attr_loc = attr_index;
        if (!_sg_strempty(&shd->gl.attrs[attr_index].name)) {
            attr_loc = glGetAttribLocation(pip->shader->gl.prog, _sg_strptr(&shd->gl.attrs[attr_index].name));
        }
        SOKOL_ASSERT(attr_loc < (GLint)_sg.limits.max_vertex_attrs);
        if (attr_loc != -1) {
            _sg_gl_attr_t* gl_attr = &pip->gl.attrs[attr_loc];
            SOKOL_ASSERT(gl_attr->vb_index == -1);
            gl_attr->vb_index = (int8_t) a_state->buffer_index;
            if (step_func == SG_VERTEXSTEP_PER_VERTEX) {
                gl_attr->divisor = 0;
            } else {
                gl_attr->divisor = (int8_t) step_rate;
                pip->cmn.use_instanced_draw = true;
            }
            SOKOL_ASSERT(l_state->stride > 0);
            gl_attr->stride = (uint8_t) l_state->stride;
            gl_attr->offset = a_state->offset;
            gl_attr->size = (uint8_t) _sg_gl_vertexformat_size(a_state->format);
            gl_attr->type = _sg_gl_vertexformat_type(a_state->format);
            gl_attr->normalized = _sg_gl_vertexformat_normalized(a_state->format);
            pip->cmn.vertex_buffer_layout_active[a_state->buffer_index] = true;
        } else {
            _SG_ERROR(GL_VERTEX_ATTRIBUTE_NOT_FOUND_IN_SHADER);
            _SG_LOGMSG(GL_VERTEX_ATTRIBUTE_NOT_FOUND_IN_SHADER, _sg_strptr(&shd->gl.attrs[attr_index].name));
        }
    }
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_pipeline(_sg_pipeline_t* pip) {
    SOKOL_ASSERT(pip);
    _sg_gl_cache_invalidate_pipeline(pip);
}

_SOKOL_PRIVATE void _sg_gl_fb_attach_texture(const _sg_gl_attachment_t* gl_att, const _sg_attachment_common_t* cmn_att, GLenum gl_att_type) {
    const _sg_image_t* img = gl_att->image;
    SOKOL_ASSERT(img);
    const GLuint gl_tex = img->gl.tex[0];
    SOKOL_ASSERT(gl_tex);
    const GLuint gl_target = img->gl.target;
    SOKOL_ASSERT(gl_target);
    const int mip_level = cmn_att->mip_level;
    const int slice = cmn_att->slice;
    switch (img->cmn.type) {
        case SG_IMAGETYPE_2D:
            glFramebufferTexture2D(GL_FRAMEBUFFER, gl_att_type, gl_target, gl_tex, mip_level);
            break;
        case SG_IMAGETYPE_CUBE:
            glFramebufferTexture2D(GL_FRAMEBUFFER, gl_att_type, _sg_gl_cubeface_target(slice), gl_tex, mip_level);
            break;
        default:
            glFramebufferTextureLayer(GL_FRAMEBUFFER, gl_att_type, gl_tex, mip_level, slice);
            break;
    }
}

_SOKOL_PRIVATE GLenum _sg_gl_depth_stencil_attachment_type(const _sg_gl_attachment_t* ds_att) {
    const _sg_image_t* img = ds_att->image;
    SOKOL_ASSERT(img);
    if (_sg_is_depth_stencil_format(img->cmn.pixel_format)) {
        return GL_DEPTH_STENCIL_ATTACHMENT;
    } else {
        return GL_DEPTH_ATTACHMENT;
    }
}

_SOKOL_PRIVATE sg_resource_state _sg_gl_create_attachments(_sg_attachments_t* atts, _sg_image_t** color_images, _sg_image_t** resolve_images, _sg_image_t* ds_image, const sg_attachments_desc* desc) {
    SOKOL_ASSERT(atts && desc);
    SOKOL_ASSERT(color_images && resolve_images);
    _SG_GL_CHECK_ERROR();

    // copy image pointers
    for (int i = 0; i < atts->cmn.num_colors; i++) {
        const sg_attachment_desc* color_desc = &desc->colors[i];
        _SOKOL_UNUSED(color_desc);
        SOKOL_ASSERT(color_desc->image.id != SG_INVALID_ID);
        SOKOL_ASSERT(0 == atts->gl.colors[i].image);
        SOKOL_ASSERT(color_images[i] && (color_images[i]->slot.id == color_desc->image.id));
        SOKOL_ASSERT(_sg_is_valid_rendertarget_color_format(color_images[i]->cmn.pixel_format));
        atts->gl.colors[i].image = color_images[i];

        const sg_attachment_desc* resolve_desc = &desc->resolves[i];
        if (resolve_desc->image.id != SG_INVALID_ID) {
            SOKOL_ASSERT(0 == atts->gl.resolves[i].image);
            SOKOL_ASSERT(resolve_images[i] && (resolve_images[i]->slot.id == resolve_desc->image.id));
            SOKOL_ASSERT(color_images[i] && (color_images[i]->cmn.pixel_format == resolve_images[i]->cmn.pixel_format));
            atts->gl.resolves[i].image = resolve_images[i];
        }
    }
    SOKOL_ASSERT(0 == atts->gl.depth_stencil.image);
    const sg_attachment_desc* ds_desc = &desc->depth_stencil;
    if (ds_desc->image.id != SG_INVALID_ID) {
        SOKOL_ASSERT(ds_image && (ds_image->slot.id == ds_desc->image.id));
        SOKOL_ASSERT(_sg_is_valid_rendertarget_depth_format(ds_image->cmn.pixel_format));
        atts->gl.depth_stencil.image = ds_image;
    }

    // store current framebuffer binding (restored at end of function)
    GLuint gl_orig_fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&gl_orig_fb);

    // create a framebuffer object
    glGenFramebuffers(1, &atts->gl.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, atts->gl.fb);

    // attach color attachments to framebuffer
    for (int i = 0; i < atts->cmn.num_colors; i++) {
        const _sg_image_t* color_img = atts->gl.colors[i].image;
        SOKOL_ASSERT(color_img);
        const GLuint gl_msaa_render_buffer = color_img->gl.msaa_render_buffer;
        if (gl_msaa_render_buffer) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, (GLenum)(GL_COLOR_ATTACHMENT0+i), GL_RENDERBUFFER, gl_msaa_render_buffer);
        } else {
            const GLenum gl_att_type = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
            _sg_gl_fb_attach_texture(&atts->gl.colors[i], &atts->cmn.colors[i], gl_att_type);
        }
    }
    // attach depth-stencil attachment
    if (atts->gl.depth_stencil.image) {
        const GLenum gl_att = _sg_gl_depth_stencil_attachment_type(&atts->gl.depth_stencil);
        const _sg_image_t* ds_img = atts->gl.depth_stencil.image;
        const GLuint gl_msaa_render_buffer = ds_img->gl.msaa_render_buffer;
        if (gl_msaa_render_buffer) {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, gl_att, GL_RENDERBUFFER, gl_msaa_render_buffer);
        } else {
            const GLenum gl_att_type = _sg_gl_depth_stencil_attachment_type(&atts->gl.depth_stencil);
            _sg_gl_fb_attach_texture(&atts->gl.depth_stencil, &atts->cmn.depth_stencil, gl_att_type);
        }
    }

    // check if framebuffer is complete
    {
        const GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
            switch (fb_status) {
                case GL_FRAMEBUFFER_UNDEFINED:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNDEFINED);
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_ATTACHMENT);
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MISSING_ATTACHMENT);
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNSUPPORTED);
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MULTISAMPLE);
                    break;
                default:
                    _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNKNOWN);
                    break;
            }
            return SG_RESOURCESTATE_FAILED;
        }
    }

    // setup color attachments for the framebuffer
    static const GLenum gl_draw_bufs[SG_MAX_COLOR_ATTACHMENTS] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(atts->cmn.num_colors, gl_draw_bufs);

    // create MSAA resolve framebuffers if necessary
    for (int i = 0; i < atts->cmn.num_colors; i++) {
        _sg_gl_attachment_t* gl_resolve_att = &atts->gl.resolves[i];
        if (gl_resolve_att->image) {
            _sg_attachment_common_t* cmn_resolve_att = &atts->cmn.resolves[i];
            SOKOL_ASSERT(0 == atts->gl.msaa_resolve_framebuffer[i]);
            glGenFramebuffers(1, &atts->gl.msaa_resolve_framebuffer[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, atts->gl.msaa_resolve_framebuffer[i]);
            _sg_gl_fb_attach_texture(gl_resolve_att, cmn_resolve_att, GL_COLOR_ATTACHMENT0);
            // check if framebuffer is complete
            const GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
                switch (fb_status) {
                    case GL_FRAMEBUFFER_UNDEFINED:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNDEFINED);
                        break;
                    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_ATTACHMENT);
                        break;
                    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MISSING_ATTACHMENT);
                        break;
                    case GL_FRAMEBUFFER_UNSUPPORTED:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNSUPPORTED);
                        break;
                    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_INCOMPLETE_MULTISAMPLE);
                        break;
                    default:
                        _SG_ERROR(GL_FRAMEBUFFER_STATUS_UNKNOWN);
                        break;
                }
                return SG_RESOURCESTATE_FAILED;
            }
            // setup color attachments for the framebuffer
            glDrawBuffers(1, &gl_draw_bufs[0]);
        }
    }

    // restore original framebuffer binding
    glBindFramebuffer(GL_FRAMEBUFFER, gl_orig_fb);
    _SG_GL_CHECK_ERROR();
    return SG_RESOURCESTATE_VALID;
}

_SOKOL_PRIVATE void _sg_gl_discard_attachments(_sg_attachments_t* atts) {
    SOKOL_ASSERT(atts);
    _SG_GL_CHECK_ERROR();
    if (0 != atts->gl.fb) {
        glDeleteFramebuffers(1, &atts->gl.fb);
    }
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        if (atts->gl.msaa_resolve_framebuffer[i]) {
            glDeleteFramebuffers(1, &atts->gl.msaa_resolve_framebuffer[i]);
        }
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE _sg_image_t* _sg_gl_attachments_color_image(const _sg_attachments_t* atts, int index) {
    SOKOL_ASSERT(atts && (index >= 0) && (index < SG_MAX_COLOR_ATTACHMENTS));
    return atts->gl.colors[index].image;
}

_SOKOL_PRIVATE _sg_image_t* _sg_gl_attachments_resolve_image(const _sg_attachments_t* atts, int index) {
    SOKOL_ASSERT(atts && (index >= 0) && (index < SG_MAX_COLOR_ATTACHMENTS));
    return atts->gl.resolves[index].image;
}

_SOKOL_PRIVATE _sg_image_t* _sg_gl_attachments_ds_image(const _sg_attachments_t* atts) {
    SOKOL_ASSERT(atts);
    return atts->gl.depth_stencil.image;
}

_SOKOL_PRIVATE void _sg_gl_begin_pass(const sg_pass* pass) {
    // FIXME: what if a texture used as render target is still bound, should we
    // unbind all currently bound textures in begin pass?
    SOKOL_ASSERT(pass);
    _SG_GL_CHECK_ERROR();
    const _sg_attachments_t* atts = _sg.cur_pass.atts;
    const sg_swapchain* swapchain = &pass->swapchain;
    const sg_pass_action* action = &pass->action;

    // bind the render pass framebuffer
    //
    // FIXME: Disabling SRGB conversion for the default framebuffer is
    // a crude hack to make behaviour for sRGB render target textures
    // identical with the Metal and D3D11 swapchains created by sokol-app.
    //
    // This will need a cleaner solution (e.g. allowing to configure
    // sokol_app.h with an sRGB or RGB framebuffer.
    if (atts) {
        // offscreen pass
        SOKOL_ASSERT(atts->gl.fb);
        #if defined(SOKOL_GLCORE)
        glEnable(GL_FRAMEBUFFER_SRGB);
        #endif
        glBindFramebuffer(GL_FRAMEBUFFER, atts->gl.fb);
    } else {
        // default pass
        #if defined(SOKOL_GLCORE)
        glDisable(GL_FRAMEBUFFER_SRGB);
        #endif
        // NOTE: on some platforms, the default framebuffer of a context
        // is null, so we can't actually assert here that the
        // framebuffer has been provided
        glBindFramebuffer(GL_FRAMEBUFFER, swapchain->gl.framebuffer);
    }
    glViewport(0, 0, _sg.cur_pass.width, _sg.cur_pass.height);
    glScissor(0, 0, _sg.cur_pass.width, _sg.cur_pass.height);

    // number of color attachments
    const int num_color_atts = atts ? atts->cmn.num_colors : 1;

    // clear color and depth-stencil attachments if needed
    bool clear_any_color = false;
    for (int i = 0; i < num_color_atts; i++) {
        if (SG_LOADACTION_CLEAR == action->colors[i].load_action) {
            clear_any_color = true;
            break;
        }
    }
    const bool clear_depth = (action->depth.load_action == SG_LOADACTION_CLEAR);
    const bool clear_stencil = (action->stencil.load_action == SG_LOADACTION_CLEAR);

    bool need_pip_cache_flush = false;
    if (clear_any_color) {
        bool need_color_mask_flush = false;
        // NOTE: not a bug to iterate over all possible color attachments
        for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            if (SG_COLORMASK_RGBA != _sg.gl.cache.color_write_mask[i]) {
                need_pip_cache_flush = true;
                need_color_mask_flush = true;
                _sg.gl.cache.color_write_mask[i] = SG_COLORMASK_RGBA;
            }
        }
        if (need_color_mask_flush) {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }
    if (clear_depth) {
        if (!_sg.gl.cache.depth.write_enabled) {
            need_pip_cache_flush = true;
            _sg.gl.cache.depth.write_enabled = true;
            glDepthMask(GL_TRUE);
        }
        if (_sg.gl.cache.depth.compare != SG_COMPAREFUNC_ALWAYS) {
            need_pip_cache_flush = true;
            _sg.gl.cache.depth.compare = SG_COMPAREFUNC_ALWAYS;
            glDepthFunc(GL_ALWAYS);
        }
    }
    if (clear_stencil) {
        if (_sg.gl.cache.stencil.write_mask != 0xFF) {
            need_pip_cache_flush = true;
            _sg.gl.cache.stencil.write_mask = 0xFF;
            glStencilMask(0xFF);
        }
    }
    if (need_pip_cache_flush) {
        // we messed with the state cache directly, need to clear cached
        // pipeline to force re-evaluation in next sg_apply_pipeline()
        _sg.gl.cache.cur_pipeline = 0;
        _sg.gl.cache.cur_pipeline_id.id = SG_INVALID_ID;
    }
    for (int i = 0; i < num_color_atts; i++) {
        if (action->colors[i].load_action == SG_LOADACTION_CLEAR) {
            glClearBufferfv(GL_COLOR, i, &action->colors[i].clear_value.r);
        }
    }
    if ((atts == 0) || (atts->gl.depth_stencil.image)) {
        if (clear_depth && clear_stencil) {
            glClearBufferfi(GL_DEPTH_STENCIL, 0, action->depth.clear_value, action->stencil.clear_value);
        } else if (clear_depth) {
            glClearBufferfv(GL_DEPTH, 0, &action->depth.clear_value);
        } else if (clear_stencil) {
            GLint val = (GLint) action->stencil.clear_value;
            glClearBufferiv(GL_STENCIL, 0, &val);
        }
    }
    // keep store actions for end-pass
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        _sg.gl.color_store_actions[i] = action->colors[i].store_action;
    }
    _sg.gl.depth_store_action = action->depth.store_action;
    _sg.gl.stencil_store_action = action->stencil.store_action;

    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_end_pass(void) {
    _SG_GL_CHECK_ERROR();

    if (_sg.cur_pass.atts) {
        const _sg_attachments_t* atts = _sg.cur_pass.atts;
        SOKOL_ASSERT(atts->slot.id == _sg.cur_pass.atts_id.id);
        bool fb_read_bound = false;
        bool fb_draw_bound = false;
        const int num_color_atts = atts->cmn.num_colors;
        for (int i = 0; i < num_color_atts; i++) {
            // perform MSAA resolve if needed
            if (atts->gl.msaa_resolve_framebuffer[i] != 0) {
                if (!fb_read_bound) {
                    SOKOL_ASSERT(atts->gl.fb);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, atts->gl.fb);
                    fb_read_bound = true;
                }
                const int w = atts->gl.colors[i].image->cmn.width;
                const int h = atts->gl.colors[i].image->cmn.height;
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, atts->gl.msaa_resolve_framebuffer[i]);
                glReadBuffer((GLenum)(GL_COLOR_ATTACHMENT0 + i));
                glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                fb_draw_bound = true;
            }
        }

        // invalidate framebuffers
        _SOKOL_UNUSED(fb_draw_bound);
        #if defined(SOKOL_GLES3)
        // need to restore framebuffer binding before invalidate if the MSAA resolve had changed the binding
        if (fb_draw_bound) {
            glBindFramebuffer(GL_FRAMEBUFFER, atts->gl.fb);
        }
        GLenum invalidate_atts[SG_MAX_COLOR_ATTACHMENTS + 2] = { 0 };
        int att_index = 0;
        for (int i = 0; i < num_color_atts; i++) {
            if (_sg.gl.color_store_actions[i] == SG_STOREACTION_DONTCARE) {
                invalidate_atts[att_index++] = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
            }
        }
        if ((_sg.gl.depth_store_action == SG_STOREACTION_DONTCARE) && (_sg.cur_pass.atts->cmn.depth_stencil.image_id.id != SG_INVALID_ID)) {
            invalidate_atts[att_index++] = GL_DEPTH_ATTACHMENT;
        }
        if ((_sg.gl.stencil_store_action == SG_STOREACTION_DONTCARE) && (_sg.cur_pass.atts->cmn.depth_stencil.image_id.id != SG_INVALID_ID)) {
            invalidate_atts[att_index++] = GL_STENCIL_ATTACHMENT;
        }
        if (att_index > 0) {
            glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, att_index, invalidate_atts);
        }
        #endif
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_apply_viewport(int x, int y, int w, int h, bool origin_top_left) {
    y = origin_top_left ? (_sg.cur_pass.height - (y+h)) : y;
    glViewport(x, y, w, h);
}

_SOKOL_PRIVATE void _sg_gl_apply_scissor_rect(int x, int y, int w, int h, bool origin_top_left) {
    y = origin_top_left ? (_sg.cur_pass.height - (y+h)) : y;
    glScissor(x, y, w, h);
}

_SOKOL_PRIVATE void _sg_gl_apply_pipeline(_sg_pipeline_t* pip) {
    SOKOL_ASSERT(pip);
    SOKOL_ASSERT(pip->shader && (pip->cmn.shader_id.id == pip->shader->slot.id));
    _SG_GL_CHECK_ERROR();
    if ((_sg.gl.cache.cur_pipeline != pip) || (_sg.gl.cache.cur_pipeline_id.id != pip->slot.id)) {
        _sg.gl.cache.cur_pipeline = pip;
        _sg.gl.cache.cur_pipeline_id.id = pip->slot.id;
        _sg.gl.cache.cur_primitive_type = _sg_gl_primitive_type(pip->gl.primitive_type);
        _sg.gl.cache.cur_index_type = _sg_gl_index_type(pip->cmn.index_type);

        // update depth state
        {
            const sg_depth_state* state_ds = &pip->gl.depth;
            sg_depth_state* cache_ds = &_sg.gl.cache.depth;
            if (state_ds->compare != cache_ds->compare) {
                cache_ds->compare = state_ds->compare;
                glDepthFunc(_sg_gl_compare_func(state_ds->compare));
                _sg_stats_add(gl.num_render_state, 1);
            }
            if (state_ds->write_enabled != cache_ds->write_enabled) {
                cache_ds->write_enabled = state_ds->write_enabled;
                glDepthMask(state_ds->write_enabled);
                _sg_stats_add(gl.num_render_state, 1);
            }
            if (!_sg_fequal(state_ds->bias, cache_ds->bias, 0.000001f) ||
                !_sg_fequal(state_ds->bias_slope_scale, cache_ds->bias_slope_scale, 0.000001f))
            {
                /* according to ANGLE's D3D11 backend:
                    D3D11 SlopeScaledDepthBias ==> GL polygonOffsetFactor
                    D3D11 DepthBias ==> GL polygonOffsetUnits
                    DepthBiasClamp has no meaning on GL
                */
                cache_ds->bias = state_ds->bias;
                cache_ds->bias_slope_scale = state_ds->bias_slope_scale;
                glPolygonOffset(state_ds->bias_slope_scale, state_ds->bias);
                _sg_stats_add(gl.num_render_state, 1);
                bool po_enabled = true;
                if (_sg_fequal(state_ds->bias, 0.0f, 0.000001f) &&
                    _sg_fequal(state_ds->bias_slope_scale, 0.0f, 0.000001f))
                {
                    po_enabled = false;
                }
                if (po_enabled != _sg.gl.cache.polygon_offset_enabled) {
                    _sg.gl.cache.polygon_offset_enabled = po_enabled;
                    if (po_enabled) {
                        glEnable(GL_POLYGON_OFFSET_FILL);
                    } else {
                        glDisable(GL_POLYGON_OFFSET_FILL);
                    }
                    _sg_stats_add(gl.num_render_state, 1);
                }
            }
        }

        // update stencil state
        {
            const sg_stencil_state* state_ss = &pip->gl.stencil;
            sg_stencil_state* cache_ss = &_sg.gl.cache.stencil;
            if (state_ss->enabled != cache_ss->enabled) {
                cache_ss->enabled = state_ss->enabled;
                if (state_ss->enabled) {
                    glEnable(GL_STENCIL_TEST);
                } else {
                    glDisable(GL_STENCIL_TEST);
                }
                _sg_stats_add(gl.num_render_state, 1);
            }
            if (state_ss->write_mask != cache_ss->write_mask) {
                cache_ss->write_mask = state_ss->write_mask;
                glStencilMask(state_ss->write_mask);
                _sg_stats_add(gl.num_render_state, 1);
            }
            for (int i = 0; i < 2; i++) {
                const sg_stencil_face_state* state_sfs = (i==0)? &state_ss->front : &state_ss->back;
                sg_stencil_face_state* cache_sfs = (i==0)? &cache_ss->front : &cache_ss->back;
                GLenum gl_face = (i==0)? GL_FRONT : GL_BACK;
                if ((state_sfs->compare != cache_sfs->compare) ||
                    (state_ss->read_mask != cache_ss->read_mask) ||
                    (state_ss->ref != cache_ss->ref))
                {
                    cache_sfs->compare = state_sfs->compare;
                    glStencilFuncSeparate(gl_face,
                        _sg_gl_compare_func(state_sfs->compare),
                        state_ss->ref,
                        state_ss->read_mask);
                    _sg_stats_add(gl.num_render_state, 1);
                }
                if ((state_sfs->fail_op != cache_sfs->fail_op) ||
                    (state_sfs->depth_fail_op != cache_sfs->depth_fail_op) ||
                    (state_sfs->pass_op != cache_sfs->pass_op))
                {
                    cache_sfs->fail_op = state_sfs->fail_op;
                    cache_sfs->depth_fail_op = state_sfs->depth_fail_op;
                    cache_sfs->pass_op = state_sfs->pass_op;
                    glStencilOpSeparate(gl_face,
                        _sg_gl_stencil_op(state_sfs->fail_op),
                        _sg_gl_stencil_op(state_sfs->depth_fail_op),
                        _sg_gl_stencil_op(state_sfs->pass_op));
                    _sg_stats_add(gl.num_render_state, 1);
                }
            }
            cache_ss->read_mask = state_ss->read_mask;
            cache_ss->ref = state_ss->ref;
        }

        if (pip->cmn.color_count > 0) {
            // update blend state
            // FIXME: separate blend state per color attachment not support, needs GL4
            const sg_blend_state* state_bs = &pip->gl.blend;
            sg_blend_state* cache_bs = &_sg.gl.cache.blend;
            if (state_bs->enabled != cache_bs->enabled) {
                cache_bs->enabled = state_bs->enabled;
                if (state_bs->enabled) {
                    glEnable(GL_BLEND);
                } else {
                    glDisable(GL_BLEND);
                }
                _sg_stats_add(gl.num_render_state, 1);
            }
            if ((state_bs->src_factor_rgb != cache_bs->src_factor_rgb) ||
                (state_bs->dst_factor_rgb != cache_bs->dst_factor_rgb) ||
                (state_bs->src_factor_alpha != cache_bs->src_factor_alpha) ||
                (state_bs->dst_factor_alpha != cache_bs->dst_factor_alpha))
            {
                cache_bs->src_factor_rgb = state_bs->src_factor_rgb;
                cache_bs->dst_factor_rgb = state_bs->dst_factor_rgb;
                cache_bs->src_factor_alpha = state_bs->src_factor_alpha;
                cache_bs->dst_factor_alpha = state_bs->dst_factor_alpha;
                glBlendFuncSeparate(_sg_gl_blend_factor(state_bs->src_factor_rgb),
                    _sg_gl_blend_factor(state_bs->dst_factor_rgb),
                    _sg_gl_blend_factor(state_bs->src_factor_alpha),
                    _sg_gl_blend_factor(state_bs->dst_factor_alpha));
                _sg_stats_add(gl.num_render_state, 1);
            }
            if ((state_bs->op_rgb != cache_bs->op_rgb) || (state_bs->op_alpha != cache_bs->op_alpha)) {
                cache_bs->op_rgb = state_bs->op_rgb;
                cache_bs->op_alpha = state_bs->op_alpha;
                glBlendEquationSeparate(_sg_gl_blend_op(state_bs->op_rgb), _sg_gl_blend_op(state_bs->op_alpha));
                _sg_stats_add(gl.num_render_state, 1);
            }

            // standalone color target state
            for (GLuint i = 0; i < (GLuint)pip->cmn.color_count; i++) {
                if (pip->gl.color_write_mask[i] != _sg.gl.cache.color_write_mask[i]) {
                    const sg_color_mask cm = pip->gl.color_write_mask[i];
                    _sg.gl.cache.color_write_mask[i] = cm;
                    #ifdef SOKOL_GLCORE
                        glColorMaski(i,
                                    (cm & SG_COLORMASK_R) != 0,
                                    (cm & SG_COLORMASK_G) != 0,
                                    (cm & SG_COLORMASK_B) != 0,
                                    (cm & SG_COLORMASK_A) != 0);
                    #else
                        if (0 == i) {
                            glColorMask((cm & SG_COLORMASK_R) != 0,
                                        (cm & SG_COLORMASK_G) != 0,
                                        (cm & SG_COLORMASK_B) != 0,
                                        (cm & SG_COLORMASK_A) != 0);
                        }
                    #endif
                    _sg_stats_add(gl.num_render_state, 1);
                }
            }

            if (!_sg_fequal(pip->cmn.blend_color.r, _sg.gl.cache.blend_color.r, 0.0001f) ||
                !_sg_fequal(pip->cmn.blend_color.g, _sg.gl.cache.blend_color.g, 0.0001f) ||
                !_sg_fequal(pip->cmn.blend_color.b, _sg.gl.cache.blend_color.b, 0.0001f) ||
                !_sg_fequal(pip->cmn.blend_color.a, _sg.gl.cache.blend_color.a, 0.0001f))
            {
                sg_color c = pip->cmn.blend_color;
                _sg.gl.cache.blend_color = c;
                glBlendColor(c.r, c.g, c.b, c.a);
                _sg_stats_add(gl.num_render_state, 1);
            }
        } // pip->cmn.color_count > 0

        if (pip->gl.cull_mode != _sg.gl.cache.cull_mode) {
            _sg.gl.cache.cull_mode = pip->gl.cull_mode;
            if (SG_CULLMODE_NONE == pip->gl.cull_mode) {
                glDisable(GL_CULL_FACE);
                _sg_stats_add(gl.num_render_state, 1);
            } else {
                glEnable(GL_CULL_FACE);
                GLenum gl_mode = (SG_CULLMODE_FRONT == pip->gl.cull_mode) ? GL_FRONT : GL_BACK;
                glCullFace(gl_mode);
                _sg_stats_add(gl.num_render_state, 2);
            }
        }
        if (pip->gl.face_winding != _sg.gl.cache.face_winding) {
            _sg.gl.cache.face_winding = pip->gl.face_winding;
            GLenum gl_winding = (SG_FACEWINDING_CW == pip->gl.face_winding) ? GL_CW : GL_CCW;
            glFrontFace(gl_winding);
            _sg_stats_add(gl.num_render_state, 1);
        }
        if (pip->gl.alpha_to_coverage_enabled != _sg.gl.cache.alpha_to_coverage_enabled) {
            _sg.gl.cache.alpha_to_coverage_enabled = pip->gl.alpha_to_coverage_enabled;
            if (pip->gl.alpha_to_coverage_enabled) {
                glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
            } else {
                glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
            }
            _sg_stats_add(gl.num_render_state, 1);
        }
        #ifdef SOKOL_GLCORE
        if (pip->gl.sample_count != _sg.gl.cache.sample_count) {
            _sg.gl.cache.sample_count = pip->gl.sample_count;
            if (pip->gl.sample_count > 1) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
            _sg_stats_add(gl.num_render_state, 1);
        }
        #endif

        // bind shader program
        if (pip->shader->gl.prog != _sg.gl.cache.prog) {
            _sg.gl.cache.prog = pip->shader->gl.prog;
            glUseProgram(pip->shader->gl.prog);
            _sg_stats_add(gl.num_use_program, 1);
        }
    }
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE bool _sg_gl_apply_bindings(_sg_bindings_t* bnd) {
    SOKOL_ASSERT(bnd);
    SOKOL_ASSERT(bnd->pip);
    _SG_GL_CHECK_ERROR();

    // bind combined image-samplers
    _SG_GL_CHECK_ERROR();
    for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
        const _sg_shader_stage_t* stage = &bnd->pip->shader->cmn.stage[stage_index];
        const _sg_gl_shader_stage_t* gl_stage = &bnd->pip->shader->gl.stage[stage_index];
        _sg_image_t** imgs = (stage_index == SG_SHADERSTAGE_VS) ? bnd->vs_imgs : bnd->fs_imgs;
        _sg_sampler_t** smps = (stage_index == SG_SHADERSTAGE_VS) ? bnd->vs_smps : bnd->fs_smps;
        const int num_imgs = (stage_index == SG_SHADERSTAGE_VS) ? bnd->num_vs_imgs : bnd->num_fs_imgs;
        const int num_smps = (stage_index == SG_SHADERSTAGE_VS) ? bnd->num_vs_smps : bnd->num_fs_smps;
        SOKOL_ASSERT(num_imgs == stage->num_images); _SOKOL_UNUSED(num_imgs);
        SOKOL_ASSERT(num_smps == stage->num_samplers); _SOKOL_UNUSED(num_smps);
        for (int img_smp_index = 0; img_smp_index < stage->num_image_samplers; img_smp_index++) {
            const int gl_tex_slot = gl_stage->image_samplers[img_smp_index].gl_tex_slot;
            if (gl_tex_slot != -1) {
                const int img_index = stage->image_samplers[img_smp_index].image_slot;
                const int smp_index = stage->image_samplers[img_smp_index].sampler_slot;
                SOKOL_ASSERT(img_index < num_imgs);
                SOKOL_ASSERT(smp_index < num_smps);
                _sg_image_t* img = imgs[img_index];
                _sg_sampler_t* smp = smps[smp_index];
                const GLenum gl_tgt = img->gl.target;
                const GLuint gl_tex = img->gl.tex[img->cmn.active_slot];
                const GLuint gl_smp = smp->gl.smp;
                _sg_gl_cache_bind_texture_sampler(gl_tex_slot, gl_tgt, gl_tex, gl_smp);
            }
        }
    }
    _SG_GL_CHECK_ERROR();

    // bind storage buffers
    for (int slot = 0; slot < bnd->num_vs_sbufs; slot++) {
        _sg_buffer_t* sb = bnd->vs_sbufs[slot];
        GLuint gl_sb = sb->gl.buf[sb->cmn.active_slot];
        _sg_gl_cache_bind_storage_buffer(SG_SHADERSTAGE_VS, slot, gl_sb);
    }
    for (int slot = 0; slot < bnd->num_fs_sbufs; slot++) {
        _sg_buffer_t* sb = bnd->fs_sbufs[slot];
        GLuint gl_sb = sb->gl.buf[sb->cmn.active_slot];
        _sg_gl_cache_bind_storage_buffer(SG_SHADERSTAGE_FS, slot, gl_sb);
    }

    // index buffer (can be 0)
    const GLuint gl_ib = bnd->ib ? bnd->ib->gl.buf[bnd->ib->cmn.active_slot] : 0;
    _sg_gl_cache_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, gl_ib);
    _sg.gl.cache.cur_ib_offset = bnd->ib_offset;

    // vertex attributes
    for (GLuint attr_index = 0; attr_index < (GLuint)_sg.limits.max_vertex_attrs; attr_index++) {
        _sg_gl_attr_t* attr = &bnd->pip->gl.attrs[attr_index];
        _sg_gl_cache_attr_t* cache_attr = &_sg.gl.cache.attrs[attr_index];
        bool cache_attr_dirty = false;
        int vb_offset = 0;
        GLuint gl_vb = 0;
        if (attr->vb_index >= 0) {
            // attribute is enabled
            SOKOL_ASSERT(attr->vb_index < bnd->num_vbs);
            _sg_buffer_t* vb = bnd->vbs[attr->vb_index];
            SOKOL_ASSERT(vb);
            gl_vb = vb->gl.buf[vb->cmn.active_slot];
            vb_offset = bnd->vb_offsets[attr->vb_index] + attr->offset;
            if ((gl_vb != cache_attr->gl_vbuf) ||
                (attr->size != cache_attr->gl_attr.size) ||
                (attr->type != cache_attr->gl_attr.type) ||
                (attr->normalized != cache_attr->gl_attr.normalized) ||
                (attr->stride != cache_attr->gl_attr.stride) ||
                (vb_offset != cache_attr->gl_attr.offset) ||
                (cache_attr->gl_attr.divisor != attr->divisor))
            {
                _sg_gl_cache_bind_buffer(GL_ARRAY_BUFFER, gl_vb);
                if(attr->type == GL_UNSIGNED_INT)
                    glVertexAttribIPointer(attr_index, attr->size, attr->type, attr->stride, (const GLvoid*)(GLintptr)vb_offset);
                else
                    glVertexAttribPointer(attr_index, attr->size, attr->type, attr->normalized, attr->stride, (const GLvoid*)(GLintptr)vb_offset);
                _sg_stats_add(gl.num_vertex_attrib_pointer, 1);
                glVertexAttribDivisor(attr_index, (GLuint)attr->divisor);
                _sg_stats_add(gl.num_vertex_attrib_divisor, 1);
                cache_attr_dirty = true;
            }
            if (cache_attr->gl_attr.vb_index == -1) {
                glEnableVertexAttribArray(attr_index);
                _sg_stats_add(gl.num_enable_vertex_attrib_array, 1);
                cache_attr_dirty = true;
            }
        } else {
            // attribute is disabled
            if (cache_attr->gl_attr.vb_index != -1) {
                glDisableVertexAttribArray(attr_index);
                _sg_stats_add(gl.num_disable_vertex_attrib_array, 1);
                cache_attr_dirty = true;
            }
        }
        if (cache_attr_dirty) {
            cache_attr->gl_attr = *attr;
            cache_attr->gl_attr.offset = vb_offset;
            cache_attr->gl_vbuf = gl_vb;
        }
    }
    _SG_GL_CHECK_ERROR();
    return true;
}

_SOKOL_PRIVATE void _sg_gl_apply_uniforms(sg_shader_stage stage_index, int ub_index, const sg_range* data) {
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline);
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline->slot.id == _sg.gl.cache.cur_pipeline_id.id);
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline->shader->slot.id == _sg.gl.cache.cur_pipeline->cmn.shader_id.id);
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline->shader->cmn.stage[stage_index].num_uniform_blocks > ub_index);
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline->shader->cmn.stage[stage_index].uniform_blocks[ub_index].size == data->size);
    const _sg_gl_shader_stage_t* gl_stage = &_sg.gl.cache.cur_pipeline->shader->gl.stage[stage_index];
    const _sg_gl_uniform_block_t* gl_ub = &gl_stage->uniform_blocks[ub_index];
    for (int u_index = 0; u_index < gl_ub->num_uniforms; u_index++) {
        const _sg_gl_uniform_t* u = &gl_ub->uniforms[u_index];
        SOKOL_ASSERT(u->type != SG_UNIFORMTYPE_INVALID);
        if (u->gl_loc == -1) {
            continue;
        }
        _sg_stats_add(gl.num_uniform, 1);
        GLfloat* fptr = (GLfloat*) (((uint8_t*)data->ptr) + u->offset);
        GLint* iptr = (GLint*) (((uint8_t*)data->ptr) + u->offset);
        switch (u->type) {
            case SG_UNIFORMTYPE_INVALID:
                break;
            case SG_UNIFORMTYPE_FLOAT:
                glUniform1fv(u->gl_loc, u->count, fptr);
                break;
            case SG_UNIFORMTYPE_FLOAT2:
                glUniform2fv(u->gl_loc, u->count, fptr);
                break;
            case SG_UNIFORMTYPE_FLOAT3:
                glUniform3fv(u->gl_loc, u->count, fptr);
                break;
            case SG_UNIFORMTYPE_FLOAT4:
                glUniform4fv(u->gl_loc, u->count, fptr);
                break;
            case SG_UNIFORMTYPE_INT:
                glUniform1iv(u->gl_loc, u->count, iptr);
                break;
            case SG_UNIFORMTYPE_INT2:
                glUniform2iv(u->gl_loc, u->count, iptr);
                break;
            case SG_UNIFORMTYPE_INT3:
                glUniform3iv(u->gl_loc, u->count, iptr);
                break;
            case SG_UNIFORMTYPE_INT4:
                glUniform4iv(u->gl_loc, u->count, iptr);
                break;
            case SG_UNIFORMTYPE_MAT4:
                glUniformMatrix4fv(u->gl_loc, u->count, GL_FALSE, fptr);
                break;
            default:
                SOKOL_UNREACHABLE;
                break;
        }
    }
}

_SOKOL_PRIVATE void _sg_gl_draw(int base_element, int num_elements, int num_instances) {
    SOKOL_ASSERT(_sg.gl.cache.cur_pipeline);
    const GLenum i_type = _sg.gl.cache.cur_index_type;
    const GLenum p_type = _sg.gl.cache.cur_primitive_type;
    const bool use_instanced_draw = (num_instances > 1) || (_sg.gl.cache.cur_pipeline->cmn.use_instanced_draw);
    if (0 != i_type) {
        // indexed rendering
        const int i_size = (i_type == GL_UNSIGNED_SHORT) ? 2 : 4;
        const int ib_offset = _sg.gl.cache.cur_ib_offset;
        const GLvoid* indices = (const GLvoid*)(GLintptr)(base_element*i_size+ib_offset);
        if (use_instanced_draw) {
            glDrawElementsInstanced(p_type, num_elements, i_type, indices, num_instances);
        } else {
            glDrawElements(p_type, num_elements, i_type, indices);
        }
    } else {
        // non-indexed rendering
        if (use_instanced_draw) {
            glDrawArraysInstanced(p_type, base_element, num_elements, num_instances);
        } else {
            glDrawArrays(p_type, base_element, num_elements);
        }
    }
}

_SOKOL_PRIVATE void _sg_gl_commit(void) {
    // "soft" clear bindings (only those that are actually bound)
    _sg_gl_cache_clear_buffer_bindings(false);
    _sg_gl_cache_clear_texture_sampler_bindings(false);
}

_SOKOL_PRIVATE void _sg_gl_update_buffer(_sg_buffer_t* buf, const sg_range* data) {
    SOKOL_ASSERT(buf && data && data->ptr && (data->size > 0));
    // only one update per buffer per frame allowed
    if (++buf->cmn.active_slot >= buf->cmn.num_slots) {
        buf->cmn.active_slot = 0;
    }
    GLenum gl_tgt = _sg_gl_buffer_target(buf->cmn.type);
    SOKOL_ASSERT(buf->cmn.active_slot < SG_NUM_INFLIGHT_FRAMES);
    GLuint gl_buf = buf->gl.buf[buf->cmn.active_slot];
    SOKOL_ASSERT(gl_buf);
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_store_buffer_binding(gl_tgt);
    _sg_gl_cache_bind_buffer(gl_tgt, gl_buf);
    glBufferSubData(gl_tgt, 0, (GLsizeiptr)data->size, data->ptr);
    _sg_gl_cache_restore_buffer_binding(gl_tgt);
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_append_buffer(_sg_buffer_t* buf, const sg_range* data, bool new_frame) {
    SOKOL_ASSERT(buf && data && data->ptr && (data->size > 0));
    if (new_frame) {
        if (++buf->cmn.active_slot >= buf->cmn.num_slots) {
            buf->cmn.active_slot = 0;
        }
    }
    GLenum gl_tgt = _sg_gl_buffer_target(buf->cmn.type);
    SOKOL_ASSERT(buf->cmn.active_slot < SG_NUM_INFLIGHT_FRAMES);
    GLuint gl_buf = buf->gl.buf[buf->cmn.active_slot];
    SOKOL_ASSERT(gl_buf);
    _SG_GL_CHECK_ERROR();
    _sg_gl_cache_store_buffer_binding(gl_tgt);
    _sg_gl_cache_bind_buffer(gl_tgt, gl_buf);
    glBufferSubData(gl_tgt, buf->cmn.append_pos, (GLsizeiptr)data->size, data->ptr);
    _sg_gl_cache_restore_buffer_binding(gl_tgt);
    _SG_GL_CHECK_ERROR();
}

_SOKOL_PRIVATE void _sg_gl_update_image(_sg_image_t* img, const sg_image_data* data) {
    SOKOL_ASSERT(img && data);
    // only one update per image per frame allowed
    if (++img->cmn.active_slot >= img->cmn.num_slots) {
        img->cmn.active_slot = 0;
    }
    SOKOL_ASSERT(img->cmn.active_slot < SG_NUM_INFLIGHT_FRAMES);
    SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
    _sg_gl_cache_store_texture_sampler_binding(0);
    _sg_gl_cache_bind_texture_sampler(0, img->gl.target, img->gl.tex[img->cmn.active_slot], 0);
    const GLenum gl_img_format = _sg_gl_teximage_format(img->cmn.pixel_format);
    const GLenum gl_img_type = _sg_gl_teximage_type(img->cmn.pixel_format);
    const int num_faces = img->cmn.type == SG_IMAGETYPE_CUBE ? 6 : 1;
    const int num_mips = img->cmn.num_mipmaps;
    for (int face_index = 0; face_index < num_faces; face_index++) {
        for (int mip_index = 0; mip_index < num_mips; mip_index++) {
            GLenum gl_img_target = img->gl.target;
            if (SG_IMAGETYPE_CUBE == img->cmn.type) {
                gl_img_target = _sg_gl_cubeface_target(face_index);
            }
            const GLvoid* data_ptr = data->subimage[face_index][mip_index].ptr;
            int mip_width = _sg_miplevel_dim(img->cmn.width, mip_index);
            int mip_height = _sg_miplevel_dim(img->cmn.height, mip_index);
            if ((SG_IMAGETYPE_2D == img->cmn.type) || (SG_IMAGETYPE_CUBE == img->cmn.type)) {
                glTexSubImage2D(gl_img_target, mip_index,
                    0, 0,
                    mip_width, mip_height,
                    gl_img_format, gl_img_type,
                    data_ptr);
            } else if ((SG_IMAGETYPE_3D == img->cmn.type) || (SG_IMAGETYPE_ARRAY == img->cmn.type)) {
                int mip_depth = img->cmn.num_slices;
                if (SG_IMAGETYPE_3D == img->cmn.type) {
                    mip_depth = _sg_miplevel_dim(img->cmn.num_slices, mip_index);
                }
                glTexSubImage3D(gl_img_target, mip_index,
                    0, 0, 0,
                    mip_width, mip_height, mip_depth,
                    gl_img_format, gl_img_type,
                    data_ptr);

            }
        }
    }
    _sg_gl_cache_restore_texture_sampler_binding(0);
}


#endif

//  ██████  ███████ ███    ██ ███████ ██████  ██  ██████     ██████   █████   ██████ ██   ██ ███████ ███    ██ ██████
// ██       ██      ████   ██ ██      ██   ██ ██ ██          ██   ██ ██   ██ ██      ██  ██  ██      ████   ██ ██   ██
// ██   ███ █████   ██ ██  ██ █████   ██████  ██ ██          ██████  ███████ ██      █████   █████   ██ ██  ██ ██   ██
// ██    ██ ██      ██  ██ ██ ██      ██   ██ ██ ██          ██   ██ ██   ██ ██      ██  ██  ██      ██  ██ ██ ██   ██
//  ██████  ███████ ██   ████ ███████ ██   ██ ██  ██████     ██████  ██   ██  ██████ ██   ██ ███████ ██   ████ ██████
//
// >>generic backend
static inline void _sg_setup_backend(const sg_desc* desc) {
    _sg_gl_setup_backend(desc);
}

static inline void _sg_discard_backend(void) {
    _sg_gl_discard_backend();
}

static inline void _sg_reset_state_cache(void) {
    _sg_gl_reset_state_cache();
}

static inline sg_resource_state _sg_create_buffer(_sg_buffer_t* buf, const sg_buffer_desc* desc) {
    return _sg_gl_create_buffer(buf, desc);
}

static inline void _sg_discard_buffer(_sg_buffer_t* buf) {
    _sg_gl_discard_buffer(buf);
}

static inline sg_resource_state _sg_create_image(_sg_image_t* img, const sg_image_desc* desc) {
    return _sg_gl_create_image(img, desc);
}

static inline void _sg_discard_image(_sg_image_t* img) {
    _sg_gl_discard_image(img);
}

static inline sg_resource_state _sg_create_sampler(_sg_sampler_t* smp, const sg_sampler_desc* desc) {
    return _sg_gl_create_sampler(smp, desc);
}

static inline void _sg_discard_sampler(_sg_sampler_t* smp) {
    _sg_gl_discard_sampler(smp);
}

static inline sg_resource_state _sg_create_shader(_sg_shader_t* shd, const sg_shader_desc* desc) {
    return _sg_gl_create_shader(shd, desc);
}

static inline void _sg_discard_shader(_sg_shader_t* shd) {
    _sg_gl_discard_shader(shd);
}

static inline sg_resource_state _sg_create_pipeline(_sg_pipeline_t* pip, _sg_shader_t* shd, const sg_pipeline_desc* desc) {
    return _sg_gl_create_pipeline(pip, shd, desc);
}

static inline void _sg_discard_pipeline(_sg_pipeline_t* pip) {
    _sg_gl_discard_pipeline(pip);
}

static inline sg_resource_state _sg_create_attachments(_sg_attachments_t* atts, _sg_image_t** color_images, _sg_image_t** resolve_images, _sg_image_t* ds_image, const sg_attachments_desc* desc) {
    return _sg_gl_create_attachments(atts, color_images, resolve_images, ds_image, desc);
}

static inline void _sg_discard_attachments(_sg_attachments_t* atts) {
    _sg_gl_discard_attachments(atts);
}

static inline _sg_image_t* _sg_attachments_color_image(const _sg_attachments_t* atts, int index) {
    return _sg_gl_attachments_color_image(atts, index);
}

static inline _sg_image_t* _sg_attachments_resolve_image(const _sg_attachments_t* atts, int index) {
    return _sg_gl_attachments_resolve_image(atts, index);
}

static inline _sg_image_t* _sg_attachments_ds_image(const _sg_attachments_t* atts) {
    return _sg_gl_attachments_ds_image(atts);
}

static inline void _sg_begin_pass(const sg_pass* pass) {
    _sg_gl_begin_pass(pass);
}

static inline void _sg_end_pass(void) {
    _sg_gl_end_pass();
}

static inline void _sg_apply_viewport(int x, int y, int w, int h, bool origin_top_left) {
    _sg_gl_apply_viewport(x, y, w, h, origin_top_left);
}

static inline void _sg_apply_scissor_rect(int x, int y, int w, int h, bool origin_top_left) {
    _sg_gl_apply_scissor_rect(x, y, w, h, origin_top_left);
}

static inline void _sg_apply_pipeline(_sg_pipeline_t* pip) {
    _sg_gl_apply_pipeline(pip);
}

static inline bool _sg_apply_bindings(_sg_bindings_t* bnd) {
    return _sg_gl_apply_bindings(bnd);
}

static inline void _sg_apply_uniforms(sg_shader_stage stage_index, int ub_index, const sg_range* data) {
    _sg_gl_apply_uniforms(stage_index, ub_index, data);
}

static inline void _sg_draw(int base_element, int num_elements, int num_instances) {
    _sg_gl_draw(base_element, num_elements, num_instances);
}

static inline void _sg_commit(void) {
    _sg_gl_commit();
}

static inline void _sg_update_buffer(_sg_buffer_t* buf, const sg_range* data) {
    _sg_gl_update_buffer(buf, data);
}

static inline void _sg_append_buffer(_sg_buffer_t* buf, const sg_range* data, bool new_frame) {
    _sg_gl_append_buffer(buf, data, new_frame);
}

static inline void _sg_update_image(_sg_image_t* img, const sg_image_data* data) {
    _sg_gl_update_image(img, data);
}

static inline void _sg_push_debug_group(const char* name) {
    _SOKOL_UNUSED(name);
}

static inline void _sg_pop_debug_group(void) {
}

// ██████   ██████   ██████  ██
// ██   ██ ██    ██ ██    ██ ██
// ██████  ██    ██ ██    ██ ██
// ██      ██    ██ ██    ██ ██
// ██       ██████   ██████  ███████
//
// >>pool
_SOKOL_PRIVATE void _sg_init_pool(_sg_pool_t* pool, int num) {
    SOKOL_ASSERT(pool && (num >= 1));
    // slot 0 is reserved for the 'invalid id', so bump the pool size by 1
    pool->size = num + 1;
    pool->queue_top = 0;
    // generation counters indexable by pool slot index, slot 0 is reserved
    size_t gen_ctrs_size = sizeof(uint32_t) * (size_t)pool->size;
    pool->gen_ctrs = (uint32_t*)_sg_malloc_clear(gen_ctrs_size);
    // it's not a bug to only reserve 'num' here
    pool->free_queue = (int*) _sg_malloc_clear(sizeof(int) * (size_t)num);
    // never allocate the zero-th pool item since the invalid id is 0
    for (int i = pool->size-1; i >= 1; i--) {
        pool->free_queue[pool->queue_top++] = i;
    }
}

_SOKOL_PRIVATE void _sg_discard_pool(_sg_pool_t* pool) {
    SOKOL_ASSERT(pool);
    SOKOL_ASSERT(pool->free_queue);
    _sg_free(pool->free_queue);
    pool->free_queue = 0;
    SOKOL_ASSERT(pool->gen_ctrs);
    _sg_free(pool->gen_ctrs);
    pool->gen_ctrs = 0;
    pool->size = 0;
    pool->queue_top = 0;
}

_SOKOL_PRIVATE int _sg_pool_alloc_index(_sg_pool_t* pool) {
    SOKOL_ASSERT(pool);
    SOKOL_ASSERT(pool->free_queue);
    if (pool->queue_top > 0) {
        int slot_index = pool->free_queue[--pool->queue_top];
        SOKOL_ASSERT((slot_index > 0) && (slot_index < pool->size));
        return slot_index;
    } else {
        // pool exhausted
        return _SG_INVALID_SLOT_INDEX;
    }
}

_SOKOL_PRIVATE void _sg_pool_free_index(_sg_pool_t* pool, int slot_index) {
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < pool->size));
    SOKOL_ASSERT(pool);
    SOKOL_ASSERT(pool->free_queue);
    SOKOL_ASSERT(pool->queue_top < pool->size);
    #ifdef SOKOL_DEBUG
    // debug check against double-free
    for (int i = 0; i < pool->queue_top; i++) {
        SOKOL_ASSERT(pool->free_queue[i] != slot_index);
    }
    #endif
    pool->free_queue[pool->queue_top++] = slot_index;
    SOKOL_ASSERT(pool->queue_top <= (pool->size-1));
}

_SOKOL_PRIVATE void _sg_reset_slot(_sg_slot_t* slot) {
    SOKOL_ASSERT(slot);
    _sg_clear(slot, sizeof(_sg_slot_t));
}

_SOKOL_PRIVATE void _sg_reset_buffer_to_alloc_state(_sg_buffer_t* buf) {
    SOKOL_ASSERT(buf);
    _sg_slot_t slot = buf->slot;
    _sg_clear(buf, sizeof(*buf));
    buf->slot = slot;
    buf->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_reset_image_to_alloc_state(_sg_image_t* img) {
    SOKOL_ASSERT(img);
    _sg_slot_t slot = img->slot;
    _sg_clear(img, sizeof(*img));
    img->slot = slot;
    img->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_reset_sampler_to_alloc_state(_sg_sampler_t* smp) {
    SOKOL_ASSERT(smp);
    _sg_slot_t slot = smp->slot;
    _sg_clear(smp, sizeof(*smp));
    smp->slot = slot;
    smp->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_reset_shader_to_alloc_state(_sg_shader_t* shd) {
    SOKOL_ASSERT(shd);
    _sg_slot_t slot = shd->slot;
    _sg_clear(shd, sizeof(*shd));
    shd->slot = slot;
    shd->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_reset_pipeline_to_alloc_state(_sg_pipeline_t* pip) {
    SOKOL_ASSERT(pip);
    _sg_slot_t slot = pip->slot;
    _sg_clear(pip, sizeof(*pip));
    pip->slot = slot;
    pip->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_reset_attachments_to_alloc_state(_sg_attachments_t* atts) {
    SOKOL_ASSERT(atts);
    _sg_slot_t slot = atts->slot;
    _sg_clear(atts, sizeof(*atts));
    atts->slot = slot;
    atts->slot.state = SG_RESOURCESTATE_ALLOC;
}

_SOKOL_PRIVATE void _sg_setup_pools(_sg_pools_t* p, const sg_desc* desc) {
    SOKOL_ASSERT(p);
    SOKOL_ASSERT(desc);
    // note: the pools here will have an additional item, since slot 0 is reserved
    SOKOL_ASSERT((desc->buffer_pool_size > 0) && (desc->buffer_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->buffer_pool, desc->buffer_pool_size);
    size_t buffer_pool_byte_size = sizeof(_sg_buffer_t) * (size_t)p->buffer_pool.size;
    p->buffers = (_sg_buffer_t*) _sg_malloc_clear(buffer_pool_byte_size);

    SOKOL_ASSERT((desc->image_pool_size > 0) && (desc->image_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->image_pool, desc->image_pool_size);
    size_t image_pool_byte_size = sizeof(_sg_image_t) * (size_t)p->image_pool.size;
    p->images = (_sg_image_t*) _sg_malloc_clear(image_pool_byte_size);

    SOKOL_ASSERT((desc->sampler_pool_size > 0) && (desc->sampler_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->sampler_pool, desc->sampler_pool_size);
    size_t sampler_pool_byte_size = sizeof(_sg_sampler_t) * (size_t)p->sampler_pool.size;
    p->samplers = (_sg_sampler_t*) _sg_malloc_clear(sampler_pool_byte_size);

    SOKOL_ASSERT((desc->shader_pool_size > 0) && (desc->shader_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->shader_pool, desc->shader_pool_size);
    size_t shader_pool_byte_size = sizeof(_sg_shader_t) * (size_t)p->shader_pool.size;
    p->shaders = (_sg_shader_t*) _sg_malloc_clear(shader_pool_byte_size);

    SOKOL_ASSERT((desc->pipeline_pool_size > 0) && (desc->pipeline_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->pipeline_pool, desc->pipeline_pool_size);
    size_t pipeline_pool_byte_size = sizeof(_sg_pipeline_t) * (size_t)p->pipeline_pool.size;
    p->pipelines = (_sg_pipeline_t*) _sg_malloc_clear(pipeline_pool_byte_size);

    SOKOL_ASSERT((desc->attachments_pool_size > 0) && (desc->attachments_pool_size < _SG_MAX_POOL_SIZE));
    _sg_init_pool(&p->attachments_pool, desc->attachments_pool_size);
    size_t attachments_pool_byte_size = sizeof(_sg_attachments_t) * (size_t)p->attachments_pool.size;
    p->attachments = (_sg_attachments_t*) _sg_malloc_clear(attachments_pool_byte_size);
}

_SOKOL_PRIVATE void _sg_discard_pools(_sg_pools_t* p) {
    SOKOL_ASSERT(p);
    _sg_free(p->attachments); p->attachments = 0;
    _sg_free(p->pipelines);   p->pipelines = 0;
    _sg_free(p->shaders);     p->shaders = 0;
    _sg_free(p->samplers);    p->samplers = 0;
    _sg_free(p->images);      p->images = 0;
    _sg_free(p->buffers);     p->buffers = 0;
    _sg_discard_pool(&p->attachments_pool);
    _sg_discard_pool(&p->pipeline_pool);
    _sg_discard_pool(&p->shader_pool);
    _sg_discard_pool(&p->sampler_pool);
    _sg_discard_pool(&p->image_pool);
    _sg_discard_pool(&p->buffer_pool);
}

/* allocate the slot at slot_index:
    - bump the slot's generation counter
    - create a resource id from the generation counter and slot index
    - set the slot's id to this id
    - set the slot's state to ALLOC
    - return the resource id
*/
_SOKOL_PRIVATE uint32_t _sg_slot_alloc(_sg_pool_t* pool, _sg_slot_t* slot, int slot_index) {
    /* FIXME: add handling for an overflowing generation counter,
       for now, just overflow (another option is to disable
       the slot)
    */
    SOKOL_ASSERT(pool && pool->gen_ctrs);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < pool->size));
    SOKOL_ASSERT(slot->id == SG_INVALID_ID);
    SOKOL_ASSERT(slot->state == SG_RESOURCESTATE_INITIAL);
    uint32_t ctr = ++pool->gen_ctrs[slot_index];
    slot->id = (ctr<<_SG_SLOT_SHIFT)|(slot_index & _SG_SLOT_MASK);
    slot->state = SG_RESOURCESTATE_ALLOC;
    return slot->id;
}

// extract slot index from id
_SOKOL_PRIVATE int _sg_slot_index(uint32_t id) {
    int slot_index = (int) (id & _SG_SLOT_MASK);
    SOKOL_ASSERT(_SG_INVALID_SLOT_INDEX != slot_index);
    return slot_index;
}

// returns pointer to resource by id without matching id check
_SOKOL_PRIVATE _sg_buffer_t* _sg_buffer_at(const _sg_pools_t* p, uint32_t buf_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != buf_id));
    int slot_index = _sg_slot_index(buf_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->buffer_pool.size));
    return &p->buffers[slot_index];
}

_SOKOL_PRIVATE _sg_image_t* _sg_image_at(const _sg_pools_t* p, uint32_t img_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != img_id));
    int slot_index = _sg_slot_index(img_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->image_pool.size));
    return &p->images[slot_index];
}

_SOKOL_PRIVATE _sg_sampler_t* _sg_sampler_at(const _sg_pools_t* p, uint32_t smp_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != smp_id));
    int slot_index = _sg_slot_index(smp_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->sampler_pool.size));
    return &p->samplers[slot_index];
}

_SOKOL_PRIVATE _sg_shader_t* _sg_shader_at(const _sg_pools_t* p, uint32_t shd_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != shd_id));
    int slot_index = _sg_slot_index(shd_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->shader_pool.size));
    return &p->shaders[slot_index];
}

_SOKOL_PRIVATE _sg_pipeline_t* _sg_pipeline_at(const _sg_pools_t* p, uint32_t pip_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != pip_id));
    int slot_index = _sg_slot_index(pip_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->pipeline_pool.size));
    return &p->pipelines[slot_index];
}

_SOKOL_PRIVATE _sg_attachments_t* _sg_attachments_at(const _sg_pools_t* p, uint32_t atts_id) {
    SOKOL_ASSERT(p && (SG_INVALID_ID != atts_id));
    int slot_index = _sg_slot_index(atts_id);
    SOKOL_ASSERT((slot_index > _SG_INVALID_SLOT_INDEX) && (slot_index < p->attachments_pool.size));
    return &p->attachments[slot_index];
}

// returns pointer to resource with matching id check, may return 0
_SOKOL_PRIVATE _sg_buffer_t* _sg_lookup_buffer(const _sg_pools_t* p, uint32_t buf_id) {
    if (SG_INVALID_ID != buf_id) {
        _sg_buffer_t* buf = _sg_buffer_at(p, buf_id);
        if (buf->slot.id == buf_id) {
            return buf;
        }
    }
    return 0;
}

_SOKOL_PRIVATE _sg_image_t* _sg_lookup_image(const _sg_pools_t* p, uint32_t img_id) {
    if (SG_INVALID_ID != img_id) {
        _sg_image_t* img = _sg_image_at(p, img_id);
        if (img->slot.id == img_id) {
            return img;
        }
    }
    return 0;
}

_SOKOL_PRIVATE _sg_sampler_t* _sg_lookup_sampler(const _sg_pools_t* p, uint32_t smp_id) {
    if (SG_INVALID_ID != smp_id) {
        _sg_sampler_t* smp = _sg_sampler_at(p, smp_id);
        if (smp->slot.id == smp_id) {
            return smp;
        }
    }
    return 0;
}

_SOKOL_PRIVATE _sg_shader_t* _sg_lookup_shader(const _sg_pools_t* p, uint32_t shd_id) {
    SOKOL_ASSERT(p);
    if (SG_INVALID_ID != shd_id) {
        _sg_shader_t* shd = _sg_shader_at(p, shd_id);
        if (shd->slot.id == shd_id) {
            return shd;
        }
    }
    return 0;
}

_SOKOL_PRIVATE _sg_pipeline_t* _sg_lookup_pipeline(const _sg_pools_t* p, uint32_t pip_id) {
    SOKOL_ASSERT(p);
    if (SG_INVALID_ID != pip_id) {
        _sg_pipeline_t* pip = _sg_pipeline_at(p, pip_id);
        if (pip->slot.id == pip_id) {
            return pip;
        }
    }
    return 0;
}

_SOKOL_PRIVATE _sg_attachments_t* _sg_lookup_attachments(const _sg_pools_t* p, uint32_t atts_id) {
    SOKOL_ASSERT(p);
    if (SG_INVALID_ID != atts_id) {
        _sg_attachments_t* atts = _sg_attachments_at(p, atts_id);
        if (atts->slot.id == atts_id) {
            return atts;
        }
    }
    return 0;
}

_SOKOL_PRIVATE void _sg_discard_all_resources(_sg_pools_t* p) {
    /*  this is a bit dumb since it loops over all pool slots to
        find the occupied slots, on the other hand it is only ever
        executed at shutdown
        NOTE: ONLY EXECUTE THIS AT SHUTDOWN
              ...because the free queues will not be reset
              and the resource slots not be cleared!
    */
    for (int i = 1; i < p->buffer_pool.size; i++) {
        sg_resource_state state = p->buffers[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_buffer(&p->buffers[i]);
        }
    }
    for (int i = 1; i < p->image_pool.size; i++) {
        sg_resource_state state = p->images[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_image(&p->images[i]);
        }
    }
    for (int i = 1; i < p->sampler_pool.size; i++) {
        sg_resource_state state = p->samplers[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_sampler(&p->samplers[i]);
        }
    }
    for (int i = 1; i < p->shader_pool.size; i++) {
        sg_resource_state state = p->shaders[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_shader(&p->shaders[i]);
        }
    }
    for (int i = 1; i < p->pipeline_pool.size; i++) {
        sg_resource_state state = p->pipelines[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_pipeline(&p->pipelines[i]);
        }
    }
    for (int i = 1; i < p->attachments_pool.size; i++) {
        sg_resource_state state = p->attachments[i].slot.state;
        if ((state == SG_RESOURCESTATE_VALID) || (state == SG_RESOURCESTATE_FAILED)) {
            _sg_discard_attachments(&p->attachments[i]);
        }
    }
}

// ██    ██  █████  ██      ██ ██████   █████  ████████ ██  ██████  ███    ██
// ██    ██ ██   ██ ██      ██ ██   ██ ██   ██    ██    ██ ██    ██ ████   ██
// ██    ██ ███████ ██      ██ ██   ██ ███████    ██    ██ ██    ██ ██ ██  ██
//  ██  ██  ██   ██ ██      ██ ██   ██ ██   ██    ██    ██ ██    ██ ██  ██ ██
//   ████   ██   ██ ███████ ██ ██████  ██   ██    ██    ██  ██████  ██   ████
//
// >>validation
#if defined(SOKOL_DEBUG)
_SOKOL_PRIVATE void _sg_validate_begin(void) {
    _sg.validate_error = SG_LOGITEM_OK;
}

_SOKOL_PRIVATE bool _sg_validate_end(void) {
    if (_sg.validate_error != SG_LOGITEM_OK) {
        #if !defined(SOKOL_VALIDATE_NON_FATAL)
            _SG_PANIC(VALIDATION_FAILED);
            return false;
        #else
            return false;
        #endif
    } else {
        return true;
    }
}
#endif

_SOKOL_PRIVATE bool _sg_validate_buffer_desc(const sg_buffer_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_BUFFERDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_BUFFERDESC_CANARY);
        _SG_VALIDATE(desc->size > 0, VALIDATE_BUFFERDESC_SIZE);
        bool injected = (0 != desc->gl_buffers[0]);
        if (!injected && (desc->usage == SG_USAGE_IMMUTABLE)) {
            _SG_VALIDATE((0 != desc->data.ptr) && (desc->data.size > 0), VALIDATE_BUFFERDESC_DATA);
            _SG_VALIDATE(desc->size == desc->data.size, VALIDATE_BUFFERDESC_DATA_SIZE);
        } else {
            _SG_VALIDATE(0 == desc->data.ptr, VALIDATE_BUFFERDESC_NO_DATA);
        }
        if (desc->type == SG_BUFFERTYPE_STORAGEBUFFER) {
            _SG_VALIDATE(_sg.features.storage_buffer, VALIDATE_BUFFERDESC_STORAGEBUFFER_SUPPORTED);
            _SG_VALIDATE(_sg_multiple_u64(desc->size, 4), VALIDATE_BUFFERDESC_STORAGEBUFFER_SIZE_MULTIPLE_4);
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE void _sg_validate_image_data(const sg_image_data* data, sg_pixel_format fmt, int width, int height, int num_faces, int num_mips, int num_slices) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(data);
        _SOKOL_UNUSED(fmt);
        _SOKOL_UNUSED(width);
        _SOKOL_UNUSED(height);
        _SOKOL_UNUSED(num_faces);
        _SOKOL_UNUSED(num_mips);
        _SOKOL_UNUSED(num_slices);
    #else
        for (int face_index = 0; face_index < num_faces; face_index++) {
            for (int mip_index = 0; mip_index < num_mips; mip_index++) {
                const bool has_data = data->subimage[face_index][mip_index].ptr != 0;
                const bool has_size = data->subimage[face_index][mip_index].size > 0;
                _SG_VALIDATE(has_data && has_size, VALIDATE_IMAGEDATA_NODATA);
                const int mip_width = _sg_miplevel_dim(width, mip_index);
                const int mip_height = _sg_miplevel_dim(height, mip_index);
                const int bytes_per_slice = _sg_surface_pitch(fmt, mip_width, mip_height, 1);
                const int expected_size = bytes_per_slice * num_slices;
                _SG_VALIDATE(expected_size == (int)data->subimage[face_index][mip_index].size, VALIDATE_IMAGEDATA_DATA_SIZE);
            }
        }
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_image_desc(const sg_image_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_IMAGEDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_IMAGEDESC_CANARY);
        _SG_VALIDATE(desc->width > 0, VALIDATE_IMAGEDESC_WIDTH);
        _SG_VALIDATE(desc->height > 0, VALIDATE_IMAGEDESC_HEIGHT);
        const sg_pixel_format fmt = desc->pixel_format;
        const sg_usage usage = desc->usage;
        const bool injected = (0 != desc->gl_textures[0]);
        if (_sg_is_depth_or_depth_stencil_format(fmt)) {
            _SG_VALIDATE(desc->type != SG_IMAGETYPE_3D, VALIDATE_IMAGEDESC_DEPTH_3D_IMAGE);
        }
        if (desc->render_target) {
            SOKOL_ASSERT(((int)fmt >= 0) && ((int)fmt < _SG_PIXELFORMAT_NUM));
            _SG_VALIDATE(_sg.formats[fmt].render, VALIDATE_IMAGEDESC_RT_PIXELFORMAT);
            _SG_VALIDATE(usage == SG_USAGE_IMMUTABLE, VALIDATE_IMAGEDESC_RT_IMMUTABLE);
            _SG_VALIDATE(desc->data.subimage[0][0].ptr==0, VALIDATE_IMAGEDESC_RT_NO_DATA);
            if (desc->sample_count > 1) {
                _SG_VALIDATE(_sg.formats[fmt].msaa, VALIDATE_IMAGEDESC_NO_MSAA_RT_SUPPORT);
                _SG_VALIDATE(desc->num_mipmaps == 1, VALIDATE_IMAGEDESC_MSAA_NUM_MIPMAPS);
                _SG_VALIDATE(desc->type != SG_IMAGETYPE_3D, VALIDATE_IMAGEDESC_MSAA_3D_IMAGE);
            }
        } else {
            _SG_VALIDATE(desc->sample_count == 1, VALIDATE_IMAGEDESC_MSAA_BUT_NO_RT);
            const bool valid_nonrt_fmt = !_sg_is_valid_rendertarget_depth_format(fmt);
            _SG_VALIDATE(valid_nonrt_fmt, VALIDATE_IMAGEDESC_NONRT_PIXELFORMAT);
            const bool is_compressed = _sg_is_compressed_pixel_format(desc->pixel_format);
            const bool is_immutable = (usage == SG_USAGE_IMMUTABLE);
            if (is_compressed) {
                _SG_VALIDATE(is_immutable, VALIDATE_IMAGEDESC_COMPRESSED_IMMUTABLE);
            }
            if (!injected && is_immutable) {
                // image desc must have valid data
                _sg_validate_image_data(&desc->data,
                    desc->pixel_format,
                    desc->width,
                    desc->height,
                    (desc->type == SG_IMAGETYPE_CUBE) ? 6 : 1,
                    desc->num_mipmaps,
                    desc->num_slices);
            } else {
                // image desc must not have data
                for (int face_index = 0; face_index < SG_CUBEFACE_NUM; face_index++) {
                    for (int mip_index = 0; mip_index < SG_MAX_MIPMAPS; mip_index++) {
                        const bool no_data = 0 == desc->data.subimage[face_index][mip_index].ptr;
                        const bool no_size = 0 == desc->data.subimage[face_index][mip_index].size;
                        if (injected) {
                            _SG_VALIDATE(no_data && no_size, VALIDATE_IMAGEDESC_INJECTED_NO_DATA);
                        }
                        if (!is_immutable) {
                            _SG_VALIDATE(no_data && no_size, VALIDATE_IMAGEDESC_DYNAMIC_NO_DATA);
                        }
                    }
                }
            }
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_sampler_desc(const sg_sampler_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_SAMPLERDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_SAMPLERDESC_CANARY);
        _SG_VALIDATE(desc->min_filter != SG_FILTER_NONE, VALIDATE_SAMPLERDESC_MINFILTER_NONE);
        _SG_VALIDATE(desc->mag_filter != SG_FILTER_NONE, VALIDATE_SAMPLERDESC_MAGFILTER_NONE);
        // restriction from WebGPU: when anisotropy > 1, all filters must be linear
        if (desc->max_anisotropy > 1) {
            _SG_VALIDATE((desc->min_filter == SG_FILTER_LINEAR)
                      && (desc->mag_filter == SG_FILTER_LINEAR)
                      && (desc->mipmap_filter == SG_FILTER_LINEAR),
                      VALIDATE_SAMPLERDESC_ANISTROPIC_REQUIRES_LINEAR_FILTERING);
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_shader_desc(const sg_shader_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_SHADERDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_SHADERDESC_CANARY);
        #if defined(SOKOL_GLCORE) || defined(SOKOL_GLES3) || defined(SOKOL_WGPU)
            // on GL or WebGPU, must provide shader source code
            _SG_VALIDATE(0 != desc->vs.source, VALIDATE_SHADERDESC_SOURCE);
            _SG_VALIDATE(0 != desc->fs.source, VALIDATE_SHADERDESC_SOURCE);
        #elif defined(SOKOL_METAL) || defined(SOKOL_D3D11)
            // on Metal or D3D11, must provide shader source code or byte code
            _SG_VALIDATE((0 != desc->vs.source)||(0 != desc->vs.bytecode.ptr), VALIDATE_SHADERDESC_SOURCE_OR_BYTECODE);
            _SG_VALIDATE((0 != desc->fs.source)||(0 != desc->fs.bytecode.ptr), VALIDATE_SHADERDESC_SOURCE_OR_BYTECODE);
        #else
            // Dummy Backend, don't require source or bytecode
        #endif
        for (int i = 0; i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
            if (desc->attrs[i].name) {
                _SG_VALIDATE(strlen(desc->attrs[i].name) < _SG_STRING_SIZE, VALIDATE_SHADERDESC_ATTR_STRING_TOO_LONG);
            }
            if (desc->attrs[i].sem_name) {
                _SG_VALIDATE(strlen(desc->attrs[i].sem_name) < _SG_STRING_SIZE, VALIDATE_SHADERDESC_ATTR_STRING_TOO_LONG);
            }
        }
        // if shader byte code, the size must also be provided
        if (0 != desc->vs.bytecode.ptr) {
            _SG_VALIDATE(desc->vs.bytecode.size > 0, VALIDATE_SHADERDESC_NO_BYTECODE_SIZE);
        }
        if (0 != desc->fs.bytecode.ptr) {
            _SG_VALIDATE(desc->fs.bytecode.size > 0, VALIDATE_SHADERDESC_NO_BYTECODE_SIZE);
        }
        for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
            const sg_shader_stage_desc* stage_desc = (stage_index == 0)? &desc->vs : &desc->fs;
            bool uniform_blocks_continuous = true;
            for (int ub_index = 0; ub_index < SG_MAX_SHADERSTAGE_UBS; ub_index++) {
                const sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
                if (ub_desc->size > 0) {
                    _SG_VALIDATE(uniform_blocks_continuous, VALIDATE_SHADERDESC_NO_CONT_UBS);
                    #if defined(_SOKOL_ANY_GL)
                    bool uniforms_continuous = true;
                    uint32_t uniform_offset = 0;
                    int num_uniforms = 0;
                    for (int u_index = 0; u_index < SG_MAX_UB_MEMBERS; u_index++) {
                        const sg_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
                        if (u_desc->type != SG_UNIFORMTYPE_INVALID) {
                            _SG_VALIDATE(uniforms_continuous, VALIDATE_SHADERDESC_NO_CONT_UB_MEMBERS);
                            #if defined(SOKOL_GLES3)
                            _SG_VALIDATE(0 != u_desc->name, VALIDATE_SHADERDESC_UB_MEMBER_NAME);
                            #endif
                            const int array_count = u_desc->array_count;
                            _SG_VALIDATE(array_count > 0, VALIDATE_SHADERDESC_UB_ARRAY_COUNT);
                            const uint32_t u_align = _sg_uniform_alignment(u_desc->type, array_count, ub_desc->layout);
                            const uint32_t u_size  = _sg_uniform_size(u_desc->type, array_count, ub_desc->layout);
                            uniform_offset = _sg_align_u32(uniform_offset, u_align);
                            uniform_offset += u_size;
                            num_uniforms++;
                            // with std140, arrays are only allowed for FLOAT4, INT4, MAT4
                            if (ub_desc->layout == SG_UNIFORMLAYOUT_STD140) {
                                if (array_count > 1) {
                                    _SG_VALIDATE((u_desc->type == SG_UNIFORMTYPE_FLOAT4) || (u_desc->type == SG_UNIFORMTYPE_INT4) || (u_desc->type == SG_UNIFORMTYPE_MAT4), VALIDATE_SHADERDESC_UB_STD140_ARRAY_TYPE);
                                }
                            }
                        } else {
                            uniforms_continuous = false;
                        }
                    }
                    if (ub_desc->layout == SG_UNIFORMLAYOUT_STD140) {
                        uniform_offset = _sg_align_u32(uniform_offset, 16);
                    }
                    _SG_VALIDATE((size_t)uniform_offset == ub_desc->size, VALIDATE_SHADERDESC_UB_SIZE_MISMATCH);
                    _SG_VALIDATE(num_uniforms > 0, VALIDATE_SHADERDESC_NO_UB_MEMBERS);
                    #endif
                } else {
                    uniform_blocks_continuous = false;
                }
            }
            bool storage_buffers_continuous = true;
            for (int sbuf_index = 0; sbuf_index < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; sbuf_index++) {
                const sg_shader_storage_buffer_desc* sbuf_desc = &stage_desc->storage_buffers[sbuf_index];
                if (sbuf_desc->used) {
                    _SG_VALIDATE(storage_buffers_continuous, VALIDATE_SHADERDESC_NO_CONT_STORAGEBUFFERS);
                    _SG_VALIDATE(sbuf_desc->readonly, VALIDATE_SHADERDESC_STORAGEBUFFER_READONLY);
                } else {
                    storage_buffers_continuous = false;
                }
            }
            bool images_continuous = true;
            int num_images = 0;
            for (int img_index = 0; img_index < SG_MAX_SHADERSTAGE_IMAGES; img_index++) {
                const sg_shader_image_desc* img_desc = &stage_desc->images[img_index];
                if (img_desc->used) {
                    _SG_VALIDATE(images_continuous, VALIDATE_SHADERDESC_NO_CONT_IMAGES);
                    num_images++;
                } else {
                    images_continuous = false;
                }
            }
            bool samplers_continuous = true;
            int num_samplers = 0;
            for (int smp_index = 0; smp_index < SG_MAX_SHADERSTAGE_SAMPLERS; smp_index++) {
                const sg_shader_sampler_desc* smp_desc = &stage_desc->samplers[smp_index];
                if (smp_desc->used) {
                    _SG_VALIDATE(samplers_continuous, VALIDATE_SHADERDESC_NO_CONT_SAMPLERS);
                    num_samplers++;
                } else {
                    samplers_continuous = false;
                }
            }
            bool image_samplers_continuous = true;
            int num_image_samplers = 0;
            for (int img_smp_index = 0; img_smp_index < SG_MAX_SHADERSTAGE_IMAGESAMPLERPAIRS; img_smp_index++) {
                const sg_shader_image_sampler_pair_desc* img_smp_desc = &stage_desc->image_sampler_pairs[img_smp_index];
                if (img_smp_desc->used) {
                    _SG_VALIDATE(image_samplers_continuous, VALIDATE_SHADERDESC_NO_CONT_IMAGE_SAMPLER_PAIRS);
                    num_image_samplers++;
                    const bool img_slot_in_range = (img_smp_desc->image_slot >= 0) && (img_smp_desc->image_slot < SG_MAX_SHADERSTAGE_IMAGES);
                    const bool smp_slot_in_range = (img_smp_desc->sampler_slot >= 0) && (img_smp_desc->sampler_slot < SG_MAX_SHADERSTAGE_SAMPLERS);
                    _SG_VALIDATE(img_slot_in_range && (img_smp_desc->image_slot < num_images), VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_IMAGE_SLOT_OUT_OF_RANGE);
                    _SG_VALIDATE(smp_slot_in_range && (img_smp_desc->sampler_slot < num_samplers), VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_IMAGE_SLOT_OUT_OF_RANGE);
                    #if defined(_SOKOL_ANY_GL)
                    _SG_VALIDATE(img_smp_desc->glsl_name != 0, VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_NAME_REQUIRED_FOR_GL);
                    #endif
                    if (img_slot_in_range && smp_slot_in_range) {
                        const sg_shader_image_desc* img_desc = &stage_desc->images[img_smp_desc->image_slot];
                        const sg_shader_sampler_desc* smp_desc = &stage_desc->samplers[img_smp_desc->sampler_slot];
                        const bool needs_nonfiltering = (img_desc->sample_type == SG_IMAGESAMPLETYPE_UINT)
                                                     || (img_desc->sample_type == SG_IMAGESAMPLETYPE_SINT)
                                                     || (img_desc->sample_type == SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT);
                        const bool needs_comparison = img_desc->sample_type == SG_IMAGESAMPLETYPE_DEPTH;
                        if (needs_nonfiltering) {
                            _SG_VALIDATE(needs_nonfiltering && (smp_desc->sampler_type == SG_SAMPLERTYPE_NONFILTERING), VALIDATE_SHADERDESC_NONFILTERING_SAMPLER_REQUIRED);
                        }
                        if (needs_comparison) {
                            _SG_VALIDATE(needs_comparison && (smp_desc->sampler_type == SG_SAMPLERTYPE_COMPARISON), VALIDATE_SHADERDESC_COMPARISON_SAMPLER_REQUIRED);
                        }
                    }
                } else {
                    _SG_VALIDATE(img_smp_desc->glsl_name == 0, VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_NAME_BUT_NOT_USED);
                    _SG_VALIDATE(img_smp_desc->image_slot == 0, VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_IMAGE_BUT_NOT_USED);
                    _SG_VALIDATE(img_smp_desc->sampler_slot == 0, VALIDATE_SHADERDESC_IMAGE_SAMPLER_PAIR_HAS_SAMPLER_BUT_NOT_USED);
                    image_samplers_continuous = false;
                }
            }
            // each image and sampler must be referenced by an image sampler
            const uint32_t expected_img_slot_mask = (uint32_t)((1 << num_images) - 1);
            const uint32_t expected_smp_slot_mask = (uint32_t)((1 << num_samplers) - 1);
            uint32_t actual_img_slot_mask = 0;
            uint32_t actual_smp_slot_mask = 0;
            for (int img_smp_index = 0; img_smp_index < num_image_samplers; img_smp_index++) {
                const sg_shader_image_sampler_pair_desc* img_smp_desc = &stage_desc->image_sampler_pairs[img_smp_index];
                actual_img_slot_mask |= (1 << ((uint32_t)img_smp_desc->image_slot & 31));
                actual_smp_slot_mask |= (1 << ((uint32_t)img_smp_desc->sampler_slot & 31));
            }
            _SG_VALIDATE(expected_img_slot_mask == actual_img_slot_mask, VALIDATE_SHADERDESC_IMAGE_NOT_REFERENCED_BY_IMAGE_SAMPLER_PAIRS);
            _SG_VALIDATE(expected_smp_slot_mask == actual_smp_slot_mask, VALIDATE_SHADERDESC_SAMPLER_NOT_REFERENCED_BY_IMAGE_SAMPLER_PAIRS);
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_pipeline_desc(const sg_pipeline_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_PIPELINEDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_PIPELINEDESC_CANARY);
        _SG_VALIDATE(desc->shader.id != SG_INVALID_ID, VALIDATE_PIPELINEDESC_SHADER);
        for (int buf_index = 0; buf_index < SG_MAX_VERTEX_BUFFERS; buf_index++) {
            const sg_vertex_buffer_layout_state* l_state = &desc->layout.buffers[buf_index];
            if (l_state->stride == 0) {
                continue;
            }
            _SG_VALIDATE(_sg_multiple_u64((uint64_t)l_state->stride, 4), VALIDATE_PIPELINEDESC_LAYOUT_STRIDE4);
        }
        const _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, desc->shader.id);
        _SG_VALIDATE(0 != shd, VALIDATE_PIPELINEDESC_SHADER);
        if (shd) {
            _SG_VALIDATE(shd->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_PIPELINEDESC_SHADER);
            bool attrs_cont = true;
            for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
                const sg_vertex_attr_state* a_state = &desc->layout.attrs[attr_index];
                if (a_state->format == SG_VERTEXFORMAT_INVALID) {
                    attrs_cont = false;
                    continue;
                }
                _SG_VALIDATE(attrs_cont, VALIDATE_PIPELINEDESC_NO_CONT_ATTRS);
                SOKOL_ASSERT(a_state->buffer_index < SG_MAX_VERTEX_BUFFERS);
            }
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_attachments_desc(const sg_attachments_desc* desc) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(desc);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(desc);
        _sg_validate_begin();
        _SG_VALIDATE(desc->_start_canary == 0, VALIDATE_ATTACHMENTSDESC_CANARY);
        _SG_VALIDATE(desc->_end_canary == 0, VALIDATE_ATTACHMENTSDESC_CANARY);
        bool atts_cont = true;
        int color_width = -1, color_height = -1, color_sample_count = -1;
        bool has_color_atts = false;
        for (int att_index = 0; att_index < SG_MAX_COLOR_ATTACHMENTS; att_index++) {
            const sg_attachment_desc* att = &desc->colors[att_index];
            if (att->image.id == SG_INVALID_ID) {
                atts_cont = false;
                continue;
            }
            _SG_VALIDATE(atts_cont, VALIDATE_ATTACHMENTSDESC_NO_CONT_COLOR_ATTS);
            has_color_atts = true;
            const _sg_image_t* img = _sg_lookup_image(&_sg.pools, att->image.id);
            _SG_VALIDATE(img, VALIDATE_ATTACHMENTSDESC_IMAGE);
            if (0 != img) {
                _SG_VALIDATE(img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_ATTACHMENTSDESC_IMAGE);
                _SG_VALIDATE(img->cmn.render_target, VALIDATE_ATTACHMENTSDESC_IMAGE_NO_RT);
                _SG_VALIDATE(att->mip_level < img->cmn.num_mipmaps, VALIDATE_ATTACHMENTSDESC_MIPLEVEL);
                if (img->cmn.type == SG_IMAGETYPE_CUBE) {
                    _SG_VALIDATE(att->slice < 6, VALIDATE_ATTACHMENTSDESC_FACE);
                } else if (img->cmn.type == SG_IMAGETYPE_ARRAY) {
                    _SG_VALIDATE(att->slice < img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_LAYER);
                } else if (img->cmn.type == SG_IMAGETYPE_3D) {
                    _SG_VALIDATE(att->slice < img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_SLICE);
                }
                if (att_index == 0) {
                    color_width = _sg_miplevel_dim(img->cmn.width, att->mip_level);
                    color_height = _sg_miplevel_dim(img->cmn.height, att->mip_level);
                    color_sample_count = img->cmn.sample_count;
                } else {
                    _SG_VALIDATE(color_width == _sg_miplevel_dim(img->cmn.width, att->mip_level), VALIDATE_ATTACHMENTSDESC_IMAGE_SIZES);
                    _SG_VALIDATE(color_height == _sg_miplevel_dim(img->cmn.height, att->mip_level), VALIDATE_ATTACHMENTSDESC_IMAGE_SIZES);
                    _SG_VALIDATE(color_sample_count == img->cmn.sample_count, VALIDATE_ATTACHMENTSDESC_IMAGE_SAMPLE_COUNTS);
                }
                _SG_VALIDATE(_sg_is_valid_rendertarget_color_format(img->cmn.pixel_format), VALIDATE_ATTACHMENTSDESC_COLOR_INV_PIXELFORMAT);

                // check resolve attachment
                const sg_attachment_desc* res_att = &desc->resolves[att_index];
                if (res_att->image.id != SG_INVALID_ID) {
                    // associated color attachment must be MSAA
                    _SG_VALIDATE(img->cmn.sample_count > 1, VALIDATE_ATTACHMENTSDESC_RESOLVE_COLOR_IMAGE_MSAA);
                    const _sg_image_t* res_img = _sg_lookup_image(&_sg.pools, res_att->image.id);
                    _SG_VALIDATE(res_img, VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE);
                    if (res_img != 0) {
                        _SG_VALIDATE(res_img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE);
                        _SG_VALIDATE(res_img->cmn.render_target, VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_NO_RT);
                        _SG_VALIDATE(res_img->cmn.sample_count == 1, VALIDATE_ATTACHMENTSDESC_RESOLVE_SAMPLE_COUNT);
                        _SG_VALIDATE(res_att->mip_level < res_img->cmn.num_mipmaps, VALIDATE_ATTACHMENTSDESC_RESOLVE_MIPLEVEL);
                        if (res_img->cmn.type == SG_IMAGETYPE_CUBE) {
                            _SG_VALIDATE(res_att->slice < 6, VALIDATE_ATTACHMENTSDESC_RESOLVE_FACE);
                        } else if (res_img->cmn.type == SG_IMAGETYPE_ARRAY) {
                            _SG_VALIDATE(res_att->slice < res_img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_RESOLVE_LAYER);
                        } else if (res_img->cmn.type == SG_IMAGETYPE_3D) {
                            _SG_VALIDATE(res_att->slice < res_img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_RESOLVE_SLICE);
                        }
                        _SG_VALIDATE(img->cmn.pixel_format == res_img->cmn.pixel_format, VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_FORMAT);
                        _SG_VALIDATE(color_width == _sg_miplevel_dim(res_img->cmn.width, res_att->mip_level), VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_SIZES);
                        _SG_VALIDATE(color_height == _sg_miplevel_dim(res_img->cmn.height, res_att->mip_level), VALIDATE_ATTACHMENTSDESC_RESOLVE_IMAGE_SIZES);
                    }
                }
            }
        }
        bool has_depth_stencil_att = false;
        if (desc->depth_stencil.image.id != SG_INVALID_ID) {
            const sg_attachment_desc* att = &desc->depth_stencil;
            const _sg_image_t* img = _sg_lookup_image(&_sg.pools, att->image.id);
            _SG_VALIDATE(img, VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE);
            has_depth_stencil_att = true;
            if (img) {
                _SG_VALIDATE(img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE);
                _SG_VALIDATE(att->mip_level < img->cmn.num_mipmaps, VALIDATE_ATTACHMENTSDESC_DEPTH_MIPLEVEL);
                if (img->cmn.type == SG_IMAGETYPE_CUBE) {
                    _SG_VALIDATE(att->slice < 6, VALIDATE_ATTACHMENTSDESC_DEPTH_FACE);
                } else if (img->cmn.type == SG_IMAGETYPE_ARRAY) {
                    _SG_VALIDATE(att->slice < img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_DEPTH_LAYER);
                } else if (img->cmn.type == SG_IMAGETYPE_3D) {
                    // NOTE: this can't actually happen because of VALIDATE_IMAGEDESC_DEPTH_3D_IMAGE
                    _SG_VALIDATE(att->slice < img->cmn.num_slices, VALIDATE_ATTACHMENTSDESC_DEPTH_SLICE);
                }
                _SG_VALIDATE(img->cmn.render_target, VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_NO_RT);
                _SG_VALIDATE((color_width == -1) || (color_width == _sg_miplevel_dim(img->cmn.width, att->mip_level)), VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_SIZES);
                _SG_VALIDATE((color_height == -1) || (color_height == _sg_miplevel_dim(img->cmn.height, att->mip_level)), VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_SIZES);
                _SG_VALIDATE((color_sample_count == -1) || (color_sample_count == img->cmn.sample_count), VALIDATE_ATTACHMENTSDESC_DEPTH_IMAGE_SAMPLE_COUNT);
                _SG_VALIDATE(_sg_is_valid_rendertarget_depth_format(img->cmn.pixel_format), VALIDATE_ATTACHMENTSDESC_DEPTH_INV_PIXELFORMAT);
            }
        }
        _SG_VALIDATE(has_color_atts || has_depth_stencil_att, VALIDATE_ATTACHMENTSDESC_NO_ATTACHMENTS);
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_begin_pass(const sg_pass* pass) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(pass);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        _sg_validate_begin();
        _SG_VALIDATE(pass->_start_canary == 0, VALIDATE_BEGINPASS_CANARY);
        _SG_VALIDATE(pass->_end_canary == 0, VALIDATE_BEGINPASS_CANARY);
        if (pass->attachments.id == SG_INVALID_ID) {
            // this is a swapchain pass
            _SG_VALIDATE(pass->swapchain.width > 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_WIDTH);
            _SG_VALIDATE(pass->swapchain.height > 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_HEIGHT);
            _SG_VALIDATE(pass->swapchain.sample_count > 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_SAMPLECOUNT);
            _SG_VALIDATE(pass->swapchain.color_format > SG_PIXELFORMAT_NONE, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_COLORFORMAT);
            // NOTE: depth buffer is optional, so depth_format is allowed to be invalid
            // NOTE: the GL framebuffer handle may actually be 0
        } else {
            // this is an 'offscreen pass'
            const _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, pass->attachments.id);
            if (atts) {
                _SG_VALIDATE(atts->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_BEGINPASS_ATTACHMENTS_VALID);
                for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                    const _sg_attachment_common_t* color_att = &atts->cmn.colors[i];
                    const _sg_image_t* color_img = _sg_attachments_color_image(atts, i);
                    if (color_img) {
                        _SG_VALIDATE(color_img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_BEGINPASS_COLOR_ATTACHMENT_IMAGE);
                        _SG_VALIDATE(color_img->slot.id == color_att->image_id.id, VALIDATE_BEGINPASS_COLOR_ATTACHMENT_IMAGE);
                    }
                    const _sg_attachment_common_t* resolve_att = &atts->cmn.resolves[i];
                    const _sg_image_t* resolve_img = _sg_attachments_resolve_image(atts, i);
                    if (resolve_img) {
                        _SG_VALIDATE(resolve_img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_BEGINPASS_RESOLVE_ATTACHMENT_IMAGE);
                        _SG_VALIDATE(resolve_img->slot.id == resolve_att->image_id.id, VALIDATE_BEGINPASS_RESOLVE_ATTACHMENT_IMAGE);
                    }
                }
                const _sg_image_t* ds_img = _sg_attachments_ds_image(atts);
                if (ds_img) {
                    const _sg_attachment_common_t* att = &atts->cmn.depth_stencil;
                    _SG_VALIDATE(ds_img->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_BEGINPASS_DEPTHSTENCIL_ATTACHMENT_IMAGE);
                    _SG_VALIDATE(ds_img->slot.id == att->image_id.id, VALIDATE_BEGINPASS_DEPTHSTENCIL_ATTACHMENT_IMAGE);
                }
            } else {
                _SG_VALIDATE(atts != 0, VALIDATE_BEGINPASS_ATTACHMENTS_EXISTS);
            }
            // swapchain params must be all zero!
            _SG_VALIDATE(pass->swapchain.width == 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_WIDTH_NOTSET);
            _SG_VALIDATE(pass->swapchain.height == 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_HEIGHT_NOTSET);
            _SG_VALIDATE(pass->swapchain.sample_count == 0, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_SAMPLECOUNT_NOTSET);
            _SG_VALIDATE(pass->swapchain.color_format == _SG_PIXELFORMAT_DEFAULT, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_COLORFORMAT_NOTSET);
            _SG_VALIDATE(pass->swapchain.depth_format == _SG_PIXELFORMAT_DEFAULT, VALIDATE_BEGINPASS_SWAPCHAIN_EXPECT_DEPTHFORMAT_NOTSET);
                _SG_VALIDATE(pass->swapchain.gl.framebuffer == 0, VALIDATE_BEGINPASS_SWAPCHAIN_GL_EXPECT_FRAMEBUFFER_NOTSET);
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_apply_pipeline(sg_pipeline pip_id) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(pip_id);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        _sg_validate_begin();
        // the pipeline object must be alive and valid
        _SG_VALIDATE(pip_id.id != SG_INVALID_ID, VALIDATE_APIP_PIPELINE_VALID_ID);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
        _SG_VALIDATE(pip != 0, VALIDATE_APIP_PIPELINE_EXISTS);
        if (!pip) {
            return _sg_validate_end();
        }
        _SG_VALIDATE(pip->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_APIP_PIPELINE_VALID);
        // the pipeline's shader must be alive and valid
        SOKOL_ASSERT(pip->shader);
        _SG_VALIDATE(pip->shader->slot.id == pip->cmn.shader_id.id, VALIDATE_APIP_SHADER_EXISTS);
        _SG_VALIDATE(pip->shader->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_APIP_SHADER_VALID);
        // check that pipeline attributes match current pass attributes
        if (_sg.cur_pass.atts_id.id != SG_INVALID_ID) {
            // an offscreen pass
            const _sg_attachments_t* atts = _sg.cur_pass.atts;
            SOKOL_ASSERT(atts);
            _SG_VALIDATE(atts->slot.id == _sg.cur_pass.atts_id.id, VALIDATE_APIP_CURPASS_ATTACHMENTS_EXISTS);
            _SG_VALIDATE(atts->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_APIP_CURPASS_ATTACHMENTS_VALID);

            _SG_VALIDATE(pip->cmn.color_count == atts->cmn.num_colors, VALIDATE_APIP_ATT_COUNT);
            for (int i = 0; i < pip->cmn.color_count; i++) {
                const _sg_image_t* att_img = _sg_attachments_color_image(atts, i);
                _SG_VALIDATE(pip->cmn.colors[i].pixel_format == att_img->cmn.pixel_format, VALIDATE_APIP_COLOR_FORMAT);
                _SG_VALIDATE(pip->cmn.sample_count == att_img->cmn.sample_count, VALIDATE_APIP_SAMPLE_COUNT);
            }
            const _sg_image_t* att_dsimg = _sg_attachments_ds_image(atts);
            if (att_dsimg) {
                _SG_VALIDATE(pip->cmn.depth.pixel_format == att_dsimg->cmn.pixel_format, VALIDATE_APIP_DEPTH_FORMAT);
            } else {
                _SG_VALIDATE(pip->cmn.depth.pixel_format == SG_PIXELFORMAT_NONE, VALIDATE_APIP_DEPTH_FORMAT);
            }
        } else {
            // default pass
            _SG_VALIDATE(pip->cmn.color_count == 1, VALIDATE_APIP_ATT_COUNT);
            _SG_VALIDATE(pip->cmn.colors[0].pixel_format == _sg.cur_pass.swapchain.color_fmt, VALIDATE_APIP_COLOR_FORMAT);
            _SG_VALIDATE(pip->cmn.depth.pixel_format == _sg.cur_pass.swapchain.depth_fmt, VALIDATE_APIP_DEPTH_FORMAT);
            _SG_VALIDATE(pip->cmn.sample_count == _sg.cur_pass.swapchain.sample_count, VALIDATE_APIP_SAMPLE_COUNT);
        }
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_apply_bindings(const sg_bindings* bindings) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(bindings);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        _sg_validate_begin();

        // a pipeline object must have been applied
        _SG_VALIDATE(_sg.cur_pipeline.id != SG_INVALID_ID, VALIDATE_ABND_PIPELINE);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, _sg.cur_pipeline.id);
        _SG_VALIDATE(pip != 0, VALIDATE_ABND_PIPELINE_EXISTS);
        if (!pip) {
            return _sg_validate_end();
        }
        _SG_VALIDATE(pip->slot.state == SG_RESOURCESTATE_VALID, VALIDATE_ABND_PIPELINE_VALID);
        SOKOL_ASSERT(pip->shader && (pip->cmn.shader_id.id == pip->shader->slot.id));

        // has expected vertex buffers, and vertex buffers still exist
        for (int i = 0; i < SG_MAX_VERTEX_BUFFERS; i++) {
            if (bindings->vertex_buffers[i].id != SG_INVALID_ID) {
                _SG_VALIDATE(pip->cmn.vertex_buffer_layout_active[i], VALIDATE_ABND_VBS);
                // buffers in vertex-buffer-slots must be of type SG_BUFFERTYPE_VERTEXBUFFER
                const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, bindings->vertex_buffers[i].id);
                _SG_VALIDATE(buf != 0, VALIDATE_ABND_VB_EXISTS);
                if (buf && buf->slot.state == SG_RESOURCESTATE_VALID) {
                    _SG_VALIDATE(SG_BUFFERTYPE_VERTEXBUFFER == buf->cmn.type, VALIDATE_ABND_VB_TYPE);
                    _SG_VALIDATE(!buf->cmn.append_overflow, VALIDATE_ABND_VB_OVERFLOW);
                }
            } else {
                // vertex buffer provided in a slot which has no vertex layout in pipeline
                _SG_VALIDATE(!pip->cmn.vertex_buffer_layout_active[i], VALIDATE_ABND_VBS);
            }
        }

        // index buffer expected or not, and index buffer still exists
        if (pip->cmn.index_type == SG_INDEXTYPE_NONE) {
            // pipeline defines non-indexed rendering, but index buffer provided
            _SG_VALIDATE(bindings->index_buffer.id == SG_INVALID_ID, VALIDATE_ABND_IB);
        } else {
            // pipeline defines indexed rendering, but no index buffer provided
            _SG_VALIDATE(bindings->index_buffer.id != SG_INVALID_ID, VALIDATE_ABND_NO_IB);
        }
        if (bindings->index_buffer.id != SG_INVALID_ID) {
            // buffer in index-buffer-slot must be of type SG_BUFFERTYPE_INDEXBUFFER
            const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, bindings->index_buffer.id);
            _SG_VALIDATE(buf != 0, VALIDATE_ABND_IB_EXISTS);
            if (buf && buf->slot.state == SG_RESOURCESTATE_VALID) {
                _SG_VALIDATE(SG_BUFFERTYPE_INDEXBUFFER == buf->cmn.type, VALIDATE_ABND_IB_TYPE);
                _SG_VALIDATE(!buf->cmn.append_overflow, VALIDATE_ABND_IB_OVERFLOW);
            }
        }

        // has expected vertex shader images
        for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_VS];
            if (stage->images[i].image_type != _SG_IMAGETYPE_DEFAULT) {
                _SG_VALIDATE(bindings->vs.images[i].id != SG_INVALID_ID, VALIDATE_ABND_VS_EXPECTED_IMAGE_BINDING);
                if (bindings->vs.images[i].id != SG_INVALID_ID) {
                    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, bindings->vs.images[i].id);
                    _SG_VALIDATE(img != 0, VALIDATE_ABND_VS_IMG_EXISTS);
                    if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
                        _SG_VALIDATE(img->cmn.type == stage->images[i].image_type, VALIDATE_ABND_VS_IMAGE_TYPE_MISMATCH);
                        _SG_VALIDATE(img->cmn.sample_count == 1, VALIDATE_ABND_VS_IMAGE_MSAA);
                        const _sg_pixelformat_info_t* info = &_sg.formats[img->cmn.pixel_format];
                        switch (stage->images[i].sample_type) {
                            case SG_IMAGESAMPLETYPE_FLOAT:
                                _SG_VALIDATE(info->filter, VALIDATE_ABND_VS_EXPECTED_FILTERABLE_IMAGE);
                                break;
                            case SG_IMAGESAMPLETYPE_DEPTH:
                                _SG_VALIDATE(info->depth, VALIDATE_ABND_VS_EXPECTED_DEPTH_IMAGE);
                                break;
                            default:
                                break;
                        }
                    }
                }
            } else {
                _SG_VALIDATE(bindings->vs.images[i].id == SG_INVALID_ID, VALIDATE_ABND_VS_UNEXPECTED_IMAGE_BINDING);
            }
        }

        // has expected vertex shader image samplers
        for (int i = 0; i < SG_MAX_SHADERSTAGE_SAMPLERS; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_VS];
            if (stage->samplers[i].sampler_type != _SG_SAMPLERTYPE_DEFAULT) {
                _SG_VALIDATE(bindings->vs.samplers[i].id != SG_INVALID_ID, VALIDATE_ABND_VS_EXPECTED_SAMPLER_BINDING);
                if (bindings->vs.samplers[i].id != SG_INVALID_ID) {
                    const _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, bindings->vs.samplers[i].id);
                    _SG_VALIDATE(smp != 0, VALIDATE_ABND_VS_SMP_EXISTS);
                    if (smp) {
                        if (stage->samplers[i].sampler_type == SG_SAMPLERTYPE_COMPARISON) {
                            _SG_VALIDATE(smp->cmn.compare != SG_COMPAREFUNC_NEVER, VALIDATE_ABND_VS_UNEXPECTED_SAMPLER_COMPARE_NEVER);
                        } else {
                            _SG_VALIDATE(smp->cmn.compare == SG_COMPAREFUNC_NEVER, VALIDATE_ABND_VS_EXPECTED_SAMPLER_COMPARE_NEVER);
                        }
                        if (stage->samplers[i].sampler_type == SG_SAMPLERTYPE_NONFILTERING) {
                            const bool nonfiltering = (smp->cmn.min_filter != SG_FILTER_LINEAR)
                                                   && (smp->cmn.mag_filter != SG_FILTER_LINEAR)
                                                   && (smp->cmn.mipmap_filter != SG_FILTER_LINEAR);
                            _SG_VALIDATE(nonfiltering, VALIDATE_ABND_VS_EXPECTED_NONFILTERING_SAMPLER);
                        }
                    }
                }
            } else {
                _SG_VALIDATE(bindings->vs.samplers[i].id == SG_INVALID_ID, VALIDATE_ABND_VS_UNEXPECTED_SAMPLER_BINDING);
            }
        }

        // has expected vertex shader storage buffers
        for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_VS];
            if (stage->storage_buffers[i].used) {
                _SG_VALIDATE(bindings->vs.storage_buffers[i].id != SG_INVALID_ID, VALIDATE_ABND_VS_EXPECTED_STORAGEBUFFER_BINDING);
                if (bindings->vs.storage_buffers[i].id != SG_INVALID_ID) {
                    const _sg_buffer_t* sbuf = _sg_lookup_buffer(&_sg.pools, bindings->vs.storage_buffers[i].id);
                    _SG_VALIDATE(sbuf != 0, VALIDATE_ABND_VS_STORAGEBUFFER_EXISTS);
                    if (sbuf) {
                        _SG_VALIDATE(sbuf->cmn.type == SG_BUFFERTYPE_STORAGEBUFFER, VALIDATE_ABND_VS_STORAGEBUFFER_BINDING_BUFFERTYPE);
                    }
                }
            } else {
                _SG_VALIDATE(bindings->vs.storage_buffers[i].id == SG_INVALID_ID, VALIDATE_ABND_VS_UNEXPECTED_STORAGEBUFFER_BINDING);
            }
        }

        // has expected fragment shader images
        for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_FS];
            if (stage->images[i].image_type != _SG_IMAGETYPE_DEFAULT) {
                _SG_VALIDATE(bindings->fs.images[i].id != SG_INVALID_ID, VALIDATE_ABND_FS_EXPECTED_IMAGE_BINDING);
                if (bindings->fs.images[i].id != SG_INVALID_ID) {
                    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, bindings->fs.images[i].id);
                    _SG_VALIDATE(img != 0, VALIDATE_ABND_FS_IMG_EXISTS);
                    if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
                        _SG_VALIDATE(img->cmn.type == stage->images[i].image_type, VALIDATE_ABND_FS_IMAGE_TYPE_MISMATCH);
                        _SG_VALIDATE(img->cmn.sample_count == 1, VALIDATE_ABND_FS_IMAGE_MSAA);
                        const _sg_pixelformat_info_t* info = &_sg.formats[img->cmn.pixel_format];
                        switch (stage->images[i].sample_type) {
                            case SG_IMAGESAMPLETYPE_FLOAT:
                                _SG_VALIDATE(info->filter, VALIDATE_ABND_FS_EXPECTED_FILTERABLE_IMAGE);
                                break;
                            case SG_IMAGESAMPLETYPE_DEPTH:
                                _SG_VALIDATE(info->depth, VALIDATE_ABND_FS_EXPECTED_DEPTH_IMAGE);
                                break;
                            default:
                                break;
                        }
                    }
                }
            } else {
                _SG_VALIDATE(bindings->fs.images[i].id == SG_INVALID_ID, VALIDATE_ABND_FS_UNEXPECTED_IMAGE_BINDING);
            }
        }

        // has expected fragment shader samplers
        for (int i = 0; i < SG_MAX_SHADERSTAGE_SAMPLERS; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_FS];
            if (stage->samplers[i].sampler_type != _SG_SAMPLERTYPE_DEFAULT) {
                _SG_VALIDATE(bindings->fs.samplers[i].id != SG_INVALID_ID, VALIDATE_ABND_FS_EXPECTED_SAMPLER_BINDING);
                if (bindings->fs.samplers[i].id != SG_INVALID_ID) {
                    const _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, bindings->fs.samplers[i].id);
                    _SG_VALIDATE(smp != 0, VALIDATE_ABND_FS_SMP_EXISTS);
                    if (smp) {
                        if (stage->samplers[i].sampler_type == SG_SAMPLERTYPE_COMPARISON) {
                            _SG_VALIDATE(smp->cmn.compare != SG_COMPAREFUNC_NEVER, VALIDATE_ABND_FS_UNEXPECTED_SAMPLER_COMPARE_NEVER);
                        } else {
                            _SG_VALIDATE(smp->cmn.compare == SG_COMPAREFUNC_NEVER, VALIDATE_ABND_FS_EXPECTED_SAMPLER_COMPARE_NEVER);
                        }
                        if (stage->samplers[i].sampler_type == SG_SAMPLERTYPE_NONFILTERING) {
                            const bool nonfiltering = (smp->cmn.min_filter != SG_FILTER_LINEAR)
                                                   && (smp->cmn.mag_filter != SG_FILTER_LINEAR)
                                                   && (smp->cmn.mipmap_filter != SG_FILTER_LINEAR);
                            _SG_VALIDATE(nonfiltering, VALIDATE_ABND_FS_EXPECTED_NONFILTERING_SAMPLER);
                        }
                    }
                }
            } else {
                _SG_VALIDATE(bindings->fs.samplers[i].id == SG_INVALID_ID, VALIDATE_ABND_FS_UNEXPECTED_SAMPLER_BINDING);
            }
        }

        // has expected fragment shader storage buffers
        for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++) {
            const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[SG_SHADERSTAGE_FS];
            if (stage->storage_buffers[i].used) {
                _SG_VALIDATE(bindings->fs.storage_buffers[i].id != SG_INVALID_ID, VALIDATE_ABND_FS_EXPECTED_STORAGEBUFFER_BINDING);
                if (bindings->fs.storage_buffers[i].id != SG_INVALID_ID) {
                    const _sg_buffer_t* sbuf = _sg_lookup_buffer(&_sg.pools, bindings->fs.storage_buffers[i].id);
                    _SG_VALIDATE(sbuf != 0, VALIDATE_ABND_FS_STORAGEBUFFER_EXISTS);
                    if (sbuf) {
                        _SG_VALIDATE(sbuf->cmn.type == SG_BUFFERTYPE_STORAGEBUFFER, VALIDATE_ABND_FS_STORAGEBUFFER_BINDING_BUFFERTYPE);
                    }
                }
            } else {
                _SG_VALIDATE(bindings->fs.storage_buffers[i].id == SG_INVALID_ID, VALIDATE_ABND_FS_UNEXPECTED_STORAGEBUFFER_BINDING);
            }
        }

        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_apply_uniforms(sg_shader_stage stage_index, int ub_index, const sg_range* data) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(stage_index);
        _SOKOL_UNUSED(ub_index);
        _SOKOL_UNUSED(data);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT((stage_index == SG_SHADERSTAGE_VS) || (stage_index == SG_SHADERSTAGE_FS));
        SOKOL_ASSERT((ub_index >= 0) && (ub_index < SG_MAX_SHADERSTAGE_UBS));
        _sg_validate_begin();
        _SG_VALIDATE(_sg.cur_pipeline.id != SG_INVALID_ID, VALIDATE_AUB_NO_PIPELINE);
        const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, _sg.cur_pipeline.id);
        SOKOL_ASSERT(pip && (pip->slot.id == _sg.cur_pipeline.id));
        SOKOL_ASSERT(pip->shader && (pip->shader->slot.id == pip->cmn.shader_id.id));

        // check that there is a uniform block at 'stage' and 'ub_index'
        const _sg_shader_stage_t* stage = &pip->shader->cmn.stage[stage_index];
        _SG_VALIDATE(ub_index < stage->num_uniform_blocks, VALIDATE_AUB_NO_UB_AT_SLOT);

        // check that the provided data size matches the uniform block size
        _SG_VALIDATE(data->size == stage->uniform_blocks[ub_index].size, VALIDATE_AUB_SIZE);

        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_update_buffer(const _sg_buffer_t* buf, const sg_range* data) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(buf);
        _SOKOL_UNUSED(data);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(buf && data && data->ptr);
        _sg_validate_begin();
        _SG_VALIDATE(buf->cmn.usage != SG_USAGE_IMMUTABLE, VALIDATE_UPDATEBUF_USAGE);
        _SG_VALIDATE(buf->cmn.size >= (int)data->size, VALIDATE_UPDATEBUF_SIZE);
        _SG_VALIDATE(buf->cmn.update_frame_index != _sg.frame_index, VALIDATE_UPDATEBUF_ONCE);
        _SG_VALIDATE(buf->cmn.append_frame_index != _sg.frame_index, VALIDATE_UPDATEBUF_APPEND);
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_append_buffer(const _sg_buffer_t* buf, const sg_range* data) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(buf);
        _SOKOL_UNUSED(data);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(buf && data && data->ptr);
        _sg_validate_begin();
        _SG_VALIDATE(buf->cmn.usage != SG_USAGE_IMMUTABLE, VALIDATE_APPENDBUF_USAGE);
        _SG_VALIDATE(buf->cmn.size >= (buf->cmn.append_pos + (int)data->size), VALIDATE_APPENDBUF_SIZE);
        _SG_VALIDATE(buf->cmn.update_frame_index != _sg.frame_index, VALIDATE_APPENDBUF_UPDATE);
        return _sg_validate_end();
    #endif
}

_SOKOL_PRIVATE bool _sg_validate_update_image(const _sg_image_t* img, const sg_image_data* data) {
    #if !defined(SOKOL_DEBUG)
        _SOKOL_UNUSED(img);
        _SOKOL_UNUSED(data);
        return true;
    #else
        if (_sg.desc.disable_validation) {
            return true;
        }
        SOKOL_ASSERT(img && data);
        _sg_validate_begin();
        _SG_VALIDATE(img->cmn.usage != SG_USAGE_IMMUTABLE, VALIDATE_UPDIMG_USAGE);
        _SG_VALIDATE(img->cmn.upd_frame_index != _sg.frame_index, VALIDATE_UPDIMG_ONCE);
        _sg_validate_image_data(data,
            img->cmn.pixel_format,
            img->cmn.width,
            img->cmn.height,
            (img->cmn.type == SG_IMAGETYPE_CUBE) ? 6 : 1,
            img->cmn.num_mipmaps,
            img->cmn.num_slices);
        return _sg_validate_end();
    #endif
}

// ██████  ███████ ███████  ██████  ██    ██ ██████   ██████ ███████ ███████
// ██   ██ ██      ██      ██    ██ ██    ██ ██   ██ ██      ██      ██
// ██████  █████   ███████ ██    ██ ██    ██ ██████  ██      █████   ███████
// ██   ██ ██           ██ ██    ██ ██    ██ ██   ██ ██      ██           ██
// ██   ██ ███████ ███████  ██████   ██████  ██   ██  ██████ ███████ ███████
//
// >>resources
_SOKOL_PRIVATE sg_buffer_desc _sg_buffer_desc_defaults(const sg_buffer_desc* desc) {
    sg_buffer_desc def = *desc;
    def.type = _sg_def(def.type, SG_BUFFERTYPE_VERTEXBUFFER);
    def.usage = _sg_def(def.usage, SG_USAGE_IMMUTABLE);
    if (def.size == 0) {
        def.size = def.data.size;
    } else if (def.data.size == 0) {
        def.data.size = def.size;
    }
    return def;
}

_SOKOL_PRIVATE sg_image_desc _sg_image_desc_defaults(const sg_image_desc* desc) {
    sg_image_desc def = *desc;
    def.type = _sg_def(def.type, SG_IMAGETYPE_2D);
    def.num_slices = _sg_def(def.num_slices, 1);
    def.num_mipmaps = _sg_def(def.num_mipmaps, 1);
    def.usage = _sg_def(def.usage, SG_USAGE_IMMUTABLE);
    if (desc->render_target) {
        def.pixel_format = _sg_def(def.pixel_format, _sg.desc.environment.defaults.color_format);
        def.sample_count = _sg_def(def.sample_count, _sg.desc.environment.defaults.sample_count);
    } else {
        def.pixel_format = _sg_def(def.pixel_format, SG_PIXELFORMAT_RGBA8);
        def.sample_count = _sg_def(def.sample_count, 1);
    }
    return def;
}

_SOKOL_PRIVATE sg_sampler_desc _sg_sampler_desc_defaults(const sg_sampler_desc* desc) {
    sg_sampler_desc def = *desc;
    def.min_filter = _sg_def(def.min_filter, SG_FILTER_NEAREST);
    def.mag_filter = _sg_def(def.mag_filter, SG_FILTER_NEAREST);
    def.mipmap_filter = _sg_def(def.mipmap_filter, SG_FILTER_NONE);
    def.wrap_u = _sg_def(def.wrap_u, SG_WRAP_REPEAT);
    def.wrap_v = _sg_def(def.wrap_v, SG_WRAP_REPEAT);
    def.wrap_w = _sg_def(def.wrap_w, SG_WRAP_REPEAT);
    def.max_lod = _sg_def_flt(def.max_lod, FLT_MAX);
    def.border_color = _sg_def(def.border_color, SG_BORDERCOLOR_OPAQUE_BLACK);
    def.compare = _sg_def(def.compare, SG_COMPAREFUNC_NEVER);
    def.max_anisotropy = _sg_def(def.max_anisotropy, 1);
    return def;
}

_SOKOL_PRIVATE sg_shader_desc _sg_shader_desc_defaults(const sg_shader_desc* desc) {
    sg_shader_desc def = *desc;
        def.vs.entry = _sg_def(def.vs.entry, "main");
        def.fs.entry = _sg_def(def.fs.entry, "main");
    for (int stage_index = 0; stage_index < SG_NUM_SHADER_STAGES; stage_index++) {
        sg_shader_stage_desc* stage_desc = (stage_index == SG_SHADERSTAGE_VS)? &def.vs : &def.fs;
        for (int ub_index = 0; ub_index < SG_MAX_SHADERSTAGE_UBS; ub_index++) {
            sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_index];
            if (0 == ub_desc->size) {
                break;
            }
            ub_desc->layout = _sg_def(ub_desc->layout, SG_UNIFORMLAYOUT_NATIVE);
            for (int u_index = 0; u_index < SG_MAX_UB_MEMBERS; u_index++) {
                sg_shader_uniform_desc* u_desc = &ub_desc->uniforms[u_index];
                if (u_desc->type == SG_UNIFORMTYPE_INVALID) {
                    break;
                }
                u_desc->array_count = _sg_def(u_desc->array_count, 1);
            }
        }
        for (int img_index = 0; img_index < SG_MAX_SHADERSTAGE_IMAGES; img_index++) {
            sg_shader_image_desc* img_desc = &stage_desc->images[img_index];
            if (!img_desc->used) {
                break;
            }
            img_desc->image_type = _sg_def(img_desc->image_type, SG_IMAGETYPE_2D);
            img_desc->sample_type = _sg_def(img_desc->sample_type, SG_IMAGESAMPLETYPE_FLOAT);
        }
        for (int smp_index = 0; smp_index < SG_MAX_SHADERSTAGE_SAMPLERS; smp_index++) {
            sg_shader_sampler_desc* smp_desc = &stage_desc->samplers[smp_index];
            if (!smp_desc->used) {
                break;
            }
            smp_desc->sampler_type = _sg_def(smp_desc->sampler_type, SG_SAMPLERTYPE_FILTERING);
        }
    }
    return def;
}

_SOKOL_PRIVATE sg_pipeline_desc _sg_pipeline_desc_defaults(const sg_pipeline_desc* desc) {
    sg_pipeline_desc def = *desc;

    def.primitive_type = _sg_def(def.primitive_type, SG_PRIMITIVETYPE_TRIANGLES);
    def.index_type = _sg_def(def.index_type, SG_INDEXTYPE_NONE);
    def.cull_mode = _sg_def(def.cull_mode, SG_CULLMODE_NONE);
    def.face_winding = _sg_def(def.face_winding, SG_FACEWINDING_CW);
    def.sample_count = _sg_def(def.sample_count, _sg.desc.environment.defaults.sample_count);

    def.stencil.front.compare = _sg_def(def.stencil.front.compare, SG_COMPAREFUNC_ALWAYS);
    def.stencil.front.fail_op = _sg_def(def.stencil.front.fail_op, SG_STENCILOP_KEEP);
    def.stencil.front.depth_fail_op = _sg_def(def.stencil.front.depth_fail_op, SG_STENCILOP_KEEP);
    def.stencil.front.pass_op = _sg_def(def.stencil.front.pass_op, SG_STENCILOP_KEEP);
    def.stencil.back.compare = _sg_def(def.stencil.back.compare, SG_COMPAREFUNC_ALWAYS);
    def.stencil.back.fail_op = _sg_def(def.stencil.back.fail_op, SG_STENCILOP_KEEP);
    def.stencil.back.depth_fail_op = _sg_def(def.stencil.back.depth_fail_op, SG_STENCILOP_KEEP);
    def.stencil.back.pass_op = _sg_def(def.stencil.back.pass_op, SG_STENCILOP_KEEP);

    def.depth.compare = _sg_def(def.depth.compare, SG_COMPAREFUNC_ALWAYS);
    def.depth.pixel_format = _sg_def(def.depth.pixel_format, _sg.desc.environment.defaults.depth_format);
    if (def.colors[0].pixel_format == SG_PIXELFORMAT_NONE) {
        // special case depth-only rendering, enforce a color count of 0
        def.color_count = 0;
    } else {
        def.color_count = _sg_def(def.color_count, 1);
    }
    if (def.color_count > SG_MAX_COLOR_ATTACHMENTS) {
        def.color_count = SG_MAX_COLOR_ATTACHMENTS;
    }
    for (int i = 0; i < def.color_count; i++) {
        sg_color_target_state* cs = &def.colors[i];
        cs->pixel_format = _sg_def(cs->pixel_format, _sg.desc.environment.defaults.color_format);
        cs->write_mask = _sg_def(cs->write_mask, SG_COLORMASK_RGBA);
        sg_blend_state* bs = &def.colors[i].blend;
        bs->src_factor_rgb = _sg_def(bs->src_factor_rgb, SG_BLENDFACTOR_ONE);
        bs->dst_factor_rgb = _sg_def(bs->dst_factor_rgb, SG_BLENDFACTOR_ZERO);
        bs->op_rgb = _sg_def(bs->op_rgb, SG_BLENDOP_ADD);
        bs->src_factor_alpha = _sg_def(bs->src_factor_alpha, SG_BLENDFACTOR_ONE);
        bs->dst_factor_alpha = _sg_def(bs->dst_factor_alpha, SG_BLENDFACTOR_ZERO);
        bs->op_alpha = _sg_def(bs->op_alpha, SG_BLENDOP_ADD);
    }

    for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
        sg_vertex_attr_state* a_state = &def.layout.attrs[attr_index];
        if (a_state->format == SG_VERTEXFORMAT_INVALID) {
            break;
        }
        SOKOL_ASSERT(a_state->buffer_index < SG_MAX_VERTEX_BUFFERS);
        sg_vertex_buffer_layout_state* l_state = &def.layout.buffers[a_state->buffer_index];
        l_state->step_func = _sg_def(l_state->step_func, SG_VERTEXSTEP_PER_VERTEX);
        l_state->step_rate = _sg_def(l_state->step_rate, 1);
    }

    // resolve vertex layout strides and offsets
    int auto_offset[SG_MAX_VERTEX_BUFFERS];
    _sg_clear(auto_offset, sizeof(auto_offset));
    bool use_auto_offset = true;
    for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
        // to use computed offsets, *all* attr offsets must be 0
        if (def.layout.attrs[attr_index].offset != 0) {
            use_auto_offset = false;
        }
    }
    for (int attr_index = 0; attr_index < SG_MAX_VERTEX_ATTRIBUTES; attr_index++) {
        sg_vertex_attr_state* a_state = &def.layout.attrs[attr_index];
        if (a_state->format == SG_VERTEXFORMAT_INVALID) {
            break;
        }
        SOKOL_ASSERT(a_state->buffer_index < SG_MAX_VERTEX_BUFFERS);
        if (use_auto_offset) {
            a_state->offset = auto_offset[a_state->buffer_index];
        }
        auto_offset[a_state->buffer_index] += _sg_vertexformat_bytesize(a_state->format);
    }
    // compute vertex strides if needed
    for (int buf_index = 0; buf_index < SG_MAX_VERTEX_BUFFERS; buf_index++) {
        sg_vertex_buffer_layout_state* l_state = &def.layout.buffers[buf_index];
        if (l_state->stride == 0) {
            l_state->stride = auto_offset[buf_index];
        }
    }

    return def;
}

_SOKOL_PRIVATE sg_attachments_desc _sg_attachments_desc_defaults(const sg_attachments_desc* desc) {
    sg_attachments_desc def = *desc;
    return def;
}

_SOKOL_PRIVATE sg_buffer _sg_alloc_buffer(void) {
    sg_buffer res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.buffer_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id = _sg_slot_alloc(&_sg.pools.buffer_pool, &_sg.pools.buffers[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(BUFFER_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE sg_image _sg_alloc_image(void) {
    sg_image res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.image_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id = _sg_slot_alloc(&_sg.pools.image_pool, &_sg.pools.images[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(IMAGE_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE sg_sampler _sg_alloc_sampler(void) {
    sg_sampler res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.sampler_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id = _sg_slot_alloc(&_sg.pools.sampler_pool, &_sg.pools.samplers[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(SAMPLER_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE sg_shader _sg_alloc_shader(void) {
    sg_shader res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.shader_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id = _sg_slot_alloc(&_sg.pools.shader_pool, &_sg.pools.shaders[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(SHADER_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE sg_pipeline _sg_alloc_pipeline(void) {
    sg_pipeline res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.pipeline_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id =_sg_slot_alloc(&_sg.pools.pipeline_pool, &_sg.pools.pipelines[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(PIPELINE_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE sg_attachments _sg_alloc_attachments(void) {
    sg_attachments res;
    int slot_index = _sg_pool_alloc_index(&_sg.pools.attachments_pool);
    if (_SG_INVALID_SLOT_INDEX != slot_index) {
        res.id = _sg_slot_alloc(&_sg.pools.attachments_pool, &_sg.pools.attachments[slot_index].slot, slot_index);
    } else {
        res.id = SG_INVALID_ID;
        _SG_ERROR(PASS_POOL_EXHAUSTED);
    }
    return res;
}

_SOKOL_PRIVATE void _sg_dealloc_buffer(_sg_buffer_t* buf) {
    SOKOL_ASSERT(buf && (buf->slot.state == SG_RESOURCESTATE_ALLOC) && (buf->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.buffer_pool, _sg_slot_index(buf->slot.id));
    _sg_reset_slot(&buf->slot);
}

_SOKOL_PRIVATE void _sg_dealloc_image(_sg_image_t* img) {
    SOKOL_ASSERT(img && (img->slot.state == SG_RESOURCESTATE_ALLOC) && (img->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.image_pool, _sg_slot_index(img->slot.id));
    _sg_reset_slot(&img->slot);
}

_SOKOL_PRIVATE void _sg_dealloc_sampler(_sg_sampler_t* smp) {
    SOKOL_ASSERT(smp && (smp->slot.state == SG_RESOURCESTATE_ALLOC) && (smp->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.sampler_pool, _sg_slot_index(smp->slot.id));
    _sg_reset_slot(&smp->slot);
}

_SOKOL_PRIVATE void _sg_dealloc_shader(_sg_shader_t* shd) {
    SOKOL_ASSERT(shd && (shd->slot.state == SG_RESOURCESTATE_ALLOC) && (shd->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.shader_pool, _sg_slot_index(shd->slot.id));
    _sg_reset_slot(&shd->slot);
}

_SOKOL_PRIVATE void _sg_dealloc_pipeline(_sg_pipeline_t* pip) {
    SOKOL_ASSERT(pip && (pip->slot.state == SG_RESOURCESTATE_ALLOC) && (pip->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.pipeline_pool, _sg_slot_index(pip->slot.id));
    _sg_reset_slot(&pip->slot);
}

_SOKOL_PRIVATE void _sg_dealloc_attachments(_sg_attachments_t* atts) {
    SOKOL_ASSERT(atts && (atts->slot.state == SG_RESOURCESTATE_ALLOC) && (atts->slot.id != SG_INVALID_ID));
    _sg_pool_free_index(&_sg.pools.attachments_pool, _sg_slot_index(atts->slot.id));
    _sg_reset_slot(&atts->slot);
}

_SOKOL_PRIVATE void _sg_init_buffer(_sg_buffer_t* buf, const sg_buffer_desc* desc) {
    SOKOL_ASSERT(buf && (buf->slot.state == SG_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    if (_sg_validate_buffer_desc(desc)) {
        _sg_buffer_common_init(&buf->cmn, desc);
        buf->slot.state = _sg_create_buffer(buf, desc);
    } else {
        buf->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((buf->slot.state == SG_RESOURCESTATE_VALID)||(buf->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_init_image(_sg_image_t* img, const sg_image_desc* desc) {
    SOKOL_ASSERT(img && (img->slot.state == SG_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    if (_sg_validate_image_desc(desc)) {
        _sg_image_common_init(&img->cmn, desc);
        img->slot.state = _sg_create_image(img, desc);
    } else {
        img->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((img->slot.state == SG_RESOURCESTATE_VALID)||(img->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_init_sampler(_sg_sampler_t* smp, const sg_sampler_desc* desc) {
    SOKOL_ASSERT(smp && (smp->slot.state == SG_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    if (_sg_validate_sampler_desc(desc)) {
        _sg_sampler_common_init(&smp->cmn, desc);
        smp->slot.state = _sg_create_sampler(smp, desc);
    } else {
        smp->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((smp->slot.state == SG_RESOURCESTATE_VALID)||(smp->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_init_shader(_sg_shader_t* shd, const sg_shader_desc* desc) {
    SOKOL_ASSERT(shd && (shd->slot.state == SG_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    if (_sg_validate_shader_desc(desc)) {
        _sg_shader_common_init(&shd->cmn, desc);
        shd->slot.state = _sg_create_shader(shd, desc);
    } else {
        shd->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((shd->slot.state == SG_RESOURCESTATE_VALID)||(shd->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_init_pipeline(_sg_pipeline_t* pip, const sg_pipeline_desc* desc) {
    SOKOL_ASSERT(pip && (pip->slot.state == SG_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    if (_sg_validate_pipeline_desc(desc)) {
        _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, desc->shader.id);
        if (shd && (shd->slot.state == SG_RESOURCESTATE_VALID)) {
            _sg_pipeline_common_init(&pip->cmn, desc);
            pip->slot.state = _sg_create_pipeline(pip, shd, desc);
        } else {
            pip->slot.state = SG_RESOURCESTATE_FAILED;
        }
    } else {
        pip->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((pip->slot.state == SG_RESOURCESTATE_VALID)||(pip->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_init_attachments(_sg_attachments_t* atts, const sg_attachments_desc* desc) {
    SOKOL_ASSERT(atts && atts->slot.state == SG_RESOURCESTATE_ALLOC);
    SOKOL_ASSERT(desc);
    if (_sg_validate_attachments_desc(desc)) {
        // lookup pass attachment image pointers
        _sg_image_t* color_images[SG_MAX_COLOR_ATTACHMENTS] = { 0 };
        _sg_image_t* resolve_images[SG_MAX_COLOR_ATTACHMENTS] = { 0 };
        _sg_image_t* ds_image = 0;
        // NOTE: validation already checked that all surfaces are same width/height
        int width = 0;
        int height = 0;
        for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            if (desc->colors[i].image.id) {
                color_images[i] = _sg_lookup_image(&_sg.pools, desc->colors[i].image.id);
                if (!(color_images[i] && color_images[i]->slot.state == SG_RESOURCESTATE_VALID)) {
                    atts->slot.state = SG_RESOURCESTATE_FAILED;
                    return;
                }
                const int mip_level = desc->colors[i].mip_level;
                width = _sg_miplevel_dim(color_images[i]->cmn.width, mip_level);
                height = _sg_miplevel_dim(color_images[i]->cmn.height, mip_level);
            }
            if (desc->resolves[i].image.id) {
                resolve_images[i] = _sg_lookup_image(&_sg.pools, desc->resolves[i].image.id);
                if (!(resolve_images[i] && resolve_images[i]->slot.state == SG_RESOURCESTATE_VALID)) {
                    atts->slot.state = SG_RESOURCESTATE_FAILED;
                    return;
                }
            }
        }
        if (desc->depth_stencil.image.id) {
            ds_image = _sg_lookup_image(&_sg.pools, desc->depth_stencil.image.id);
            if (!(ds_image && ds_image->slot.state == SG_RESOURCESTATE_VALID)) {
                atts->slot.state = SG_RESOURCESTATE_FAILED;
                return;
            }
            const int mip_level = desc->depth_stencil.mip_level;
            width = _sg_miplevel_dim(ds_image->cmn.width, mip_level);
            height = _sg_miplevel_dim(ds_image->cmn.height, mip_level);
        }
        _sg_attachments_common_init(&atts->cmn, desc, width, height);
        atts->slot.state = _sg_create_attachments(atts, color_images, resolve_images, ds_image, desc);
    } else {
        atts->slot.state = SG_RESOURCESTATE_FAILED;
    }
    SOKOL_ASSERT((atts->slot.state == SG_RESOURCESTATE_VALID)||(atts->slot.state == SG_RESOURCESTATE_FAILED));
}

_SOKOL_PRIVATE void _sg_uninit_buffer(_sg_buffer_t* buf) {
    SOKOL_ASSERT(buf && ((buf->slot.state == SG_RESOURCESTATE_VALID) || (buf->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_buffer(buf);
    _sg_reset_buffer_to_alloc_state(buf);
}

_SOKOL_PRIVATE void _sg_uninit_image(_sg_image_t* img) {
    SOKOL_ASSERT(img && ((img->slot.state == SG_RESOURCESTATE_VALID) || (img->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_image(img);
    _sg_reset_image_to_alloc_state(img);
}

_SOKOL_PRIVATE void _sg_uninit_sampler(_sg_sampler_t* smp) {
    SOKOL_ASSERT(smp && ((smp->slot.state == SG_RESOURCESTATE_VALID) || (smp->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_sampler(smp);
    _sg_reset_sampler_to_alloc_state(smp);
}

_SOKOL_PRIVATE void _sg_uninit_shader(_sg_shader_t* shd) {
    SOKOL_ASSERT(shd && ((shd->slot.state == SG_RESOURCESTATE_VALID) || (shd->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_shader(shd);
    _sg_reset_shader_to_alloc_state(shd);
}

_SOKOL_PRIVATE void _sg_uninit_pipeline(_sg_pipeline_t* pip) {
    SOKOL_ASSERT(pip && ((pip->slot.state == SG_RESOURCESTATE_VALID) || (pip->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_pipeline(pip);
    _sg_reset_pipeline_to_alloc_state(pip);
}

_SOKOL_PRIVATE void _sg_uninit_attachments(_sg_attachments_t* atts) {
    SOKOL_ASSERT(atts && ((atts->slot.state == SG_RESOURCESTATE_VALID) || (atts->slot.state == SG_RESOURCESTATE_FAILED)));
    _sg_discard_attachments(atts);
    _sg_reset_attachments_to_alloc_state(atts);
}

_SOKOL_PRIVATE void _sg_setup_commit_listeners(const sg_desc* desc) {
    SOKOL_ASSERT(desc->max_commit_listeners > 0);
    SOKOL_ASSERT(0 == _sg.commit_listeners.items);
    SOKOL_ASSERT(0 == _sg.commit_listeners.num);
    SOKOL_ASSERT(0 == _sg.commit_listeners.upper);
    _sg.commit_listeners.num = desc->max_commit_listeners;
    const size_t size = (size_t)_sg.commit_listeners.num * sizeof(sg_commit_listener);
    _sg.commit_listeners.items = (sg_commit_listener*)_sg_malloc_clear(size);
}

_SOKOL_PRIVATE void _sg_discard_commit_listeners(void) {
    SOKOL_ASSERT(0 != _sg.commit_listeners.items);
    _sg_free(_sg.commit_listeners.items);
    _sg.commit_listeners.items = 0;
}

_SOKOL_PRIVATE void _sg_notify_commit_listeners(void) {
    SOKOL_ASSERT(_sg.commit_listeners.items);
    for (int i = 0; i < _sg.commit_listeners.upper; i++) {
        const sg_commit_listener* listener = &_sg.commit_listeners.items[i];
        if (listener->func) {
            listener->func(listener->user_data);
        }
    }
}

_SOKOL_PRIVATE bool _sg_add_commit_listener(const sg_commit_listener* new_listener) {
    SOKOL_ASSERT(new_listener && new_listener->func);
    SOKOL_ASSERT(_sg.commit_listeners.items);
    // first check if the listener hadn't been added already
    for (int i = 0; i < _sg.commit_listeners.upper; i++) {
        const sg_commit_listener* slot = &_sg.commit_listeners.items[i];
        if ((slot->func == new_listener->func) && (slot->user_data == new_listener->user_data)) {
            _SG_ERROR(IDENTICAL_COMMIT_LISTENER);
            return false;
        }
    }
    // first try to plug a hole
    sg_commit_listener* slot = 0;
    for (int i = 0; i < _sg.commit_listeners.upper; i++) {
        if (_sg.commit_listeners.items[i].func == 0) {
            slot = &_sg.commit_listeners.items[i];
            break;
        }
    }
    if (!slot) {
        // append to end
        if (_sg.commit_listeners.upper < _sg.commit_listeners.num) {
            slot = &_sg.commit_listeners.items[_sg.commit_listeners.upper++];
        }
    }
    if (!slot) {
        _SG_ERROR(COMMIT_LISTENER_ARRAY_FULL);
        return false;
    }
    *slot = *new_listener;
    return true;
}

_SOKOL_PRIVATE bool _sg_remove_commit_listener(const sg_commit_listener* listener) {
    SOKOL_ASSERT(listener && listener->func);
    SOKOL_ASSERT(_sg.commit_listeners.items);
    for (int i = 0; i < _sg.commit_listeners.upper; i++) {
        sg_commit_listener* slot = &_sg.commit_listeners.items[i];
        // both the function pointer and user data must match!
        if ((slot->func == listener->func) && (slot->user_data == listener->user_data)) {
            slot->func = 0;
            slot->user_data = 0;
            // NOTE: since _sg_add_commit_listener() already catches duplicates,
            // we don't need to worry about them here
            return true;
        }
    }
    return false;
}

_SOKOL_PRIVATE sg_desc _sg_desc_defaults(const sg_desc* desc) {
    /*
        NOTE: on WebGPU, the default color pixel format MUST be provided,
        cannot be a default compile-time constant.
    */
    sg_desc res = *desc;
    #if defined(SOKOL_WGPU)
        SOKOL_ASSERT(SG_PIXELFORMAT_NONE < res.environment.defaults.color_format);
    #elif defined(SOKOL_METAL) || defined(SOKOL_D3D11)
        res.environment.defaults.color_format = _sg_def(res.environment.defaults.color_format, SG_PIXELFORMAT_BGRA8);
    #else
        res.environment.defaults.color_format = _sg_def(res.environment.defaults.color_format, SG_PIXELFORMAT_RGBA8);
    #endif
    res.environment.defaults.depth_format = _sg_def(res.environment.defaults.depth_format, SG_PIXELFORMAT_DEPTH_STENCIL);
    res.environment.defaults.sample_count = _sg_def(res.environment.defaults.sample_count, 1);
    res.buffer_pool_size = _sg_def(res.buffer_pool_size, _SG_DEFAULT_BUFFER_POOL_SIZE);
    res.image_pool_size = _sg_def(res.image_pool_size, _SG_DEFAULT_IMAGE_POOL_SIZE);
    res.sampler_pool_size = _sg_def(res.sampler_pool_size, _SG_DEFAULT_SAMPLER_POOL_SIZE);
    res.shader_pool_size = _sg_def(res.shader_pool_size, _SG_DEFAULT_SHADER_POOL_SIZE);
    res.pipeline_pool_size = _sg_def(res.pipeline_pool_size, _SG_DEFAULT_PIPELINE_POOL_SIZE);
    res.attachments_pool_size = _sg_def(res.attachments_pool_size, _SG_DEFAULT_ATTACHMENTS_POOL_SIZE);
    res.uniform_buffer_size = _sg_def(res.uniform_buffer_size, _SG_DEFAULT_UB_SIZE);
    res.max_commit_listeners = _sg_def(res.max_commit_listeners, _SG_DEFAULT_MAX_COMMIT_LISTENERS);
    return res;
}

_SOKOL_PRIVATE sg_pass _sg_pass_defaults(const sg_pass* pass) {
    sg_pass res = *pass;
    if (res.attachments.id == SG_INVALID_ID) {
        // this is a swapchain-pass
        res.swapchain.sample_count = _sg_def(res.swapchain.sample_count, _sg.desc.environment.defaults.sample_count);
        res.swapchain.color_format = _sg_def(res.swapchain.color_format, _sg.desc.environment.defaults.color_format);
        res.swapchain.depth_format = _sg_def(res.swapchain.depth_format, _sg.desc.environment.defaults.depth_format);
    }
    res.action = _sg_pass_action_defaults(&res.action);
    return res;
}

// ██████  ██    ██ ██████  ██      ██  ██████
// ██   ██ ██    ██ ██   ██ ██      ██ ██
// ██████  ██    ██ ██████  ██      ██ ██
// ██      ██    ██ ██   ██ ██      ██ ██
// ██       ██████  ██████  ███████ ██  ██████
//
// >>public
SOKOL_API_IMPL void sg_setup(const sg_desc* desc) {
    SOKOL_ASSERT(desc);
    SOKOL_ASSERT((desc->_start_canary == 0) && (desc->_end_canary == 0));
    SOKOL_ASSERT((desc->allocator.alloc_fn && desc->allocator.free_fn) || (!desc->allocator.alloc_fn && !desc->allocator.free_fn));
    _SG_CLEAR_ARC_STRUCT(_sg_state_t, _sg);
    _sg.desc = _sg_desc_defaults(desc);
    _sg_setup_pools(&_sg.pools, &_sg.desc);
    _sg_setup_commit_listeners(&_sg.desc);
    _sg.frame_index = 1;
    _sg.stats_enabled = true;
    _sg_setup_backend(&_sg.desc);
    _sg.valid = true;
}

SOKOL_API_IMPL void sg_shutdown(void) {
    _sg_discard_all_resources(&_sg.pools);
    _sg_discard_backend();
    _sg_discard_commit_listeners();
    _sg_discard_pools(&_sg.pools);
    _SG_CLEAR_ARC_STRUCT(_sg_state_t, _sg);
}

SOKOL_API_IMPL bool sg_isvalid(void) {
    return _sg.valid;
}

SOKOL_API_IMPL sg_desc sg_query_desc(void) {
    SOKOL_ASSERT(_sg.valid);
    return _sg.desc;
}

SOKOL_API_IMPL sg_backend sg_query_backend(void) {
    SOKOL_ASSERT(_sg.valid);
    return _sg.backend;
}

SOKOL_API_IMPL sg_features sg_query_features(void) {
    SOKOL_ASSERT(_sg.valid);
    return _sg.features;
}

SOKOL_API_IMPL sg_limits sg_query_limits(void) {
    SOKOL_ASSERT(_sg.valid);
    return _sg.limits;
}

SOKOL_API_IMPL sg_pixelformat_info sg_query_pixelformat(sg_pixel_format fmt) {
    SOKOL_ASSERT(_sg.valid);
    int fmt_index = (int) fmt;
    SOKOL_ASSERT((fmt_index > SG_PIXELFORMAT_NONE) && (fmt_index < _SG_PIXELFORMAT_NUM));
    const _sg_pixelformat_info_t* src = &_sg.formats[fmt_index];
    sg_pixelformat_info res;
    _sg_clear(&res, sizeof(res));
    res.sample = src->sample;
    res.filter = src->filter;
    res.render = src->render;
    res.blend = src->blend;
    res.msaa = src->msaa;
    res.depth = src->depth;
    res.compressed = _sg_is_compressed_pixel_format(fmt);
    if (!res.compressed) {
        res.bytes_per_pixel = _sg_pixelformat_bytesize(fmt);
    }
    return res;
}

SOKOL_API_IMPL int sg_query_row_pitch(sg_pixel_format fmt, int width, int row_align_bytes) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(width > 0);
    SOKOL_ASSERT((row_align_bytes > 0) && _sg_ispow2(row_align_bytes));
    SOKOL_ASSERT(((int)fmt > SG_PIXELFORMAT_NONE) && ((int)fmt < _SG_PIXELFORMAT_NUM));
    return _sg_row_pitch(fmt, width, row_align_bytes);
}

SOKOL_API_IMPL int sg_query_surface_pitch(sg_pixel_format fmt, int width, int height, int row_align_bytes) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT((width > 0) && (height > 0));
    SOKOL_ASSERT((row_align_bytes > 0) && _sg_ispow2(row_align_bytes));
    SOKOL_ASSERT(((int)fmt > SG_PIXELFORMAT_NONE) && ((int)fmt < _SG_PIXELFORMAT_NUM));
    return _sg_surface_pitch(fmt, width, height, row_align_bytes);
}

SOKOL_API_IMPL sg_frame_stats sg_query_frame_stats(void) {
    SOKOL_ASSERT(_sg.valid);
    return _sg.prev_stats;
}

SOKOL_API_IMPL sg_trace_hooks sg_install_trace_hooks(const sg_trace_hooks* trace_hooks) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(trace_hooks);
    _SOKOL_UNUSED(trace_hooks);
    #if defined(SOKOL_TRACE_HOOKS)
        sg_trace_hooks old_hooks = _sg.hooks;
        _sg.hooks = *trace_hooks;
    #else
        static sg_trace_hooks old_hooks;
        _SG_WARN(TRACE_HOOKS_NOT_ENABLED);
    #endif
    return old_hooks;
}

SOKOL_API_IMPL sg_buffer sg_alloc_buffer(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_buffer res = _sg_alloc_buffer();
    _SG_TRACE_ARGS(alloc_buffer, res);
    return res;
}

SOKOL_API_IMPL sg_image sg_alloc_image(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_image res = _sg_alloc_image();
    _SG_TRACE_ARGS(alloc_image, res);
    return res;
}

SOKOL_API_IMPL sg_sampler sg_alloc_sampler(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_sampler res = _sg_alloc_sampler();
    _SG_TRACE_ARGS(alloc_sampler, res);
    return res;
}

SOKOL_API_IMPL sg_shader sg_alloc_shader(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_shader res = _sg_alloc_shader();
    _SG_TRACE_ARGS(alloc_shader, res);
    return res;
}

SOKOL_API_IMPL sg_pipeline sg_alloc_pipeline(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_pipeline res = _sg_alloc_pipeline();
    _SG_TRACE_ARGS(alloc_pipeline, res);
    return res;
}

SOKOL_API_IMPL sg_attachments sg_alloc_attachments(void) {
    SOKOL_ASSERT(_sg.valid);
    sg_attachments res = _sg_alloc_attachments();
    _SG_TRACE_ARGS(alloc_attachments, res);
    return res;
}

SOKOL_API_IMPL void sg_dealloc_buffer(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        if (buf->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_buffer(buf);
        } else {
            _SG_ERROR(DEALLOC_BUFFER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_buffer, buf_id);
}

SOKOL_API_IMPL void sg_dealloc_image(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        if (img->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_image(img);
        } else {
            _SG_ERROR(DEALLOC_IMAGE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_image, img_id);
}

SOKOL_API_IMPL void sg_dealloc_sampler(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        if (smp->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_sampler(smp);
        } else {
            _SG_ERROR(DEALLOC_SAMPLER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_sampler, smp_id);
}

SOKOL_API_IMPL void sg_dealloc_shader(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        if (shd->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_shader(shd);
        } else {
            _SG_ERROR(DEALLOC_SHADER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_shader, shd_id);
}

SOKOL_API_IMPL void sg_dealloc_pipeline(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        if (pip->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_pipeline(pip);
        } else {
            _SG_ERROR(DEALLOC_PIPELINE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_pipeline, pip_id);
}

SOKOL_API_IMPL void sg_dealloc_attachments(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        if (atts->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_attachments(atts);
        } else {
            _SG_ERROR(DEALLOC_ATTACHMENTS_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(dealloc_attachments, atts_id);
}

SOKOL_API_IMPL void sg_init_buffer(sg_buffer buf_id, const sg_buffer_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_buffer_desc desc_def = _sg_buffer_desc_defaults(desc);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        if (buf->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_buffer(buf, &desc_def);
            SOKOL_ASSERT((buf->slot.state == SG_RESOURCESTATE_VALID) || (buf->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_BUFFER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_buffer, buf_id, &desc_def);
}

SOKOL_API_IMPL void sg_init_image(sg_image img_id, const sg_image_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_image_desc desc_def = _sg_image_desc_defaults(desc);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        if (img->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_image(img, &desc_def);
            SOKOL_ASSERT((img->slot.state == SG_RESOURCESTATE_VALID) || (img->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_IMAGE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_image, img_id, &desc_def);
}

SOKOL_API_IMPL void sg_init_sampler(sg_sampler smp_id, const sg_sampler_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_sampler_desc desc_def = _sg_sampler_desc_defaults(desc);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        if (smp->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_sampler(smp, &desc_def);
            SOKOL_ASSERT((smp->slot.state == SG_RESOURCESTATE_VALID) || (smp->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_SAMPLER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_sampler, smp_id, &desc_def);
}

SOKOL_API_IMPL void sg_init_shader(sg_shader shd_id, const sg_shader_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_shader_desc desc_def = _sg_shader_desc_defaults(desc);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        if (shd->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_shader(shd, &desc_def);
            SOKOL_ASSERT((shd->slot.state == SG_RESOURCESTATE_VALID) || (shd->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_SHADER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_shader, shd_id, &desc_def);
}

SOKOL_API_IMPL void sg_init_pipeline(sg_pipeline pip_id, const sg_pipeline_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_pipeline_desc desc_def = _sg_pipeline_desc_defaults(desc);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        if (pip->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_pipeline(pip, &desc_def);
            SOKOL_ASSERT((pip->slot.state == SG_RESOURCESTATE_VALID) || (pip->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_PIPELINE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_pipeline, pip_id, &desc_def);
}

SOKOL_API_IMPL void sg_init_attachments(sg_attachments atts_id, const sg_attachments_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    sg_attachments_desc desc_def = _sg_attachments_desc_defaults(desc);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        if (atts->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_init_attachments(atts, &desc_def);
            SOKOL_ASSERT((atts->slot.state == SG_RESOURCESTATE_VALID) || (atts->slot.state == SG_RESOURCESTATE_FAILED));
        } else {
            _SG_ERROR(INIT_ATTACHMENTS_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(init_attachments, atts_id, &desc_def);
}

SOKOL_API_IMPL void sg_uninit_buffer(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        if ((buf->slot.state == SG_RESOURCESTATE_VALID) || (buf->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_buffer(buf);
            SOKOL_ASSERT(buf->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_BUFFER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_buffer, buf_id);
}

SOKOL_API_IMPL void sg_uninit_image(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        if ((img->slot.state == SG_RESOURCESTATE_VALID) || (img->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_image(img);
            SOKOL_ASSERT(img->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_IMAGE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_image, img_id);
}

SOKOL_API_IMPL void sg_uninit_sampler(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        if ((smp->slot.state == SG_RESOURCESTATE_VALID) || (smp->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_sampler(smp);
            SOKOL_ASSERT(smp->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_SAMPLER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_sampler, smp_id);
}

SOKOL_API_IMPL void sg_uninit_shader(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        if ((shd->slot.state == SG_RESOURCESTATE_VALID) || (shd->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_shader(shd);
            SOKOL_ASSERT(shd->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_SHADER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_shader, shd_id);
}

SOKOL_API_IMPL void sg_uninit_pipeline(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        if ((pip->slot.state == SG_RESOURCESTATE_VALID) || (pip->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_pipeline(pip);
            SOKOL_ASSERT(pip->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_PIPELINE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_pipeline, pip_id);
}

SOKOL_API_IMPL void sg_uninit_attachments(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        if ((atts->slot.state == SG_RESOURCESTATE_VALID) || (atts->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_attachments(atts);
            SOKOL_ASSERT(atts->slot.state == SG_RESOURCESTATE_ALLOC);
        } else {
            _SG_ERROR(UNINIT_ATTACHMENTS_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(uninit_attachments, atts_id);
}

SOKOL_API_IMPL void sg_fail_buffer(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        if (buf->slot.state == SG_RESOURCESTATE_ALLOC) {
            buf->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_BUFFER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_buffer, buf_id);
}

SOKOL_API_IMPL void sg_fail_image(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        if (img->slot.state == SG_RESOURCESTATE_ALLOC) {
            img->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_IMAGE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_image, img_id);
}

SOKOL_API_IMPL void sg_fail_sampler(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        if (smp->slot.state == SG_RESOURCESTATE_ALLOC) {
            smp->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_SAMPLER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_sampler, smp_id);
}

SOKOL_API_IMPL void sg_fail_shader(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        if (shd->slot.state == SG_RESOURCESTATE_ALLOC) {
            shd->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_SHADER_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_shader, shd_id);
}

SOKOL_API_IMPL void sg_fail_pipeline(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        if (pip->slot.state == SG_RESOURCESTATE_ALLOC) {
            pip->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_PIPELINE_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_pipeline, pip_id);
}

SOKOL_API_IMPL void sg_fail_attachments(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        if (atts->slot.state == SG_RESOURCESTATE_ALLOC) {
            atts->slot.state = SG_RESOURCESTATE_FAILED;
        } else {
            _SG_ERROR(FAIL_ATTACHMENTS_INVALID_STATE);
        }
    }
    _SG_TRACE_ARGS(fail_attachments, atts_id);
}

SOKOL_API_IMPL sg_resource_state sg_query_buffer_state(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    sg_resource_state res = buf ? buf->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_resource_state sg_query_image_state(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    sg_resource_state res = img ? img->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_resource_state sg_query_sampler_state(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    sg_resource_state res = smp ? smp->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_resource_state sg_query_shader_state(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    sg_resource_state res = shd ? shd->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_resource_state sg_query_pipeline_state(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    sg_resource_state res = pip ? pip->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_resource_state sg_query_attachments_state(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    sg_resource_state res = atts ? atts->slot.state : SG_RESOURCESTATE_INVALID;
    return res;
}

SOKOL_API_IMPL sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_buffer_desc desc_def = _sg_buffer_desc_defaults(desc);
    sg_buffer buf_id = _sg_alloc_buffer();
    if (buf_id.id != SG_INVALID_ID) {
        _sg_buffer_t* buf = _sg_buffer_at(&_sg.pools, buf_id.id);
        SOKOL_ASSERT(buf && (buf->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_buffer(buf, &desc_def);
        SOKOL_ASSERT((buf->slot.state == SG_RESOURCESTATE_VALID) || (buf->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_buffer, &desc_def, buf_id);
    return buf_id;
}

SOKOL_API_IMPL sg_image sg_make_image(const sg_image_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_image_desc desc_def = _sg_image_desc_defaults(desc);
    sg_image img_id = _sg_alloc_image();
    if (img_id.id != SG_INVALID_ID) {
        _sg_image_t* img = _sg_image_at(&_sg.pools, img_id.id);
        SOKOL_ASSERT(img && (img->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_image(img, &desc_def);
        SOKOL_ASSERT((img->slot.state == SG_RESOURCESTATE_VALID) || (img->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_image, &desc_def, img_id);
    return img_id;
}

SOKOL_API_IMPL sg_sampler sg_make_sampler(const sg_sampler_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_sampler_desc desc_def = _sg_sampler_desc_defaults(desc);
    sg_sampler smp_id = _sg_alloc_sampler();
    if (smp_id.id != SG_INVALID_ID) {
        _sg_sampler_t* smp = _sg_sampler_at(&_sg.pools, smp_id.id);
        SOKOL_ASSERT(smp && (smp->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_sampler(smp, &desc_def);
        SOKOL_ASSERT((smp->slot.state == SG_RESOURCESTATE_VALID) || (smp->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_sampler, &desc_def, smp_id);
    return smp_id;
}

SOKOL_API_IMPL sg_shader sg_make_shader(const sg_shader_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_shader_desc desc_def = _sg_shader_desc_defaults(desc);
    sg_shader shd_id = _sg_alloc_shader();
    if (shd_id.id != SG_INVALID_ID) {
        _sg_shader_t* shd = _sg_shader_at(&_sg.pools, shd_id.id);
        SOKOL_ASSERT(shd && (shd->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_shader(shd, &desc_def);
        SOKOL_ASSERT((shd->slot.state == SG_RESOURCESTATE_VALID) || (shd->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_shader, &desc_def, shd_id);
    return shd_id;
}

SOKOL_API_IMPL sg_pipeline sg_make_pipeline(const sg_pipeline_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_pipeline_desc desc_def = _sg_pipeline_desc_defaults(desc);
    sg_pipeline pip_id = _sg_alloc_pipeline();
    if (pip_id.id != SG_INVALID_ID) {
        _sg_pipeline_t* pip = _sg_pipeline_at(&_sg.pools, pip_id.id);
        SOKOL_ASSERT(pip && (pip->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_pipeline(pip, &desc_def);
        SOKOL_ASSERT((pip->slot.state == SG_RESOURCESTATE_VALID) || (pip->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_pipeline, &desc_def, pip_id);
    return pip_id;
}

SOKOL_API_IMPL sg_attachments sg_make_attachments(const sg_attachments_desc* desc) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(desc);
    sg_attachments_desc desc_def = _sg_attachments_desc_defaults(desc);
    sg_attachments atts_id = _sg_alloc_attachments();
    if (atts_id.id != SG_INVALID_ID) {
        _sg_attachments_t* atts = _sg_attachments_at(&_sg.pools, atts_id.id);
        SOKOL_ASSERT(atts && (atts->slot.state == SG_RESOURCESTATE_ALLOC));
        _sg_init_attachments(atts, &desc_def);
        SOKOL_ASSERT((atts->slot.state == SG_RESOURCESTATE_VALID) || (atts->slot.state == SG_RESOURCESTATE_FAILED));
    }
    _SG_TRACE_ARGS(make_attachments, &desc_def, atts_id);
    return atts_id;
}

SOKOL_API_IMPL void sg_destroy_buffer(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_buffer, buf_id);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        if ((buf->slot.state == SG_RESOURCESTATE_VALID) || (buf->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_buffer(buf);
            SOKOL_ASSERT(buf->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (buf->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_buffer(buf);
            SOKOL_ASSERT(buf->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_destroy_image(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_image, img_id);
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        if ((img->slot.state == SG_RESOURCESTATE_VALID) || (img->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_image(img);
            SOKOL_ASSERT(img->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (img->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_image(img);
            SOKOL_ASSERT(img->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_destroy_sampler(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_sampler, smp_id);
    _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        if ((smp->slot.state == SG_RESOURCESTATE_VALID) || (smp->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_sampler(smp);
            SOKOL_ASSERT(smp->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (smp->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_sampler(smp);
            SOKOL_ASSERT(smp->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_destroy_shader(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_shader, shd_id);
    _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        if ((shd->slot.state == SG_RESOURCESTATE_VALID) || (shd->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_shader(shd);
            SOKOL_ASSERT(shd->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (shd->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_shader(shd);
            SOKOL_ASSERT(shd->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_destroy_pipeline(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_pipeline, pip_id);
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        if ((pip->slot.state == SG_RESOURCESTATE_VALID) || (pip->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_pipeline(pip);
            SOKOL_ASSERT(pip->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (pip->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_pipeline(pip);
            SOKOL_ASSERT(pip->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_destroy_attachments(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    _SG_TRACE_ARGS(destroy_attachments, atts_id);
    _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        if ((atts->slot.state == SG_RESOURCESTATE_VALID) || (atts->slot.state == SG_RESOURCESTATE_FAILED)) {
            _sg_uninit_attachments(atts);
            SOKOL_ASSERT(atts->slot.state == SG_RESOURCESTATE_ALLOC);
        }
        if (atts->slot.state == SG_RESOURCESTATE_ALLOC) {
            _sg_dealloc_attachments(atts);
            SOKOL_ASSERT(atts->slot.state == SG_RESOURCESTATE_INITIAL);
        }
    }
}

SOKOL_API_IMPL void sg_begin_pass(const sg_pass* pass) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(!_sg.cur_pass.valid);
    SOKOL_ASSERT(!_sg.cur_pass.in_pass);
    SOKOL_ASSERT(pass);
    SOKOL_ASSERT((pass->_start_canary == 0) && (pass->_end_canary == 0));
    const sg_pass pass_def = _sg_pass_defaults(pass);
    if (!_sg_validate_begin_pass(&pass_def)) {
        return;
    }
    if (pass_def.attachments.id != SG_INVALID_ID) {
        // an offscreen pass
        SOKOL_ASSERT(_sg.cur_pass.atts == 0);
        _sg.cur_pass.atts = _sg_lookup_attachments(&_sg.pools, pass_def.attachments.id);
        if (0 == _sg.cur_pass.atts) {
            _SG_ERROR(BEGINPASS_ATTACHMENT_INVALID);
            return;
        }
        _sg.cur_pass.atts_id = pass_def.attachments;
        _sg.cur_pass.width = _sg.cur_pass.atts->cmn.width;
        _sg.cur_pass.height = _sg.cur_pass.atts->cmn.height;
    } else {
        // a swapchain pass
        SOKOL_ASSERT(pass_def.swapchain.width > 0);
        SOKOL_ASSERT(pass_def.swapchain.height > 0);
        SOKOL_ASSERT(pass_def.swapchain.color_format > SG_PIXELFORMAT_NONE);
        SOKOL_ASSERT(pass_def.swapchain.sample_count > 0);
        _sg.cur_pass.width = pass_def.swapchain.width;
        _sg.cur_pass.height = pass_def.swapchain.height;
        _sg.cur_pass.swapchain.color_fmt = pass_def.swapchain.color_format;
        _sg.cur_pass.swapchain.depth_fmt = pass_def.swapchain.depth_format;
        _sg.cur_pass.swapchain.sample_count = pass_def.swapchain.sample_count;
    }
    _sg.cur_pass.valid = true;  // may be overruled by backend begin-pass functions
    _sg.cur_pass.in_pass = true;
    _sg_begin_pass(&pass_def);
    _SG_TRACE_ARGS(begin_pass, &pass_def);
}

SOKOL_API_IMPL void sg_apply_viewport(int x, int y, int width, int height, bool origin_top_left) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    _sg_stats_add(num_apply_viewport, 1);
    if (!_sg.cur_pass.valid) {
        return;
    }
    _sg_apply_viewport(x, y, width, height, origin_top_left);
    _SG_TRACE_ARGS(apply_viewport, x, y, width, height, origin_top_left);
}

SOKOL_API_IMPL void sg_apply_viewportf(float x, float y, float width, float height, bool origin_top_left) {
    sg_apply_viewport((int)x, (int)y, (int)width, (int)height, origin_top_left);
}

SOKOL_API_IMPL void sg_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    _sg_stats_add(num_apply_scissor_rect, 1);
    if (!_sg.cur_pass.valid) {
        return;
    }
    _sg_apply_scissor_rect(x, y, width, height, origin_top_left);
    _SG_TRACE_ARGS(apply_scissor_rect, x, y, width, height, origin_top_left);
}

SOKOL_API_IMPL void sg_apply_scissor_rectf(float x, float y, float width, float height, bool origin_top_left) {
    sg_apply_scissor_rect((int)x, (int)y, (int)width, (int)height, origin_top_left);
}

SOKOL_API_IMPL void sg_apply_pipeline(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    _sg_stats_add(num_apply_pipeline, 1);
    if (!_sg_validate_apply_pipeline(pip_id)) {
        _sg.next_draw_valid = false;
        return;
    }
    if (!_sg.cur_pass.valid) {
        return;
    }
    _sg.cur_pipeline = pip_id;
    _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    SOKOL_ASSERT(pip);
    _sg.next_draw_valid = (SG_RESOURCESTATE_VALID == pip->slot.state);
    SOKOL_ASSERT(pip->shader && (pip->shader->slot.id == pip->cmn.shader_id.id));
    _sg_apply_pipeline(pip);
    _SG_TRACE_ARGS(apply_pipeline, pip_id);
}

SOKOL_API_IMPL void sg_apply_bindings(const sg_bindings* bindings) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    SOKOL_ASSERT(bindings);
    SOKOL_ASSERT((bindings->_start_canary == 0) && (bindings->_end_canary==0));
    _sg_stats_add(num_apply_bindings, 1);
    if (!_sg_validate_apply_bindings(bindings)) {
        _sg.next_draw_valid = false;
        return;
    }
    if (!_sg.cur_pass.valid) {
        return;
    }

    _sg_bindings_t bnd;
    _sg_clear(&bnd, sizeof(bnd));
    bnd.pip = _sg_lookup_pipeline(&_sg.pools, _sg.cur_pipeline.id);
    if (0 == bnd.pip) {
        _sg.next_draw_valid = false;
    }

    for (int i = 0; i < SG_MAX_VERTEX_BUFFERS; i++, bnd.num_vbs++) {
        if (bindings->vertex_buffers[i].id) {
            bnd.vbs[i] = _sg_lookup_buffer(&_sg.pools, bindings->vertex_buffers[i].id);
            bnd.vb_offsets[i] = bindings->vertex_buffer_offsets[i];
            if (bnd.vbs[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.vbs[i]->slot.state);
                _sg.next_draw_valid &= !bnd.vbs[i]->cmn.append_overflow;
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    if (bindings->index_buffer.id) {
        bnd.ib = _sg_lookup_buffer(&_sg.pools, bindings->index_buffer.id);
        bnd.ib_offset = bindings->index_buffer_offset;
        if (bnd.ib) {
            _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.ib->slot.state);
            _sg.next_draw_valid &= !bnd.ib->cmn.append_overflow;
        } else {
            _sg.next_draw_valid = false;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++, bnd.num_vs_imgs++) {
        if (bindings->vs.images[i].id) {
            bnd.vs_imgs[i] = _sg_lookup_image(&_sg.pools, bindings->vs.images[i].id);
            if (bnd.vs_imgs[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.vs_imgs[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_SAMPLERS; i++, bnd.num_vs_smps++) {
        if (bindings->vs.samplers[i].id) {
            bnd.vs_smps[i] = _sg_lookup_sampler(&_sg.pools, bindings->vs.samplers[i].id);
            if (bnd.vs_smps[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.vs_smps[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++, bnd.num_vs_sbufs++) {
        if (bindings->vs.storage_buffers[i].id) {
            bnd.vs_sbufs[i] = _sg_lookup_buffer(&_sg.pools, bindings->vs.storage_buffers[i].id);
            if (bnd.vs_sbufs[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.vs_sbufs[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_IMAGES; i++, bnd.num_fs_imgs++) {
        if (bindings->fs.images[i].id) {
            bnd.fs_imgs[i] = _sg_lookup_image(&_sg.pools, bindings->fs.images[i].id);
            if (bnd.fs_imgs[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.fs_imgs[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_SAMPLERS; i++, bnd.num_fs_smps++) {
        if (bindings->fs.samplers[i].id) {
            bnd.fs_smps[i] = _sg_lookup_sampler(&_sg.pools, bindings->fs.samplers[i].id);
            if (bnd.fs_smps[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.fs_smps[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }

    for (int i = 0; i < SG_MAX_SHADERSTAGE_STORAGEBUFFERS; i++, bnd.num_fs_sbufs++) {
        if (bindings->fs.storage_buffers[i].id) {
            bnd.fs_sbufs[i] = _sg_lookup_buffer(&_sg.pools, bindings->fs.storage_buffers[i].id);
            if (bnd.fs_sbufs[i]) {
                _sg.next_draw_valid &= (SG_RESOURCESTATE_VALID == bnd.fs_sbufs[i]->slot.state);
            } else {
                _sg.next_draw_valid = false;
            }
        } else {
            break;
        }
    }
    if (_sg.next_draw_valid) {
        _sg.next_draw_valid &= _sg_apply_bindings(&bnd);
        _SG_TRACE_ARGS(apply_bindings, bindings);
    }
}

SOKOL_API_IMPL void sg_apply_uniforms(sg_shader_stage stage, int ub_index, const sg_range* data) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    SOKOL_ASSERT((stage == SG_SHADERSTAGE_VS) || (stage == SG_SHADERSTAGE_FS));
    SOKOL_ASSERT((ub_index >= 0) && (ub_index < SG_MAX_SHADERSTAGE_UBS));
    SOKOL_ASSERT(data && data->ptr && (data->size > 0));
    _sg_stats_add(num_apply_uniforms, 1);
    _sg_stats_add(size_apply_uniforms, (uint32_t)data->size);
    if (!_sg_validate_apply_uniforms(stage, ub_index, data)) {
        _sg.next_draw_valid = false;
        return;
    }
    if (!_sg.cur_pass.valid) {
        return;
    }
    if (!_sg.next_draw_valid) {
        return;
    }
    _sg_apply_uniforms(stage, ub_index, data);
    _SG_TRACE_ARGS(apply_uniforms, stage, ub_index, data);
}

SOKOL_API_IMPL void sg_draw(int base_element, int num_elements, int num_instances) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    SOKOL_ASSERT(base_element >= 0);
    SOKOL_ASSERT(num_elements >= 0);
    SOKOL_ASSERT(num_instances >= 0);
    _sg_stats_add(num_draw, 1);
    if (!_sg.cur_pass.valid) {
        return;
    }
    if (!_sg.next_draw_valid) {
        return;
    }
    /* attempting to draw with zero elements or instances is not technically an
       error, but might be handled as an error in the backend API (e.g. on Metal)
    */
    if ((0 == num_elements) || (0 == num_instances)) {
        return;
    }
    _sg_draw(base_element, num_elements, num_instances);
    _SG_TRACE_ARGS(draw, base_element, num_elements, num_instances);
}

SOKOL_API_IMPL void sg_end_pass(void) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(_sg.cur_pass.in_pass);
    _sg_stats_add(num_passes, 1);
    // NOTE: don't exit early if !_sg.cur_pass.valid
    _sg_end_pass();
    _sg.cur_pipeline.id = SG_INVALID_ID;
    _sg_clear(&_sg.cur_pass, sizeof(_sg.cur_pass));
    _SG_TRACE_NOARGS(end_pass);
}

SOKOL_API_IMPL void sg_commit(void) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(!_sg.cur_pass.valid);
    SOKOL_ASSERT(!_sg.cur_pass.in_pass);
    _sg_commit();
    _sg.stats.frame_index = _sg.frame_index;
    _sg.prev_stats = _sg.stats;
    _sg_clear(&_sg.stats, sizeof(_sg.stats));
    _sg_notify_commit_listeners();
    _SG_TRACE_NOARGS(commit);
    _sg.frame_index++;
}

SOKOL_API_IMPL void sg_reset_state_cache(void) {
    SOKOL_ASSERT(_sg.valid);
    _sg_reset_state_cache();
    _SG_TRACE_NOARGS(reset_state_cache);
}

SOKOL_API_IMPL void sg_update_buffer(sg_buffer buf_id, const sg_range* data) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(data && data->ptr && (data->size > 0));
    _sg_stats_add(num_update_buffer, 1);
    _sg_stats_add(size_update_buffer, (uint32_t)data->size);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if ((data->size > 0) && buf && (buf->slot.state == SG_RESOURCESTATE_VALID)) {
        if (_sg_validate_update_buffer(buf, data)) {
            SOKOL_ASSERT(data->size <= (size_t)buf->cmn.size);
            // only one update allowed per buffer and frame
            SOKOL_ASSERT(buf->cmn.update_frame_index != _sg.frame_index);
            // update and append on same buffer in same frame not allowed
            SOKOL_ASSERT(buf->cmn.append_frame_index != _sg.frame_index);
            _sg_update_buffer(buf, data);
            buf->cmn.update_frame_index = _sg.frame_index;
        }
    }
    _SG_TRACE_ARGS(update_buffer, buf_id, data);
}

SOKOL_API_IMPL int sg_append_buffer(sg_buffer buf_id, const sg_range* data) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(data && data->ptr);
    _sg_stats_add(num_append_buffer, 1);
    _sg_stats_add(size_append_buffer, (uint32_t)data->size);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    int result;
    if (buf) {
        // rewind append cursor in a new frame
        if (buf->cmn.append_frame_index != _sg.frame_index) {
            buf->cmn.append_pos = 0;
            buf->cmn.append_overflow = false;
        }
        if (((size_t)buf->cmn.append_pos + data->size) > (size_t)buf->cmn.size) {
            buf->cmn.append_overflow = true;
        }
        const int start_pos = buf->cmn.append_pos;
        // NOTE: the multiple-of-4 requirement for the buffer offset is coming
        // from WebGPU, but we want identical behaviour between backends
        SOKOL_ASSERT(_sg_multiple_u64((uint64_t)start_pos, 4));
        if (buf->slot.state == SG_RESOURCESTATE_VALID) {
            if (_sg_validate_append_buffer(buf, data)) {
                if (!buf->cmn.append_overflow && (data->size > 0)) {
                    // update and append on same buffer in same frame not allowed
                    SOKOL_ASSERT(buf->cmn.update_frame_index != _sg.frame_index);
                    _sg_append_buffer(buf, data, buf->cmn.append_frame_index != _sg.frame_index);
                    buf->cmn.append_pos += (int) _sg_roundup_u64(data->size, 4);
                    buf->cmn.append_frame_index = _sg.frame_index;
                }
            }
        }
        result = start_pos;
    } else {
        // FIXME: should we return -1 here?
        result = 0;
    }
    _SG_TRACE_ARGS(append_buffer, buf_id, data, result);
    return result;
}

SOKOL_API_IMPL bool sg_query_buffer_overflow(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    bool result = buf ? buf->cmn.append_overflow : false;
    return result;
}

SOKOL_API_IMPL bool sg_query_buffer_will_overflow(sg_buffer buf_id, size_t size) {
    SOKOL_ASSERT(_sg.valid);
    _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    bool result = false;
    if (buf) {
        int append_pos = buf->cmn.append_pos;
        // rewind append cursor in a new frame
        if (buf->cmn.append_frame_index != _sg.frame_index) {
            append_pos = 0;
        }
        if ((append_pos + _sg_roundup((int)size, 4)) > buf->cmn.size) {
            result = true;
        }
    }
    return result;
}

SOKOL_API_IMPL void sg_update_image(sg_image img_id, const sg_image_data* data) {
    SOKOL_ASSERT(_sg.valid);
    _sg_stats_add(num_update_image, 1);
    for (int face_index = 0; face_index < SG_CUBEFACE_NUM; face_index++) {
        for (int mip_index = 0; mip_index < SG_MAX_MIPMAPS; mip_index++) {
            if (data->subimage[face_index][mip_index].size == 0) {
                break;
            }
            _sg_stats_add(size_update_image, (uint32_t)data->subimage[face_index][mip_index].size);
        }
    }
    _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
        if (_sg_validate_update_image(img, data)) {
            SOKOL_ASSERT(img->cmn.upd_frame_index != _sg.frame_index);
            _sg_update_image(img, data);
            img->cmn.upd_frame_index = _sg.frame_index;
        }
    }
    _SG_TRACE_ARGS(update_image, img_id, data);
}

SOKOL_API_IMPL void sg_push_debug_group(const char* name) {
    SOKOL_ASSERT(_sg.valid);
    SOKOL_ASSERT(name);
    _sg_push_debug_group(name);
    _SG_TRACE_ARGS(push_debug_group, name);
}

SOKOL_API_IMPL void sg_pop_debug_group(void) {
    SOKOL_ASSERT(_sg.valid);
    _sg_pop_debug_group();
    _SG_TRACE_NOARGS(pop_debug_group);
}

SOKOL_API_IMPL bool sg_add_commit_listener(sg_commit_listener listener) {
    SOKOL_ASSERT(_sg.valid);
    return _sg_add_commit_listener(&listener);
}

SOKOL_API_IMPL bool sg_remove_commit_listener(sg_commit_listener listener) {
    SOKOL_ASSERT(_sg.valid);
    return _sg_remove_commit_listener(&listener);
}

SOKOL_API_IMPL void sg_enable_frame_stats(void) {
    SOKOL_ASSERT(_sg.valid);
    _sg.stats_enabled = true;
}

SOKOL_API_IMPL void sg_disable_frame_stats(void) {
    SOKOL_ASSERT(_sg.valid);
    _sg.stats_enabled = false;
}

SOKOL_API_IMPL bool sg_frame_stats_enabled(void) {
    return _sg.stats_enabled;
}

SOKOL_API_IMPL sg_buffer_info sg_query_buffer_info(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_buffer_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        info.slot.state = buf->slot.state;
        info.slot.res_id = buf->slot.id;
        info.update_frame_index = buf->cmn.update_frame_index;
        info.append_frame_index = buf->cmn.append_frame_index;
        info.append_pos = buf->cmn.append_pos;
        info.append_overflow = buf->cmn.append_overflow;
        #if defined(SOKOL_D3D11)
        info.num_slots = 1;
        info.active_slot = 0;
        #else
        info.num_slots = buf->cmn.num_slots;
        info.active_slot = buf->cmn.active_slot;
        #endif
    }
    return info;
}

SOKOL_API_IMPL sg_image_info sg_query_image_info(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_image_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        info.slot.state = img->slot.state;
        info.slot.res_id = img->slot.id;
        info.upd_frame_index = img->cmn.upd_frame_index;
        #if defined(SOKOL_D3D11)
        info.num_slots = 1;
        info.active_slot = 0;
        #else
        info.num_slots = img->cmn.num_slots;
        info.active_slot = img->cmn.active_slot;
        #endif
    }
    return info;
}

SOKOL_API_IMPL sg_sampler_info sg_query_sampler_info(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_sampler_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        info.slot.state = smp->slot.state;
        info.slot.res_id = smp->slot.id;
    }
    return info;
}

SOKOL_API_IMPL sg_shader_info sg_query_shader_info(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_shader_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        info.slot.state = shd->slot.state;
        info.slot.res_id = shd->slot.id;
    }
    return info;
}

SOKOL_API_IMPL sg_pipeline_info sg_query_pipeline_info(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_pipeline_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        info.slot.state = pip->slot.state;
        info.slot.res_id = pip->slot.id;
    }
    return info;
}

SOKOL_API_IMPL sg_attachments_info sg_query_attachments_info(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_attachments_info info;
    _sg_clear(&info, sizeof(info));
    const _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        info.slot.state = atts->slot.state;
        info.slot.res_id = atts->slot.id;
    }
    return info;
}

SOKOL_API_IMPL sg_buffer_desc sg_query_buffer_desc(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_buffer_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
    if (buf) {
        desc.size = (size_t)buf->cmn.size;
        desc.type = buf->cmn.type;
        desc.usage = buf->cmn.usage;
    }
    return desc;
}

SOKOL_API_IMPL sg_image_desc sg_query_image_desc(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_image_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
    if (img) {
        desc.type = img->cmn.type;
        desc.render_target = img->cmn.render_target;
        desc.width = img->cmn.width;
        desc.height = img->cmn.height;
        desc.num_slices = img->cmn.num_slices;
        desc.num_mipmaps = img->cmn.num_mipmaps;
        desc.usage = img->cmn.usage;
        desc.pixel_format = img->cmn.pixel_format;
        desc.sample_count = img->cmn.sample_count;
    }
    return desc;
}

SOKOL_API_IMPL sg_sampler_desc sg_query_sampler_desc(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_sampler_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
    if (smp) {
        desc.min_filter = smp->cmn.min_filter;
        desc.mag_filter = smp->cmn.mag_filter;
        desc.mipmap_filter = smp->cmn.mipmap_filter;
        desc.wrap_u = smp->cmn.wrap_u;
        desc.wrap_v = smp->cmn.wrap_v;
        desc.wrap_w = smp->cmn.wrap_w;
        desc.min_lod = smp->cmn.min_lod;
        desc.max_lod = smp->cmn.max_lod;
        desc.border_color = smp->cmn.border_color;
        desc.compare = smp->cmn.compare;
        desc.max_anisotropy = smp->cmn.max_anisotropy;
    }
    return desc;
}

SOKOL_API_IMPL sg_shader_desc sg_query_shader_desc(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_shader_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
    if (shd) {
        for (int stage_idx = 0; stage_idx < SG_NUM_SHADER_STAGES; stage_idx++) {
            sg_shader_stage_desc* stage_desc = (stage_idx == 0) ? &desc.vs : &desc.fs;
            const _sg_shader_stage_t* stage = &shd->cmn.stage[stage_idx];
            for (int ub_idx = 0; ub_idx < stage->num_uniform_blocks; ub_idx++) {
                sg_shader_uniform_block_desc* ub_desc = &stage_desc->uniform_blocks[ub_idx];
                const _sg_shader_uniform_block_t* ub = &stage->uniform_blocks[ub_idx];
                ub_desc->size = ub->size;
            }
            for (int img_idx = 0; img_idx < stage->num_images; img_idx++) {
                sg_shader_image_desc* img_desc = &stage_desc->images[img_idx];
                const _sg_shader_image_t* img = &stage->images[img_idx];
                img_desc->used = true;
                img_desc->image_type = img->image_type;
                img_desc->sample_type = img->sample_type;
                img_desc->multisampled = img->multisampled;
            }
            for (int smp_idx = 0; smp_idx < stage->num_samplers; smp_idx++) {
                sg_shader_sampler_desc* smp_desc = &stage_desc->samplers[smp_idx];
                const _sg_shader_sampler_t* smp = &stage->samplers[smp_idx];
                smp_desc->used = true;
                smp_desc->sampler_type = smp->sampler_type;
            }
            for (int img_smp_idx = 0; img_smp_idx < stage->num_image_samplers; img_smp_idx++) {
                sg_shader_image_sampler_pair_desc* img_smp_desc = &stage_desc->image_sampler_pairs[img_smp_idx];
                const _sg_shader_image_sampler_t* img_smp = &stage->image_samplers[img_smp_idx];
                img_smp_desc->used = true;
                img_smp_desc->image_slot = img_smp->image_slot;
                img_smp_desc->sampler_slot = img_smp->sampler_slot;
                img_smp_desc->glsl_name = 0;
            }
        }
    }
    return desc;
}

SOKOL_API_IMPL sg_pipeline_desc sg_query_pipeline_desc(sg_pipeline pip_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_pipeline_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_pipeline_t* pip = _sg_lookup_pipeline(&_sg.pools, pip_id.id);
    if (pip) {
        desc.shader = pip->cmn.shader_id;
        desc.layout = pip->cmn.layout;
        desc.depth = pip->cmn.depth;
        desc.stencil = pip->cmn.stencil;
        desc.color_count = pip->cmn.color_count;
        for (int i = 0; i < pip->cmn.color_count; i++) {
            desc.colors[i] = pip->cmn.colors[i];
        }
        desc.primitive_type = pip->cmn.primitive_type;
        desc.index_type = pip->cmn.index_type;
        desc.cull_mode = pip->cmn.cull_mode;
        desc.face_winding = pip->cmn.face_winding;
        desc.sample_count = pip->cmn.sample_count;
        desc.blend_color = pip->cmn.blend_color;
        desc.alpha_to_coverage_enabled = pip->cmn.alpha_to_coverage_enabled;
    }
    return desc;
}

SOKOL_API_IMPL sg_attachments_desc sg_query_attachments_desc(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_attachments_desc desc;
    _sg_clear(&desc, sizeof(desc));
    const _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
    if (atts) {
        for (int i = 0; i < atts->cmn.num_colors; i++) {
            desc.colors[i].image = atts->cmn.colors[i].image_id;
            desc.colors[i].mip_level = atts->cmn.colors[i].mip_level;
            desc.colors[i].slice = atts->cmn.colors[i].slice;
        }
        desc.depth_stencil.image = atts->cmn.depth_stencil.image_id;
        desc.depth_stencil.mip_level = atts->cmn.depth_stencil.mip_level;
        desc.depth_stencil.slice = atts->cmn.depth_stencil.slice;
    }
    return desc;
}

SOKOL_API_IMPL sg_buffer_desc sg_query_buffer_defaults(const sg_buffer_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_buffer_desc_defaults(desc);
}

SOKOL_API_IMPL sg_image_desc sg_query_image_defaults(const sg_image_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_image_desc_defaults(desc);
}

SOKOL_API_IMPL sg_sampler_desc sg_query_sampler_defaults(const sg_sampler_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_sampler_desc_defaults(desc);
}

SOKOL_API_IMPL sg_shader_desc sg_query_shader_defaults(const sg_shader_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_shader_desc_defaults(desc);
}

SOKOL_API_IMPL sg_pipeline_desc sg_query_pipeline_defaults(const sg_pipeline_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_pipeline_desc_defaults(desc);
}

SOKOL_API_IMPL sg_attachments_desc sg_query_attachments_defaults(const sg_attachments_desc* desc) {
    SOKOL_ASSERT(_sg.valid && desc);
    return _sg_attachments_desc_defaults(desc);
}

SOKOL_API_IMPL sg_gl_buffer_info sg_gl_query_buffer_info(sg_buffer buf_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_gl_buffer_info res;
    _sg_clear(&res, sizeof(res));
    #if defined(_SOKOL_ANY_GL)
        const _sg_buffer_t* buf = _sg_lookup_buffer(&_sg.pools, buf_id.id);
        if (buf) {
            for (int i = 0; i < SG_NUM_INFLIGHT_FRAMES; i++) {
                res.buf[i] = buf->gl.buf[i];
            }
            res.active_slot = buf->cmn.active_slot;
        }
    #else
        _SOKOL_UNUSED(buf_id);
    #endif
    return res;
}

SOKOL_API_IMPL sg_gl_image_info sg_gl_query_image_info(sg_image img_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_gl_image_info res;
    _sg_clear(&res, sizeof(res));
    #if defined(_SOKOL_ANY_GL)
        const _sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
        if (img) {
            for (int i = 0; i < SG_NUM_INFLIGHT_FRAMES; i++) {
                res.tex[i] = img->gl.tex[i];
            }
            res.tex_target = img->gl.target;
            res.msaa_render_buffer = img->gl.msaa_render_buffer;
            res.active_slot = img->cmn.active_slot;
        }
    #else
        _SOKOL_UNUSED(img_id);
    #endif
    return res;
}

SOKOL_API_IMPL sg_gl_sampler_info sg_gl_query_sampler_info(sg_sampler smp_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_gl_sampler_info res;
    _sg_clear(&res, sizeof(res));
    #if defined(_SOKOL_ANY_GL)
        const _sg_sampler_t* smp = _sg_lookup_sampler(&_sg.pools, smp_id.id);
        if (smp) {
            res.smp = smp->gl.smp;
        }
    #else
        _SOKOL_UNUSED(smp_id);
    #endif
    return res;
}

SOKOL_API_IMPL sg_gl_shader_info sg_gl_query_shader_info(sg_shader shd_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_gl_shader_info res;
    _sg_clear(&res, sizeof(res));
    #if defined(_SOKOL_ANY_GL)
        const _sg_shader_t* shd = _sg_lookup_shader(&_sg.pools, shd_id.id);
        if (shd) {
            res.prog = shd->gl.prog;
        }
    #else
        _SOKOL_UNUSED(shd_id);
    #endif
    return res;
}

SOKOL_API_IMPL sg_gl_attachments_info sg_gl_query_attachments_info(sg_attachments atts_id) {
    SOKOL_ASSERT(_sg.valid);
    sg_gl_attachments_info res;
    _sg_clear(&res, sizeof(res));
    #if defined(_SOKOL_ANY_GL)
        const _sg_attachments_t* atts = _sg_lookup_attachments(&_sg.pools, atts_id.id);
        if (atts) {
            res.framebuffer = atts->gl.fb;
            for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                res.msaa_resolve_framebuffer[i] = atts->gl.msaa_resolve_framebuffer[i];
            }
        }
    #else
        _SOKOL_UNUSED(atts_id);
    #endif
    return res;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // SOKOL_GFX_IMPL
