#pragma once
/*
    #version:1# (machine generated, don't edit!)

    Generated by sokol-shdc (https://github.com/floooh/sokol-tools)

    Cmdline:
        sokol-shdc -f sokol_impl -i glsl/node.glsl -l glsl430:hlsl5 -o glsl/node.h

    Overview:
    =========
    Shader program: 'node':
        Get shader desc: n_node_shader_desc(sg_query_backend());
        Vertex Shader: vs
        Fragment Shader: fs
        Attributes:
            ATTR_node_geom => 0
            ATTR_node_pos => 1
            ATTR_node_dir => 2
            ATTR_node_col0 => 3
            ATTR_node_len => 4
    Bindings:
        Uniform block 'Vparam':
            C struct: n_Vparam_t
            Bind slot: UB_Vparam => 0
*/
#if !defined(SOKOL_GFX_INCLUDED)
#error "Please include sokol_gfx.h before node.h"
#endif
#if !defined(SOKOL_SHDC_ALIGN)
#if defined(_MSC_VER)
#define SOKOL_SHDC_ALIGN(a) __declspec(align(a))
#else
#define SOKOL_SHDC_ALIGN(a) __attribute__((aligned(a)))
#endif
#endif
const sg_shader_desc* n_node_shader_desc(sg_backend backend);
#define ATTR_node_geom (0)
#define ATTR_node_pos (1)
#define ATTR_node_dir (2)
#define ATTR_node_col0 (3)
#define ATTR_node_len (4)
#define UB_Vparam (0)
#pragma pack(push,1)
SOKOL_SHDC_ALIGN(16) typedef struct n_Vparam_t {
    float mvp[16];
} n_Vparam_t;
#pragma pack(pop)
#if defined(SOKOL_SHDC_IMPL)
/*
    #version 430

    uniform vec4 Vparam[4];
    layout(location = 0) in vec2 geom;
    layout(location = 4) in float len;
    layout(location = 2) in vec3 dir;
    layout(location = 1) in vec3 pos;
    layout(location = 0) out vec4 col;
    layout(location = 3) in vec4 col0;

    vec2 rotatez(vec2 v, float c, float s)
    {
        return mat2(vec2(c, s), vec2(-s, c)) * v;
    }

    void main()
    {
        vec2 param = geom * vec2(len, 1.0);
        float param_1 = dir.x;
        float param_2 = dir.y;
        gl_Position = mat4(Vparam[0], Vparam[1], Vparam[2], Vparam[3]) * vec4(vec3(rotatez(param, param_1, param_2), 0.0) + pos, 1.0);
        col = col0;
    }

*/
static const uint8_t n_vs_source_glsl430[596] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x34,0x33,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x76,0x65,0x63,0x34,0x20,0x56,0x70,0x61,0x72,0x61,
    0x6d,0x5b,0x34,0x5d,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,
    0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,
    0x63,0x32,0x20,0x67,0x65,0x6f,0x6d,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,
    0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x34,0x29,0x20,0x69,0x6e,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x6c,0x65,0x6e,0x3b,0x0a,0x6c,0x61,0x79,0x6f,
    0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x32,0x29,
    0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x33,0x20,0x64,0x69,0x72,0x3b,0x0a,0x6c,0x61,
    0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,
    0x31,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x33,0x20,0x70,0x6f,0x73,0x3b,0x0a,
    0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,
    0x3d,0x20,0x30,0x29,0x20,0x6f,0x75,0x74,0x20,0x76,0x65,0x63,0x34,0x20,0x63,0x6f,
    0x6c,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x33,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,
    0x63,0x6f,0x6c,0x30,0x3b,0x0a,0x0a,0x76,0x65,0x63,0x32,0x20,0x72,0x6f,0x74,0x61,
    0x74,0x65,0x7a,0x28,0x76,0x65,0x63,0x32,0x20,0x76,0x2c,0x20,0x66,0x6c,0x6f,0x61,
    0x74,0x20,0x63,0x2c,0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x73,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,0x20,0x6d,0x61,0x74,0x32,0x28,
    0x76,0x65,0x63,0x32,0x28,0x63,0x2c,0x20,0x73,0x29,0x2c,0x20,0x76,0x65,0x63,0x32,
    0x28,0x2d,0x73,0x2c,0x20,0x63,0x29,0x29,0x20,0x2a,0x20,0x76,0x3b,0x0a,0x7d,0x0a,
    0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,
    0x20,0x20,0x20,0x76,0x65,0x63,0x32,0x20,0x70,0x61,0x72,0x61,0x6d,0x20,0x3d,0x20,
    0x67,0x65,0x6f,0x6d,0x20,0x2a,0x20,0x76,0x65,0x63,0x32,0x28,0x6c,0x65,0x6e,0x2c,
    0x20,0x31,0x2e,0x30,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,
    0x20,0x70,0x61,0x72,0x61,0x6d,0x5f,0x31,0x20,0x3d,0x20,0x64,0x69,0x72,0x2e,0x78,
    0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x70,0x61,0x72,0x61,
    0x6d,0x5f,0x32,0x20,0x3d,0x20,0x64,0x69,0x72,0x2e,0x79,0x3b,0x0a,0x20,0x20,0x20,
    0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x6d,
    0x61,0x74,0x34,0x28,0x56,0x70,0x61,0x72,0x61,0x6d,0x5b,0x30,0x5d,0x2c,0x20,0x56,
    0x70,0x61,0x72,0x61,0x6d,0x5b,0x31,0x5d,0x2c,0x20,0x56,0x70,0x61,0x72,0x61,0x6d,
    0x5b,0x32,0x5d,0x2c,0x20,0x56,0x70,0x61,0x72,0x61,0x6d,0x5b,0x33,0x5d,0x29,0x20,
    0x2a,0x20,0x76,0x65,0x63,0x34,0x28,0x76,0x65,0x63,0x33,0x28,0x72,0x6f,0x74,0x61,
    0x74,0x65,0x7a,0x28,0x70,0x61,0x72,0x61,0x6d,0x2c,0x20,0x70,0x61,0x72,0x61,0x6d,
    0x5f,0x31,0x2c,0x20,0x70,0x61,0x72,0x61,0x6d,0x5f,0x32,0x29,0x2c,0x20,0x30,0x2e,
    0x30,0x29,0x20,0x2b,0x20,0x70,0x6f,0x73,0x2c,0x20,0x31,0x2e,0x30,0x29,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x63,0x6f,0x6c,0x20,0x3d,0x20,0x63,0x6f,0x6c,0x30,0x3b,0x0a,
    0x7d,0x0a,0x0a,0x00,
};
/*
    #version 430

    layout(location = 0) out vec4 c0;
    layout(location = 0) in vec4 col;

    void main()
    {
        c0 = col;
    }

*/
static const uint8_t n_fs_source_glsl430[115] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x34,0x33,0x30,0x0a,0x0a,0x6c,0x61,
    0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,
    0x30,0x29,0x20,0x6f,0x75,0x74,0x20,0x76,0x65,0x63,0x34,0x20,0x63,0x30,0x3b,0x0a,
    0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,
    0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,0x63,0x6f,0x6c,
    0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x63,0x30,0x20,0x3d,0x20,0x63,0x6f,0x6c,0x3b,0x0a,0x7d,
    0x0a,0x0a,0x00,
};
/*
    cbuffer Vparam : register(b0)
    {
        row_major float4x4 _70_mvp : packoffset(c0);
    };


    static float4 gl_Position;
    static float2 geom;
    static float len;
    static float3 dir;
    static float3 pos;
    static float4 col;
    static float4 col0;

    struct SPIRV_Cross_Input
    {
        float2 geom : TEXCOORD0;
        float3 pos : TEXCOORD1;
        float3 dir : TEXCOORD2;
        float4 col0 : TEXCOORD3;
        float len : TEXCOORD4;
    };

    struct SPIRV_Cross_Output
    {
        float4 col : TEXCOORD0;
        float4 gl_Position : SV_Position;
    };

    float2 rotatez(float2 v, float c, float s)
    {
        return mul(v, float2x2(float2(c, s), float2(-s, c)));
    }

    void vert_main()
    {
        float2 param = geom * float2(len, 1.0f);
        float param_1 = dir.x;
        float param_2 = dir.y;
        gl_Position = mul(float4(float3(rotatez(param, param_1, param_2), 0.0f) + pos, 1.0f), _70_mvp);
        col = col0;
    }

    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        geom = stage_input.geom;
        len = stage_input.len;
        dir = stage_input.dir;
        pos = stage_input.pos;
        col0 = stage_input.col0;
        vert_main();
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.col = col;
        return stage_output;
    }
*/
static const uint8_t n_vs_source_hlsl5[1192] = {
    0x63,0x62,0x75,0x66,0x66,0x65,0x72,0x20,0x56,0x70,0x61,0x72,0x61,0x6d,0x20,0x3a,
    0x20,0x72,0x65,0x67,0x69,0x73,0x74,0x65,0x72,0x28,0x62,0x30,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x72,0x6f,0x77,0x5f,0x6d,0x61,0x6a,0x6f,0x72,0x20,0x66,0x6c,
    0x6f,0x61,0x74,0x34,0x78,0x34,0x20,0x5f,0x37,0x30,0x5f,0x6d,0x76,0x70,0x20,0x3a,
    0x20,0x70,0x61,0x63,0x6b,0x6f,0x66,0x66,0x73,0x65,0x74,0x28,0x63,0x30,0x29,0x3b,
    0x0a,0x7d,0x3b,0x0a,0x0a,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,
    0x61,0x74,0x34,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,
    0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x67,
    0x65,0x6f,0x6d,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,
    0x74,0x20,0x6c,0x65,0x6e,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,
    0x6f,0x61,0x74,0x33,0x20,0x64,0x69,0x72,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x33,0x20,0x70,0x6f,0x73,0x3b,0x0a,0x73,0x74,0x61,
    0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x6f,0x6c,0x3b,0x0a,
    0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x6f,
    0x6c,0x30,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,
    0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x67,0x65,0x6f,0x6d,0x20,
    0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x20,0x20,0x20,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x33,0x20,0x70,0x6f,0x73,0x20,0x3a,0x20,0x54,0x45,
    0x58,0x43,0x4f,0x4f,0x52,0x44,0x31,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,
    0x61,0x74,0x33,0x20,0x64,0x69,0x72,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,
    0x52,0x44,0x32,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,
    0x63,0x6f,0x6c,0x30,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x33,
    0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x6c,0x65,0x6e,0x20,
    0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x34,0x3b,0x0a,0x7d,0x3b,0x0a,
    0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,
    0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x6f,0x6c,0x20,0x3a,0x20,0x54,0x45,
    0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,
    0x61,0x74,0x34,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,
    0x3a,0x20,0x53,0x56,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x7d,
    0x3b,0x0a,0x0a,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x72,0x6f,0x74,0x61,0x74,0x65,
    0x7a,0x28,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x76,0x2c,0x20,0x66,0x6c,0x6f,0x61,
    0x74,0x20,0x63,0x2c,0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x73,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,0x20,0x6d,0x75,0x6c,0x28,0x76,
    0x2c,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x78,0x32,0x28,0x66,0x6c,0x6f,0x61,0x74,
    0x32,0x28,0x63,0x2c,0x20,0x73,0x29,0x2c,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x28,
    0x2d,0x73,0x2c,0x20,0x63,0x29,0x29,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x76,0x6f,0x69,
    0x64,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x70,0x61,0x72,0x61,0x6d,
    0x20,0x3d,0x20,0x67,0x65,0x6f,0x6d,0x20,0x2a,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,
    0x28,0x6c,0x65,0x6e,0x2c,0x20,0x31,0x2e,0x30,0x66,0x29,0x3b,0x0a,0x20,0x20,0x20,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x20,0x70,0x61,0x72,0x61,0x6d,0x5f,0x31,0x20,0x3d,
    0x20,0x64,0x69,0x72,0x2e,0x78,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,
    0x74,0x20,0x70,0x61,0x72,0x61,0x6d,0x5f,0x32,0x20,0x3d,0x20,0x64,0x69,0x72,0x2e,
    0x79,0x3b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x6d,0x75,0x6c,0x28,0x66,0x6c,0x6f,0x61,0x74,0x34,0x28,
    0x66,0x6c,0x6f,0x61,0x74,0x33,0x28,0x72,0x6f,0x74,0x61,0x74,0x65,0x7a,0x28,0x70,
    0x61,0x72,0x61,0x6d,0x2c,0x20,0x70,0x61,0x72,0x61,0x6d,0x5f,0x31,0x2c,0x20,0x70,
    0x61,0x72,0x61,0x6d,0x5f,0x32,0x29,0x2c,0x20,0x30,0x2e,0x30,0x66,0x29,0x20,0x2b,
    0x20,0x70,0x6f,0x73,0x2c,0x20,0x31,0x2e,0x30,0x66,0x29,0x2c,0x20,0x5f,0x37,0x30,
    0x5f,0x6d,0x76,0x70,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x63,0x6f,0x6c,0x20,0x3d,
    0x20,0x63,0x6f,0x6c,0x30,0x3b,0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,
    0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,
    0x6e,0x28,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,
    0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,
    0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x67,0x65,0x6f,0x6d,0x20,0x3d,0x20,0x73,0x74,
    0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x67,0x65,0x6f,0x6d,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x6c,0x65,0x6e,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,
    0x69,0x6e,0x70,0x75,0x74,0x2e,0x6c,0x65,0x6e,0x3b,0x0a,0x20,0x20,0x20,0x20,0x64,
    0x69,0x72,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,
    0x2e,0x64,0x69,0x72,0x3b,0x0a,0x20,0x20,0x20,0x20,0x70,0x6f,0x73,0x20,0x3d,0x20,
    0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x70,0x6f,0x73,0x3b,
    0x0a,0x20,0x20,0x20,0x20,0x63,0x6f,0x6c,0x30,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,
    0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x63,0x6f,0x6c,0x30,0x3b,0x0a,0x20,0x20,
    0x20,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x3b,0x0a,0x20,
    0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,
    0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,
    0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,
    0x74,0x70,0x75,0x74,0x2e,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,
    0x20,0x3d,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,
    0x2e,0x63,0x6f,0x6c,0x20,0x3d,0x20,0x63,0x6f,0x6c,0x3b,0x0a,0x20,0x20,0x20,0x20,
    0x72,0x65,0x74,0x75,0x72,0x6e,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,
    0x70,0x75,0x74,0x3b,0x0a,0x7d,0x0a,0x00,
};
/*
    static float4 c0;
    static float4 col;

    struct SPIRV_Cross_Input
    {
        float4 col : TEXCOORD0;
    };

    struct SPIRV_Cross_Output
    {
        float4 c0 : SV_Target0;
    };

    void frag_main()
    {
        c0 = col;
    }

    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        col = stage_input.col;
        frag_main();
        SPIRV_Cross_Output stage_output;
        stage_output.c0 = c0;
        return stage_output;
    }
*/
static const uint8_t n_fs_source_hlsl5[385] = {
    0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x30,
    0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,
    0x63,0x6f,0x6c,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,
    0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x6f,0x6c,0x20,
    0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,
    0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,
    0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x63,0x30,0x20,0x3a,0x20,0x53,0x56,0x5f,
    0x54,0x61,0x72,0x67,0x65,0x74,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x76,0x6f,0x69,
    0x64,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x63,0x30,0x20,0x3d,0x20,0x63,0x6f,0x6c,0x3b,0x0a,0x7d,0x0a,
    0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,
    0x70,0x75,0x74,0x20,0x6d,0x61,0x69,0x6e,0x28,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,
    0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,
    0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x63,0x6f,
    0x6c,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,
    0x63,0x6f,0x6c,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,
    0x69,0x6e,0x28,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,
    0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,
    0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,
    0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x2e,0x63,0x30,0x20,0x3d,
    0x20,0x63,0x30,0x3b,0x0a,0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,0x20,
    0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x7d,0x0a,
    0x00,
};
const sg_shader_desc* n_node_shader_desc(sg_backend backend) {
    if (backend == SG_BACKEND_GLCORE) {
        static sg_shader_desc desc;
        static bool valid;
        if (!valid) {
            valid = true;
            desc.vertex_func.source = (const char*)n_vs_source_glsl430;
            desc.vertex_func.entry = "main";
            desc.fragment_func.source = (const char*)n_fs_source_glsl430;
            desc.fragment_func.entry = "main";
            desc.attrs[0].glsl_name = "geom";
            desc.attrs[1].glsl_name = "pos";
            desc.attrs[2].glsl_name = "dir";
            desc.attrs[3].glsl_name = "col0";
            desc.attrs[4].glsl_name = "len";
            desc.uniform_blocks[0].stage = SG_SHADERSTAGE_VERTEX;
            desc.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
            desc.uniform_blocks[0].size = 64;
            desc.uniform_blocks[0].glsl_uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
            desc.uniform_blocks[0].glsl_uniforms[0].array_count = 4;
            desc.uniform_blocks[0].glsl_uniforms[0].glsl_name = "Vparam";
            desc.label = "n_node_shader";
        }
        return &desc;
    }
    if (backend == SG_BACKEND_D3D11) {
        static sg_shader_desc desc;
        static bool valid;
        if (!valid) {
            valid = true;
            desc.vertex_func.source = (const char*)n_vs_source_hlsl5;
            desc.vertex_func.d3d11_target = "vs_5_0";
            desc.vertex_func.entry = "main";
            desc.fragment_func.source = (const char*)n_fs_source_hlsl5;
            desc.fragment_func.d3d11_target = "ps_5_0";
            desc.fragment_func.entry = "main";
            desc.attrs[0].hlsl_sem_name = "TEXCOORD";
            desc.attrs[0].hlsl_sem_index = 0;
            desc.attrs[1].hlsl_sem_name = "TEXCOORD";
            desc.attrs[1].hlsl_sem_index = 1;
            desc.attrs[2].hlsl_sem_name = "TEXCOORD";
            desc.attrs[2].hlsl_sem_index = 2;
            desc.attrs[3].hlsl_sem_name = "TEXCOORD";
            desc.attrs[3].hlsl_sem_index = 3;
            desc.attrs[4].hlsl_sem_name = "TEXCOORD";
            desc.attrs[4].hlsl_sem_index = 4;
            desc.uniform_blocks[0].stage = SG_SHADERSTAGE_VERTEX;
            desc.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
            desc.uniform_blocks[0].size = 64;
            desc.uniform_blocks[0].hlsl_register_b_n = 0;
            desc.label = "n_node_shader";
        }
        return &desc;
    }
    return 0;
}
#endif // SOKOL_SHDC_IMPL