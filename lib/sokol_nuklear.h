#ifndef SOKOL_NUKLEAR_INCLUDED
#define SOKOL_NUKLEAR_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if !defined(SOKOL_GFX_INCLUDED)
#error "Please include sokol_gfx.h before sokol_nuklear.h"
#endif
#if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP) && !defined(SOKOL_APP_INCLUDED)
#error "Please include sokol_app.h before sokol_nuklear.h"
#endif
#if !defined(NK_UNDEFINED)
#error "Please include nuklear.h before sokol_nuklear.h"
#endif

#if defined(SOKOL_API_DECL) && !defined(SOKOL_NUKLEAR_API_DECL)
#define SOKOL_NUKLEAR_API_DECL SOKOL_API_DECL
#endif
#ifndef SOKOL_NUKLEAR_API_DECL
#if defined(_WIN32) && defined(SOKOL_DLL) && defined(SOKOL_NUKLEAR_IMPL)
#define SOKOL_NUKLEAR_API_DECL __declspec(dllexport)
#elif defined(_WIN32) && defined(SOKOL_DLL)
#define SOKOL_NUKLEAR_API_DECL __declspec(dllimport)
#else
#define SOKOL_NUKLEAR_API_DECL extern
#endif
#endif

enum {
    SNK_INVALID_ID = 0,
};

/*
    snk_image_t

    A combined image-sampler pair used to inject custom images and samplers into Nuklear

    Create with snk_make_image(), and convert to an nk_handle via snk_nkhandle().
*/
typedef struct snk_image_t { uint32_t id; } snk_image_t;

/*
    snk_image_desc_t

    Descriptor struct for snk_make_image(). You must provide
    at least an sg_image handle. Keeping the sg_sampler handle
    zero-initialized will select the builtin default sampler
    which uses linear filtering.
*/
typedef struct snk_image_desc_t {
    sg_image image;
    sg_sampler sampler;
} snk_image_desc_t;

/*
    snk_log_item

    An enum with a unique item for each log message, warning, error
    and validation layer message.
*/
#define _SNK_LOG_ITEMS \
    _SNK_LOGITEM_XMACRO(OK, "Ok") \
    _SNK_LOGITEM_XMACRO(MALLOC_FAILED, "memory allocation failed") \
    _SNK_LOGITEM_XMACRO(IMAGE_POOL_EXHAUSTED, "image pool exhausted") \

#define _SNK_LOGITEM_XMACRO(item,msg) SNK_LOGITEM_##item,
typedef enum snk_log_item_t {
    _SNK_LOG_ITEMS
} snk_log_item_t;
#undef _SNK_LOGITEM_XMACRO

/*
    snk_allocator_t

    Used in snk_desc_t to provide custom memory-alloc and -free functions
    to sokol_nuklear.h. If memory management should be overridden, both the
    alloc_fn and free_fn function must be provided (e.g. it's not valid to
    override one function but not the other).
*/
typedef struct snk_allocator_t {
    void* (*alloc_fn)(size_t size, void* user_data);
    void (*free_fn)(void* ptr, void* user_data);
    void* user_data;
} snk_allocator_t;

/*
    snk_logger

    Used in snk_desc_t to provide a logging function. Please be aware
    that without logging function, sokol-nuklear will be completely
    silent, e.g. it will not report errors, warnings and
    validation layer messages. For maximum error verbosity,
    compile in debug mode (e.g. NDEBUG *not* defined) and install
    a logger (for instance the standard logging function from sokol_log.h).
*/
typedef struct snk_logger_t {
    void (*func)(
        const char* tag,                // always "snk"
        uint32_t log_level,             // 0=panic, 1=error, 2=warning, 3=info
        uint32_t log_item_id,           // SNK_LOGITEM_*
        const char* message_or_null,    // a message string, may be nullptr in release mode
        uint32_t line_nr,               // line number in sokol_imgui.h
        const char* filename_or_null,   // source filename, may be nullptr in release mode
        void* user_data);
    void* user_data;
} snk_logger_t;


typedef struct snk_desc_t {
    int max_vertices;                   // default: 65536
    int image_pool_size;                // default: 256
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    int sample_count;
    float dpi_scale;
    bool no_default_font;
    snk_allocator_t allocator;          // optional memory allocation overrides (default: malloc/free)
    snk_logger_t logger;                // optional log function override
} snk_desc_t;

SOKOL_NUKLEAR_API_DECL void snk_setup(const snk_desc_t* desc);
SOKOL_NUKLEAR_API_DECL struct nk_context* snk_get_context(void);
SOKOL_NUKLEAR_API_DECL struct nk_context* snk_new_frame(void);
SOKOL_NUKLEAR_API_DECL void snk_render(int width, int height);
SOKOL_NUKLEAR_API_DECL snk_image_t snk_make_image(const snk_image_desc_t* desc);
SOKOL_NUKLEAR_API_DECL void snk_destroy_image(snk_image_t img);
SOKOL_NUKLEAR_API_DECL snk_image_desc_t snk_query_image_desc(snk_image_t img);
SOKOL_NUKLEAR_API_DECL nk_handle snk_nkhandle(snk_image_t img);
SOKOL_NUKLEAR_API_DECL snk_image_t snk_image_from_nkhandle(nk_handle handle);
#if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
SOKOL_NUKLEAR_API_DECL bool snk_handle_event(const sapp_event* ev);
SOKOL_NUKLEAR_API_DECL nk_flags snk_edit_string(struct nk_context *ctx, nk_flags flags, char *memory, int *len, int max, nk_plugin_filter filter);
#endif
SOKOL_NUKLEAR_API_DECL void snk_shutdown(void);

#endif /* SOKOL_NUKLEAR_INCLUDED */

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef SOKOL_NUKLEAR_IMPL
#define SOKOL_NUKLEAR_IMPL_INCLUDED (1)

#if !defined(NK_INCLUDE_VERTEX_BUFFER_OUTPUT)
#error "Please ensure that NK_INCLUDE_VERTEX_BUFFER_OUTPUT is #defined before including nuklear.h"
#endif

#ifdef __cplusplus
#error "The sokol_nuklear.h implementation must be compiled as C."
#endif

