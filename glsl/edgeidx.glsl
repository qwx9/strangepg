@module ei

@vs vs
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 pos;
flat out uint idx;

void main(){
	gl_Position = mvp * vec4(pos, 1.0);
	idx = gl_InstanceIndex + 1 | 1<<31;
}
@end

@fs fs
precision mediump float;

layout(binding=1) uniform Fparam {
	vec4 color;
};
flat in uint idx;
out vec4 c0;
out uint c1;

void main(){
	c0 = color;
	c1 = idx;
}
@end

@program edgeidx vs fs
