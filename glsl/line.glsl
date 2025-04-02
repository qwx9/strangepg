@module line

@vs vs
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec4 pos;
in vec4 color;
out vec4 col0;

void main(){
	gl_Position = mvp * pos;
	col0 = color;
}
@end

@fs fs
precision mediump float;

in vec4 col0;
out vec4 fcol;

void main(){
	fcol = col0;
}
@end

@program s vs fs