#include <stdlib.h>
#include <string.h> // memset

#if defined(__EMSCRIPTEN__) && !defined(SOKOL_NUKLEAR_NO_SOKOL_APP) && !defined(SOKOL_DUMMY_BACKEND)
#include <emscripten.h>
#endif

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
#ifndef _SOKOL_PRIVATE
    #if defined(__GNUC__) || defined(__clang__)
        #define _SOKOL_PRIVATE __attribute__((unused)) static
    #else
        #define _SOKOL_PRIVATE static
    #endif
#endif

#define _SNK_INIT_COOKIE (0xBABEBABE)
#define _SNK_INVALID_SLOT_INDEX (0)
#define _SNK_SLOT_SHIFT (16)
#define _SNK_MAX_POOL_SIZE (1<<_SNK_SLOT_SHIFT)
#define _SNK_SLOT_MASK (_SNK_MAX_POOL_SIZE-1)

// helper macros
#define _snk_def(val, def) (((val) == 0) ? (def) : (val))

typedef struct _snk_vertex_t {
    float pos[2];
    float uv[2];
    uint8_t col[4];
} _snk_vertex_t;

typedef struct _snk_vs_params_t {
    float   disp_size[2];
    uint8_t _pad_8[8];
} _snk_vs_params_t;

typedef enum {
    _SNK_RESOURCESTATE_INITIAL,
    _SNK_RESOURCESTATE_ALLOC,
    _SNK_RESOURCESTATE_VALID,
    _SNK_RESOURCESTATE_FAILED,
    _SNK_RESOURCESTATE_INVALID,
    _SNK_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} _snk_resource_state;

typedef struct {
    uint32_t id;
    _snk_resource_state state;
} _snk_slot_t;

typedef struct {
    int size;
    int queue_top;
    uint32_t* gen_ctrs;
    int* free_queue;
} _snk_pool_t;

typedef struct {
    _snk_slot_t slot;
    sg_image image;
    sg_sampler sampler;
} _snk_image_t;

typedef struct {
    _snk_pool_t pool;
    _snk_image_t* items;
} _snk_image_pool_t;

typedef struct {
    uint32_t init_cookie;
    snk_desc_t desc;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    _snk_vs_params_t vs_params;
    size_t vertex_buffer_size;
    size_t index_buffer_size;
    sg_buffer vbuf;
    sg_buffer ibuf;
    sg_image font_img;
    sg_sampler font_smp;
    snk_image_t default_font;
    sg_image def_img;
    sg_sampler def_smp;
    sg_shader shd;
    sg_pipeline pip;
    bool is_osx;    // true if running on OSX (or HTML5 OSX), needed for copy/paste
    _snk_image_pool_t image_pool;
    #if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
    int mouse_pos[2];
    float mouse_scroll[2];
    bool mouse_did_move;
    bool mouse_did_scroll;
    bool btn_down[NK_BUTTON_MAX];
    bool btn_up[NK_BUTTON_MAX];
    char char_buffer[NK_INPUT_MAX];
    bool keys_down[NK_KEY_MAX];
    bool keys_up[NK_KEY_MAX];
    #endif
} _snk_state_t;
static _snk_state_t _snuklear;

/*
    Embedded source code compiled with:

    sokol-shdc -i snuk.glsl -o snuk.h -l glsl410:glsl300es:hlsl4:metal_macos:metal_ios:metal_sim:wgsl -b

    @vs vs
    uniform vs_params {
        vec2 disp_size;
    };
    in vec2 position;
    in vec2 texcoord0;
    in vec4 color0;
    out vec2 uv;
    out vec4 color;
    void main() {
        gl_Position = vec4(((position/disp_size)-0.5)*vec2(2.0,-2.0), 0.5, 1.0);
        uv = texcoord0;
        color = color0;
    }
    @end

    @fs fs
    uniform texture2D tex;
    uniform sampler smp;
    in vec2 uv;
    in vec4 color;
    out vec4 frag_color;
    void main() {
        frag_color = texture(sampler2D(tex, smp), uv) * color;
    }
    @end

    @program snuk vs fs
*/
#if defined(SOKOL_GLCORE)
static const uint8_t _snk_vs_source_glsl410[383] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x34,0x31,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x76,0x65,0x63,0x34,0x20,0x76,0x73,0x5f,0x70,0x61,
    0x72,0x61,0x6d,0x73,0x5b,0x31,0x5d,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,
    0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,
    0x20,0x76,0x65,0x63,0x32,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,
    0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,
    0x3d,0x20,0x30,0x29,0x20,0x6f,0x75,0x74,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,
    0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,
    0x6e,0x20,0x3d,0x20,0x31,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x32,0x20,0x74,
    0x65,0x78,0x63,0x6f,0x6f,0x72,0x64,0x30,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,
    0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x31,0x29,0x20,0x6f,
    0x75,0x74,0x20,0x76,0x65,0x63,0x34,0x20,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x6c,
    0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,
    0x20,0x32,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,0x63,0x6f,0x6c,0x6f,
    0x72,0x30,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,
    0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x76,0x65,0x63,0x34,0x28,0x28,0x28,0x70,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x20,0x2f,0x20,0x76,0x73,0x5f,0x70,0x61,0x72,0x61,0x6d,0x73,
    0x5b,0x30,0x5d,0x2e,0x78,0x79,0x29,0x20,0x2d,0x20,0x76,0x65,0x63,0x32,0x28,0x30,
    0x2e,0x35,0x29,0x29,0x20,0x2a,0x20,0x76,0x65,0x63,0x32,0x28,0x32,0x2e,0x30,0x2c,
    0x20,0x2d,0x32,0x2e,0x30,0x29,0x2c,0x20,0x30,0x2e,0x35,0x2c,0x20,0x31,0x2e,0x30,
    0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x74,0x65,0x78,0x63,
    0x6f,0x6f,0x72,0x64,0x30,0x3b,0x0a,0x20,0x20,0x20,0x20,0x63,0x6f,0x6c,0x6f,0x72,
    0x20,0x3d,0x20,0x63,0x6f,0x6c,0x6f,0x72,0x30,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
