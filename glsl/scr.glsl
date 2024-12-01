@module s

@vs vs
@glsl_options flip_vert_y

in vec2 pos;
out vec2 uv0;

void main(){
	gl_Position = vec4(2 * pos - 1, 0.5, 1.0);
	uv0 = vec2(pos.x, 1 - pos.y);
}
@end

@fs fs

layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler smp;
in vec2 uv0;
out vec4 col;

void main(){
	col = texture(sampler2D(tex, smp), uv0);
}
@end

@program scr vs fs
