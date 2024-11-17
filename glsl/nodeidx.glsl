@module ni

@vs vs
precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec2 geom;
in vec3 pos;
in vec3 dir;
in vec4 col0;
in float len;
out vec4 col;
flat out uint idx;

vec2 rotatez(vec2 v, float c, float s){
	return mat2(c, s, -s, c) * v;
}

void main(){
	vec2 g = geom * vec2(len, 1.0);
	vec3 r = vec3(rotatez(g, dir.x, dir.y), 0.0);
	gl_Position = mvp * vec4(r + pos, 1.0);
	col = col0;
	idx = gl_InstanceIndex + 1;
}
@end

@fs fs
precision mediump float;

in vec4 col;
flat in uint idx;

layout(location=0) out vec4 c0;
layout(location=1) out uint c1;

void main(){
	c0 = col;
	c1 = idx;
}
@end

@program nodeidx vs fs