static const uint8_t _snk_fs_source_glsl410[219] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x34,0x31,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x32,0x44,0x20,
    0x74,0x65,0x78,0x5f,0x73,0x6d,0x70,0x3b,0x0a,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,
    0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x6f,
    0x75,0x74,0x20,0x76,0x65,0x63,0x34,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,
    0x6f,0x72,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,
    0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x32,
    0x20,0x75,0x76,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,
    0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x31,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,
    0x34,0x20,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,
    0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x72,0x61,0x67,
    0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x74,0x65,0x78,0x74,0x75,0x72,0x65,
    0x28,0x74,0x65,0x78,0x5f,0x73,0x6d,0x70,0x2c,0x20,0x75,0x76,0x29,0x20,0x2a,0x20,
    0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
#endif

#if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
static void _snk_clipboard_copy(nk_handle usr, const char *text, int len) {
    (void)usr;
    if (len == 0) {
        return;
    }
    sapp_set_clipboard_string(text);
}

static void _snk_clipboard_paste(nk_handle usr, struct nk_text_edit *edit) {
    const char *text = sapp_get_clipboard_string();
    if (text) {
        nk_textedit_paste(edit, text, nk_strlen(text));
    }
    (void)usr;
}

#if defined(__EMSCRIPTEN__) && !defined(SOKOL_DUMMY_BACKEND)
EM_JS(int, snk_js_is_osx, (void), {
    if (navigator.userAgent.includes('Macintosh')) {
        return 1;
    } else {
        return 0;
    }
});
#endif

static bool _snk_is_osx(void) {
    #if defined(SOKOL_DUMMY_BACKEND)
        return false;
    #elif defined(__EMSCRIPTEN__)
        return snk_js_is_osx();
    #elif defined(__APPLE__)
        return true;
    #else
        return false;
    #endif
}
#endif // !SOKOL_NUKLEAR_NO_SOKOL_APP

// ██       ██████   ██████   ██████  ██ ███    ██  ██████
// ██      ██    ██ ██       ██       ██ ████   ██ ██
// ██      ██    ██ ██   ███ ██   ███ ██ ██ ██  ██ ██   ███
// ██      ██    ██ ██    ██ ██    ██ ██ ██  ██ ██ ██    ██
// ███████  ██████   ██████   ██████  ██ ██   ████  ██████
//
// >>logging
#if defined(SOKOL_DEBUG)
#define _SNK_LOGITEM_XMACRO(item,msg) #item ": " msg,
static const char* _snk_log_messages[] = {
    _SNK_LOG_ITEMS
};
#undef _SNK_LOGITEM_XMACRO
#endif // SOKOL_DEBUG

#define _SNK_PANIC(code) _snk_log(SNK_LOGITEM_ ##code, 0, 0, __LINE__)
#define _SNK_ERROR(code) _snk_log(SNK_LOGITEM_ ##code, 1, 0, __LINE__)
#define _SNK_WARN(code) _snk_log(SNK_LOGITEM_ ##code, 2, 0, __LINE__)
#define _SNK_INFO(code) _snk_log(SNK_LOGITEM_ ##code, 3, 0, __LINE__)
#define _SNK_LOGMSG(code,msg) _snk_log(SNK_LOGITEM_ ##code, 3, msg, __LINE__)

