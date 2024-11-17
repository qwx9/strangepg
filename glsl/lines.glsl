@module l

@vs vs
precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 pos;
in vec4 color;
out vec4 col0;

void main(){
	gl_Position = mvp * vec4(pos, 1.0);
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

@program line vs fs
