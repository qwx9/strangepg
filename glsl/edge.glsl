@module edge

@vs vs
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 pos;

void main(){
	gl_Position = mvp * vec4(pos, 1.0);
}
@end

@fs fs
precision mediump float;

layout(binding=1) uniform Fparam {
	vec4 color;
};
out vec4 c;

void main(){
	c = color;
}
@end

@vs vsoff
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 pos;
flat out uint idx;

void main(){
	gl_Position = mvp * vec4(pos, 1.0);
	idx = (gl_VertexIndex >> 1) + 1 | 1<<31;
}
@end

@fs fsoff
precision mediump float;

flat in uint idx;
out uint fc;

void main(){
	fc = idx;
}
@end

@program idx vsoff fsoff
@program s vs fs