static void _snk_log(snk_log_item_t log_item, uint32_t log_level, const char* msg, uint32_t line_nr) {
    if (_snuklear.desc.logger.func) {
        const char* filename = 0;
        #if defined(SOKOL_DEBUG)
            filename = __FILE__;
            if (0 == msg) {
                msg = _snk_log_messages[log_item];
            }
        #endif
        _snuklear.desc.logger.func("snk", log_level, log_item, msg, line_nr, filename, _snuklear.desc.logger.user_data);
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
static void _snk_clear(void* ptr, size_t size) {
    SOKOL_ASSERT(ptr && (size > 0));
    memset(ptr, 0, size);
}

static void* _snk_malloc(size_t size) {
    SOKOL_ASSERT(size > 0);
    void* ptr;
    if (_snuklear.desc.allocator.alloc_fn) {
        ptr = _snuklear.desc.allocator.alloc_fn(size, _snuklear.desc.allocator.user_data);
    } else {
        ptr = malloc(size);
    }
    if (0 == ptr) {
        _SNK_PANIC(MALLOC_FAILED);
    }
    return ptr;
}

static void* _snk_malloc_clear(size_t size) {
    void* ptr = _snk_malloc(size);
    _snk_clear(ptr, size);
    return ptr;
}

static void _snk_free(void* ptr) {
    if (_snuklear.desc.allocator.free_fn) {
        _snuklear.desc.allocator.free_fn(ptr, _snuklear.desc.allocator.user_data);
    } else {
        free(ptr);
    }
}

// ██████   ██████   ██████  ██
// ██   ██ ██    ██ ██    ██ ██
// ██████  ██    ██ ██    ██ ██
// ██      ██    ██ ██    ██ ██
// ██       ██████   ██████  ███████
//
// >>pool
static void _snk_init_pool(_snk_pool_t* pool, int num) {
    SOKOL_ASSERT(pool && (num >= 1));
    // slot 0 is reserved for the 'invalid id', so bump the pool size by 1
    pool->size = num + 1;
    pool->queue_top = 0;
    // generation counters indexable by pool slot index, slot 0 is reserved
    size_t gen_ctrs_size = sizeof(uint32_t) * (size_t)pool->size;
    pool->gen_ctrs = (uint32_t*) _snk_malloc_clear(gen_ctrs_size);
    // it's not a bug to only reserve 'num' here
    pool->free_queue = (int*) _snk_malloc_clear(sizeof(int) * (size_t)num);
    // never allocate the zero-th pool item since the invalid id is 0
    for (int i = pool->size-1; i >= 1; i--) {
        pool->free_queue[pool->queue_top++] = i;
    }
}

static void _snk_discard_pool(_snk_pool_t* pool) {
    SOKOL_ASSERT(pool);
    SOKOL_ASSERT(pool->free_queue);
    _snk_free(pool->free_queue);
    pool->free_queue = 0;
    SOKOL_ASSERT(pool->gen_ctrs);
    _snk_free(pool->gen_ctrs);
    pool->gen_ctrs = 0;
    pool->size = 0;
    pool->queue_top = 0;
}

static int _snk_pool_alloc_index(_snk_pool_t* pool) {
    SOKOL_ASSERT(pool);
    SOKOL_ASSERT(pool->free_queue);
    if (pool->queue_top > 0) {
        int slot_index = pool->free_queue[--pool->queue_top];
        SOKOL_ASSERT((slot_index > 0) && (slot_index < pool->size));
        return slot_index;
    } else {
        // pool exhausted
        return _SNK_INVALID_SLOT_INDEX;
    }
}

static void _snk_pool_free_index(_snk_pool_t* pool, int slot_index) {
    SOKOL_ASSERT((slot_index > _SNK_INVALID_SLOT_INDEX) && (slot_index < pool->size));
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

/* initialize a pool slot:
    - bump the slot's generation counter
    - create a resource id from the generation counter and slot index
    - set the slot's id to this id
    - set the slot's state to ALLOC
    - return the handle id
*/
static uint32_t _snk_slot_init(_snk_pool_t* pool, _snk_slot_t* slot, int slot_index) {
    /* FIXME: add handling for an overflowing generation counter,
       for now, just overflow (another option is to disable
       the slot)
    */
    SOKOL_ASSERT(pool && pool->gen_ctrs);
    SOKOL_ASSERT((slot_index > _SNK_INVALID_SLOT_INDEX) && (slot_index < pool->size));
    SOKOL_ASSERT((slot->state == _SNK_RESOURCESTATE_INITIAL) && (slot->id == SNK_INVALID_ID));
    uint32_t ctr = ++pool->gen_ctrs[slot_index];
    slot->id = (ctr<<_SNK_SLOT_SHIFT)|(slot_index & _SNK_SLOT_MASK);
    slot->state = _SNK_RESOURCESTATE_ALLOC;
    return slot->id;
}

// extract slot index from id
static int _snk_slot_index(uint32_t id) {
    int slot_index = (int) (id & _SNK_SLOT_MASK);
    SOKOL_ASSERT(_SNK_INVALID_SLOT_INDEX != slot_index);
    return slot_index;
}

static void _snk_init_item_pool(_snk_pool_t* pool, int pool_size, void** items_ptr, size_t item_size_bytes) {
    // NOTE: the pools will have an additional item, since slot 0 is reserved
    SOKOL_ASSERT(pool && (pool->size == 0));
    SOKOL_ASSERT((pool_size > 0) && (pool_size < _SNK_MAX_POOL_SIZE));
    SOKOL_ASSERT(items_ptr && (*items_ptr == 0));
    SOKOL_ASSERT(item_size_bytes > 0);
    _snk_init_pool(pool, pool_size);
    const size_t pool_size_bytes = item_size_bytes * (size_t)pool->size;
    *items_ptr = _snk_malloc_clear(pool_size_bytes);
}

static void _snk_discard_item_pool(_snk_pool_t* pool, void** items_ptr) {
    SOKOL_ASSERT(pool && (pool->size != 0));
    SOKOL_ASSERT(items_ptr && (*items_ptr != 0));
    _snk_free(*items_ptr); *items_ptr = 0;
    _snk_discard_pool(pool);
}

static void _snk_setup_image_pool(int pool_size) {
    _snk_image_pool_t* p = &_snuklear.image_pool;
    _snk_init_item_pool(&p->pool, pool_size, (void**)&p->items, sizeof(_snk_image_t));
}

static void _snk_discard_image_pool(void) {
    _snk_image_pool_t* p = &_snuklear.image_pool;
    _snk_discard_item_pool(&p->pool, (void**)&p->items);
}

static snk_image_t _snk_make_image_handle(uint32_t id) {
    snk_image_t handle = { id };
    return handle;
}

static _snk_image_t* _snk_image_at(uint32_t id) {
    SOKOL_ASSERT(SNK_INVALID_ID != id);
    const _snk_image_pool_t* p = &_snuklear.image_pool;
    int slot_index = _snk_slot_index(id);
    SOKOL_ASSERT((slot_index > _SNK_INVALID_SLOT_INDEX) && (slot_index < p->pool.size));
    return &p->items[slot_index];
}

static _snk_image_t* _snk_lookup_image(uint32_t id) {
    if (SNK_INVALID_ID != id) {
        _snk_image_t* img = _snk_image_at(id);
        if (img->slot.id == id) {
            return img;
        }
    }
    return 0;
}

static snk_image_t _snk_alloc_image(void) {
    _snk_image_pool_t* p = &_snuklear.image_pool;
    int slot_index = _snk_pool_alloc_index(&p->pool);
    if (_SNK_INVALID_SLOT_INDEX != slot_index) {
        uint32_t id = _snk_slot_init(&p->pool, &p->items[slot_index].slot, slot_index);
        return _snk_make_image_handle(id);
    } else {
        // pool exhausted
        return _snk_make_image_handle(SNK_INVALID_ID);
    }
}

static _snk_resource_state _snk_init_image(_snk_image_t* img, const snk_image_desc_t* desc) {
    SOKOL_ASSERT(img && (img->slot.state == _SNK_RESOURCESTATE_ALLOC));
    SOKOL_ASSERT(desc);
    img->image = desc->image;
    img->sampler = desc->sampler;
    return _SNK_RESOURCESTATE_VALID;
}

static void _snk_deinit_image(_snk_image_t* img) {
    SOKOL_ASSERT(img);
    img->image.id = SNK_INVALID_ID;
    img->sampler.id = SNK_INVALID_ID;
}

static void _snk_destroy_image(snk_image_t img_id) {
    _snk_image_t* img = _snk_lookup_image(img_id.id);
    if (img) {
        _snk_deinit_image(img);
        _snk_image_pool_t* p = &_snuklear.image_pool;
        _snk_clear(img, sizeof(_snk_image_t));
        _snk_pool_free_index(&p->pool, _snk_slot_index(img_id.id));
    }
}

static void _snk_destroy_all_images(void) {
    _snk_image_pool_t* p = &_snuklear.image_pool;
    for (int i = 0; i < p->pool.size; i++) {
        _snk_image_t* img = &p->items[i];
        _snk_destroy_image(_snk_make_image_handle(img->slot.id));
    }
}

static snk_image_desc_t _snk_image_desc_defaults(const snk_image_desc_t* desc) {
    SOKOL_ASSERT(desc);
    snk_image_desc_t res = *desc;
    res.image.id = _snk_def(res.image.id, _snuklear.def_img.id);
    res.sampler.id = _snk_def(res.sampler.id, _snuklear.def_smp.id);
    return res;
}

static snk_desc_t _snk_desc_defaults(const snk_desc_t* desc) {
    SOKOL_ASSERT(desc);
    snk_desc_t res = *desc;
    res.max_vertices = _snk_def(res.max_vertices, 65536);
    res.dpi_scale = _snk_def(res.dpi_scale, 1.0f);
    res.image_pool_size = _snk_def(res.image_pool_size, 256);
    return res;
}

// ██████  ██    ██ ██████  ██      ██  ██████
// ██   ██ ██    ██ ██   ██ ██      ██ ██
// ██████  ██    ██ ██████  ██      ██ ██
// ██      ██    ██ ██   ██ ██      ██ ██
// ██       ██████  ██████  ███████ ██  ██████
//
// >>public
SOKOL_API_IMPL void snk_setup(const snk_desc_t* desc) {
    SOKOL_ASSERT(desc);
    _snk_clear(&_snuklear, sizeof(_snuklear));
    _snuklear.init_cookie = _SNK_INIT_COOKIE;
    _snuklear.desc = _snk_desc_defaults(desc);
    #if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
    _snuklear.is_osx = _snk_is_osx();
    #endif
    // can keep color_format, depth_format and sample_count as is,
    // since sokol_gfx.h will do its own default-value handling

    _snk_setup_image_pool(_snuklear.desc.image_pool_size);

    // initialize Nuklear
    nk_bool init_res = nk_init_default(&_snuklear.ctx, 0);
    SOKOL_ASSERT(1 == init_res); (void)init_res;    // silence unused warning in release mode
#if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
    _snuklear.ctx.clip.copy = _snk_clipboard_copy;
    _snuklear.ctx.clip.paste = _snk_clipboard_paste;
#endif

    // create sokol-gfx resources
    sg_push_debug_group("sokol-nuklear");

    // vertex buffer
    _snuklear.vertex_buffer_size = (size_t)_snuklear.desc.max_vertices * sizeof(_snk_vertex_t);
    _snuklear.vbuf = sg_make_buffer(&(sg_buffer_desc){
        .usage = SG_USAGE_STREAM,
        .size = _snuklear.vertex_buffer_size,
        .label = "sokol-nuklear-vertices"
    });

    // index buffer
    _snuklear.index_buffer_size = (size_t)_snuklear.desc.max_vertices * 3 * sizeof(uint16_t);
    _snuklear.ibuf = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_STREAM,
        .size = _snuklear.index_buffer_size,
        .label = "sokol-nuklear-indices"
    });

    // default font sampler
    _snuklear.font_smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .label = "sokol-nuklear-font-sampler",
    });

    // default user-image sampler
    _snuklear.def_smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
        .label = "sokol-nuklear-default-sampler",
    });

    // default font texture
    if (!_snuklear.desc.no_default_font) {
        nk_font_atlas_init_default(&_snuklear.atlas);
        nk_font_atlas_begin(&_snuklear.atlas);
        int font_width = 0, font_height = 0;
        const void* pixels = nk_font_atlas_bake(&_snuklear.atlas, &font_width, &font_height, NK_FONT_ATLAS_RGBA32);
        SOKOL_ASSERT((font_width > 0) && (font_height > 0));
        _snuklear.font_img = sg_make_image(&(sg_image_desc){
            .width = font_width,
            .height = font_height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .data.subimage[0][0] = {
                .ptr = pixels,
                .size = (size_t)(font_width * font_height) * sizeof(uint32_t)
            },
            .label = "sokol-nuklear-font"
        });
        _snuklear.default_font = snk_make_image(&(snk_image_desc_t){
            .image = _snuklear.font_img,
            .sampler = _snuklear.font_smp,
        });
        nk_font_atlas_end(&_snuklear.atlas, snk_nkhandle(_snuklear.default_font), 0);
        nk_font_atlas_cleanup(&_snuklear.atlas);
        if (_snuklear.atlas.default_font) {
            nk_style_set_font(&_snuklear.ctx, &_snuklear.atlas.default_font->handle);
        }
    }

    // default user image
    static uint32_t def_pixels[64];
    memset(def_pixels, 0xFF, sizeof(def_pixels));
    _snuklear.def_img = sg_make_image(&(sg_image_desc){
        .width = 8,
        .height = 8,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(def_pixels),
        .label = "sokol-nuklear-default-image",
    });

    // shader
        const char* vs_entry = "main";
        const char* fs_entry = "main";
    sg_range vs_bytecode = { .ptr = 0, .size = 0 };
    sg_range fs_bytecode = { .ptr = 0, .size = 0 };
    const char* vs_source = 0;
    const char* fs_source = 0;
    #if defined(SOKOL_GLCORE)
        vs_source = (const char*)_snk_vs_source_glsl410;
        fs_source = (const char*)_snk_fs_source_glsl410;
    #endif
    _snuklear.shd = sg_make_shader(&(sg_shader_desc){
        .attrs = {
            [0] = { .name = "position", .sem_name = "TEXCOORD", .sem_index = 0 },
            [1] = { .name = "texcoord0", .sem_name = "TEXCOORD", .sem_index = 1 },
            [2] = { .name = "color0", .sem_name = "TEXCOORD", .sem_index = 2 },
        },
        .vs = {
            .source = vs_source,
            .bytecode = vs_bytecode,
            .entry = vs_entry,
            .uniform_blocks[0] = {
                .size = sizeof(_snk_vs_params_t),
                .uniforms[0] = {
                    .name = "vs_params",
                    .type = SG_UNIFORMTYPE_FLOAT4,
                    .array_count = 1,
                }
            },
        },
        .fs = {
            .source = fs_source,
            .bytecode = fs_bytecode,
            .entry = fs_entry,
            .images[0] = { .used = true, .image_type = SG_IMAGETYPE_2D, .sample_type = SG_IMAGESAMPLETYPE_FLOAT },
            .samplers[0] = { .used = true, .sampler_type = SG_SAMPLERTYPE_FILTERING },
            .image_sampler_pairs[0] = { .used = true, .glsl_name = "tex_smp", .image_slot = 0, .sampler_slot = 0 },
        },
        .label = "sokol-nuklear-shader"
    });

    // pipeline object
    _snuklear.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .attrs = {
                [0] = { .offset = offsetof(_snk_vertex_t, pos), .format=SG_VERTEXFORMAT_FLOAT2 },
                [1] = { .offset = offsetof(_snk_vertex_t, uv), .format=SG_VERTEXFORMAT_FLOAT2 },
                [2] = { .offset = offsetof(_snk_vertex_t, col), .format=SG_VERTEXFORMAT_UBYTE4N }
            }
        },
        .shader = _snuklear.shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .sample_count = _snuklear.desc.sample_count,
        .depth.pixel_format = _snuklear.desc.depth_format,
        .colors[0] = {
            .pixel_format = _snuklear.desc.color_format,
            .write_mask = SG_COLORMASK_RGB,
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            }
        },
        .label = "sokol-nuklear-pipeline"
    });

    sg_pop_debug_group();
}

