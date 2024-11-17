@module s

@vs vs
precision mediump float;

in vec2 pos;
out vec2 uv0;

void main(){
	gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
	uv0 = pos;
}
@end

@fs fs
precision mediump float;

layout(binding=0) uniform texture2D tex;
layout(binding=0) uniform sampler smp;
in vec2 uv0;
out vec4 col;

void main(){
	col = texture(sampler2D(tex, smp), uv0);
}
@end

@program scr vs fs