SOKOL_API_IMPL struct nk_context* snk_get_context(void) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    return &_snuklear.ctx;
}

SOKOL_API_IMPL void snk_shutdown(void) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    nk_free(&_snuklear.ctx);
    nk_font_atlas_clear(&_snuklear.atlas);

    // NOTE: it's valid to call the destroy funcs with SG_INVALID_ID
    sg_push_debug_group("sokol-nuklear");
    sg_destroy_pipeline(_snuklear.pip);
    sg_destroy_shader(_snuklear.shd);
    sg_destroy_sampler(_snuklear.font_smp);
    sg_destroy_image(_snuklear.font_img);
    sg_destroy_sampler(_snuklear.def_smp);
    sg_destroy_image(_snuklear.def_img);
    sg_destroy_buffer(_snuklear.ibuf);
    sg_destroy_buffer(_snuklear.vbuf);
    sg_pop_debug_group();
    _snk_destroy_all_images();
    _snk_discard_image_pool();
    _snuklear.init_cookie = 0;
}

SOKOL_API_IMPL struct nk_context* snk_new_frame(void) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    #if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
    nk_input_begin(&_snuklear.ctx);
    if (_snuklear.mouse_did_move) {
        nk_input_motion(&_snuklear.ctx, _snuklear.mouse_pos[0], _snuklear.mouse_pos[1]);
        _snuklear.mouse_did_move = false;
    }
    if (_snuklear.mouse_did_scroll) {
        nk_input_scroll(&_snuklear.ctx, nk_vec2(_snuklear.mouse_scroll[0], _snuklear.mouse_scroll[1]));
        _snuklear.mouse_did_scroll = false;
    }
    for (int i = 0; i < NK_BUTTON_MAX; i++) {
        if (_snuklear.btn_down[i]) {
            _snuklear.btn_down[i] = false;
            nk_input_button(&_snuklear.ctx, (enum nk_buttons)i, _snuklear.mouse_pos[0], _snuklear.mouse_pos[1], 1);
        } else if (_snuklear.btn_up[i]) {
            _snuklear.btn_up[i] = false;
            nk_input_button(&_snuklear.ctx, (enum nk_buttons)i, _snuklear.mouse_pos[0], _snuklear.mouse_pos[1], 0);
        }
    }
    const size_t char_buffer_len = strlen(_snuklear.char_buffer);
    if (char_buffer_len > 0) {
        for (size_t i = 0; i < char_buffer_len; i++) {
            nk_input_char(&_snuklear.ctx, _snuklear.char_buffer[i]);
        }
        _snk_clear(_snuklear.char_buffer, NK_INPUT_MAX);
    }
    for (int i = 0; i < NK_KEY_MAX; i++) {
        if (_snuklear.keys_down[i]) {
            nk_input_key(&_snuklear.ctx, (enum nk_keys)i, true);
            _snuklear.keys_down[i] = 0;
        }
        if (_snuklear.keys_up[i]) {
            nk_input_key(&_snuklear.ctx, (enum nk_keys)i, false);
            _snuklear.keys_up[i] = 0;
        }
    }
    nk_input_end(&_snuklear.ctx);
    #endif

    nk_clear(&_snuklear.ctx);
    return &_snuklear.ctx;
}

SOKOL_API_IMPL snk_image_t snk_make_image(const snk_image_desc_t* desc) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    SOKOL_ASSERT(desc);
    const snk_image_desc_t desc_def = _snk_image_desc_defaults(desc);
    snk_image_t img_id = _snk_alloc_image();
    _snk_image_t* img = _snk_lookup_image(img_id.id);
    if (img) {
        img->slot.state = _snk_init_image(img, &desc_def);
        SOKOL_ASSERT((img->slot.state == _SNK_RESOURCESTATE_VALID) || (img->slot.state == _SNK_RESOURCESTATE_FAILED));
    } else {
        _SNK_ERROR(IMAGE_POOL_EXHAUSTED);
    }
    return img_id;
}

SOKOL_API_IMPL void snk_destroy_image(snk_image_t img_id) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    _snk_destroy_image(img_id);
}

SOKOL_API_IMPL snk_image_desc_t snk_query_image_desc(snk_image_t img_id) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    _snk_image_t* img = _snk_lookup_image(img_id.id);
    if (img) {
        return (snk_image_desc_t){
            .image = img->image,
            .sampler = img->sampler,
        };
    } else {
        return (snk_image_desc_t){0};
    }
}

SOKOL_API_IMPL nk_handle snk_nkhandle(snk_image_t img) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    return (nk_handle) { .id = (int)img.id };
}

SOKOL_API_IMPL snk_image_t snk_image_from_nkhandle(nk_handle h) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    return (snk_image_t){ .id = (uint32_t) h.id };
}

static void _snk_bind_image_sampler(sg_bindings* bindings, nk_handle h) {
    _snk_image_t* img = _snk_lookup_image((uint32_t)h.id);
    if (img) {
        bindings->fs.images[0] = img->image;
        bindings->fs.samplers[0] = img->sampler;
    } else {
        bindings->fs.images[0] = _snuklear.def_img;
        bindings->fs.samplers[0] = _snuklear.def_smp;
    }
}

SOKOL_API_IMPL void snk_render(int width, int height) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct _snk_vertex_t, pos)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct _snk_vertex_t, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct _snk_vertex_t, col)},
        {NK_VERTEX_LAYOUT_END}
    };
    struct nk_convert_config cfg = {
        .shape_AA = NK_ANTI_ALIASING_ON,
        .line_AA = NK_ANTI_ALIASING_ON,
        .vertex_layout = vertex_layout,
        .vertex_size = sizeof(_snk_vertex_t),
        .vertex_alignment = 4,
        .circle_segment_count = 22,
        .curve_segment_count = 22,
        .arc_segment_count = 22,
        .global_alpha = 1.0f
    };

    _snuklear.vs_params.disp_size[0] = (float)width;
    _snuklear.vs_params.disp_size[1] = (float)height;

    // Setup vert/index buffers and convert
    struct nk_buffer cmds, verts, idx;
    nk_buffer_init_default(&cmds);
    nk_buffer_init_default(&verts);
    nk_buffer_init_default(&idx);
    nk_convert(&_snuklear.ctx, &cmds, &verts, &idx, &cfg);

    // Check for vertex- and index-buffer overflow, assert in debug-mode,
    // otherwise silently skip rendering
    const bool vertex_buffer_overflow = nk_buffer_total(&verts) > _snuklear.vertex_buffer_size;
    const bool index_buffer_overflow = nk_buffer_total(&idx) > _snuklear.index_buffer_size;
    SOKOL_ASSERT(!vertex_buffer_overflow && !index_buffer_overflow);
    if (!vertex_buffer_overflow && !index_buffer_overflow) {

        // Setup rendering
        sg_update_buffer(_snuklear.vbuf, &(sg_range){ nk_buffer_memory_const(&verts), nk_buffer_total(&verts) });
        sg_update_buffer(_snuklear.ibuf, &(sg_range){ nk_buffer_memory_const(&idx), nk_buffer_total(&idx) });
        const float dpi_scale = _snuklear.desc.dpi_scale;
        const int fb_width = (int)(_snuklear.vs_params.disp_size[0] * dpi_scale);
        const int fb_height = (int)(_snuklear.vs_params.disp_size[1] * dpi_scale);
        sg_apply_viewport(0, 0, fb_width, fb_height, true);
        sg_apply_scissor_rect(0, 0, fb_width, fb_height, true);
        sg_apply_pipeline(_snuklear.pip);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(_snuklear.vs_params));

        // Iterate through the command list, rendering each one
        const struct nk_draw_command* cmd = NULL;
        int idx_offset = 0;
        sg_bindings bindings = {
            .vertex_buffers[0] = _snuklear.vbuf,
            .index_buffer = _snuklear.ibuf,
            .index_buffer_offset = idx_offset
        };
        nk_draw_foreach(cmd, &_snuklear.ctx, &cmds) {
            if (cmd->elem_count > 0) {
                _snk_bind_image_sampler(&bindings, cmd->texture);
                sg_apply_bindings(&bindings);
                sg_apply_scissor_rectf(cmd->clip_rect.x * dpi_scale,
                                       cmd->clip_rect.y * dpi_scale,
                                       cmd->clip_rect.w * dpi_scale,
                                       cmd->clip_rect.h * dpi_scale,
                                       true);
                sg_draw(0, (int)cmd->elem_count, 1);
                bindings.index_buffer_offset += (int)cmd->elem_count * (int)sizeof(uint16_t);
            }
        }
        sg_apply_scissor_rect(0, 0, fb_width, fb_height, true);
    }

    // Cleanup
    nk_buffer_free(&cmds);
    nk_buffer_free(&verts);
    nk_buffer_free(&idx);
}

#if !defined(SOKOL_NUKLEAR_NO_SOKOL_APP)
_SOKOL_PRIVATE bool _snk_is_ctrl(uint32_t modifiers) {
    if (_snuklear.is_osx) {
        return 0 != (modifiers & SAPP_MODIFIER_SUPER);
    } else {
        return 0 != (modifiers & SAPP_MODIFIER_CTRL);
    }
}

_SOKOL_PRIVATE void _snk_append_char(uint32_t char_code) {
    size_t idx = strlen(_snuklear.char_buffer);
    if (idx<NK_INPUT_MAX) {
        _snuklear.char_buffer[idx] = (char)char_code;
    }
}

_SOKOL_PRIVATE enum nk_keys _snk_event_to_nuklearkey(const sapp_event* ev) {
    switch (ev->key_code) {
        case SAPP_KEYCODE_C:
            if (_snk_is_ctrl(ev->modifiers)) {
                return NK_KEY_COPY;
            } else {
                return NK_KEY_NONE;
            }
            break;
        case SAPP_KEYCODE_X:
            if (_snk_is_ctrl(ev->modifiers)) {
                return NK_KEY_CUT;
            } else {
                return NK_KEY_NONE;
            }
            break;
        case SAPP_KEYCODE_A:
            if (_snk_is_ctrl(ev->modifiers)) {
                return NK_KEY_TEXT_SELECT_ALL;
            } else {
                return NK_KEY_NONE;
            }
            break;
        case SAPP_KEYCODE_Z:
            if (_snk_is_ctrl(ev->modifiers)) {
                if (ev->modifiers & SAPP_MODIFIER_SHIFT) {
                    return NK_KEY_TEXT_REDO;
                } else {
                    return NK_KEY_TEXT_UNDO;
                }
            } else {
                return NK_KEY_NONE;
            }
            break;
        case SAPP_KEYCODE_DELETE: return NK_KEY_DEL;
        case SAPP_KEYCODE_ENTER: return NK_KEY_ENTER;
        case SAPP_KEYCODE_TAB: return NK_KEY_TAB;
        case SAPP_KEYCODE_BACKSPACE: return NK_KEY_BACKSPACE;
        case SAPP_KEYCODE_UP: return NK_KEY_UP;
        case SAPP_KEYCODE_DOWN: return NK_KEY_DOWN;
        case SAPP_KEYCODE_LEFT: return NK_KEY_LEFT;
        case SAPP_KEYCODE_RIGHT: return NK_KEY_RIGHT;
        case SAPP_KEYCODE_LEFT_SHIFT: return NK_KEY_SHIFT;
        case SAPP_KEYCODE_RIGHT_SHIFT: return NK_KEY_SHIFT;
        case SAPP_KEYCODE_LEFT_CONTROL: return NK_KEY_CTRL;
        case SAPP_KEYCODE_RIGHT_CONTROL: return NK_KEY_CTRL;
        default:
            return NK_KEY_NONE;
    }
}

SOKOL_API_IMPL bool snk_handle_event(const sapp_event* ev) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    const float dpi_scale = _snuklear.desc.dpi_scale;
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            _snuklear.mouse_pos[0] = (int) (ev->mouse_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->mouse_y / dpi_scale);
            switch (ev->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT:
                    _snuklear.btn_down[NK_BUTTON_LEFT] = true;
                    break;
                case SAPP_MOUSEBUTTON_RIGHT:
                    _snuklear.btn_down[NK_BUTTON_RIGHT] = true;
                    break;
                case SAPP_MOUSEBUTTON_MIDDLE:
                    _snuklear.btn_down[NK_BUTTON_MIDDLE] = true;
                    break;
                default:
                    break;
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            _snuklear.mouse_pos[0] = (int) (ev->mouse_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->mouse_y / dpi_scale);
            switch (ev->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT:
                    _snuklear.btn_up[NK_BUTTON_LEFT] = true;
                    break;
                case SAPP_MOUSEBUTTON_RIGHT:
                    _snuklear.btn_up[NK_BUTTON_RIGHT] = true;
                    break;
                case SAPP_MOUSEBUTTON_MIDDLE:
                    _snuklear.btn_up[NK_BUTTON_MIDDLE] = true;
                    break;
                default:
                    break;
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            _snuklear.mouse_pos[0] = (int) (ev->mouse_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->mouse_y / dpi_scale);
            _snuklear.mouse_did_move = true;
            break;
        case SAPP_EVENTTYPE_MOUSE_ENTER:
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
            for (int i = 0; i < NK_BUTTON_MAX; i++) {
                _snuklear.btn_down[i] = false;
                _snuklear.btn_up[i] = false;
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            _snuklear.mouse_scroll[0] = ev->scroll_x;
            _snuklear.mouse_scroll[1] = ev->scroll_y;
            _snuklear.mouse_did_scroll = true;
            break;
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
            _snuklear.btn_down[NK_BUTTON_LEFT] = true;
            _snuklear.mouse_pos[0] = (int) (ev->touches[0].pos_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->touches[0].pos_y / dpi_scale);
            _snuklear.mouse_did_move = true;
            break;
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
            _snuklear.mouse_pos[0] = (int) (ev->touches[0].pos_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->touches[0].pos_y / dpi_scale);
            _snuklear.mouse_did_move = true;
            break;
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
            _snuklear.btn_up[NK_BUTTON_LEFT] = true;
            _snuklear.mouse_pos[0] = (int) (ev->touches[0].pos_x / dpi_scale);
            _snuklear.mouse_pos[1] = (int) (ev->touches[0].pos_y / dpi_scale);
            _snuklear.mouse_did_move = true;
            break;
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
            _snuklear.btn_up[NK_BUTTON_LEFT] = false;
            _snuklear.btn_down[NK_BUTTON_LEFT] = false;
            break;
        case SAPP_EVENTTYPE_KEY_DOWN:
            /* intercept Ctrl-V, this is handled via EVENTTYPE_CLIPBOARD_PASTED */
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_V)) {
                break;
            }
            /* on web platform, don't forward Ctrl-X, Ctrl-V to the browser */
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_X)) {
                sapp_consume_event();
            }
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_C)) {
                sapp_consume_event();
            }
            _snuklear.keys_down[_snk_event_to_nuklearkey(ev)] = true;
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            /* intercept Ctrl-V, this is handled via EVENTTYPE_CLIPBOARD_PASTED */
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_V)) {
                break;
            }
            /* on web platform, don't forward Ctrl-X, Ctrl-V to the browser */
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_X)) {
                sapp_consume_event();
            }
            if (_snk_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_C)) {
                sapp_consume_event();
            }
            _snuklear.keys_up[_snk_event_to_nuklearkey(ev)] = true;
            break;
        case SAPP_EVENTTYPE_CHAR:
            if ((ev->char_code >= 32) &&
                (ev->char_code != 127) &&
                (0 == (ev->modifiers & (SAPP_MODIFIER_ALT|SAPP_MODIFIER_CTRL|SAPP_MODIFIER_SUPER))))
            {
                _snk_append_char(ev->char_code);
            }
            break;
        case SAPP_EVENTTYPE_CLIPBOARD_PASTED:
            _snuklear.keys_down[NK_KEY_PASTE] = _snuklear.keys_up[NK_KEY_PASTE] = true;
            break;
        default:
            break;
    }
    return nk_item_is_any_active(&_snuklear.ctx);
}

SOKOL_API_IMPL nk_flags snk_edit_string(struct nk_context *ctx, nk_flags flags, char *memory, int *len, int max, nk_plugin_filter filter) {
    SOKOL_ASSERT(_SNK_INIT_COOKIE == _snuklear.init_cookie);
    nk_flags event = nk_edit_string(ctx, flags, memory, len, max, filter);
    if ((event & NK_EDIT_ACTIVATED) && !sapp_keyboard_shown()) {
        sapp_show_keyboard(true);
    }
    if ((event & NK_EDIT_DEACTIVATED) && sapp_keyboard_shown()) {
        sapp_show_keyboard(false);
    }
    return event;
}
#endif // SOKOL_NUKLEAR_NO_SOKOL_APP

#endif // SOKOL_IMPL
