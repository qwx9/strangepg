@module n

@vs vs
@glsl_options fixup_clipspace

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

vec2 rotatez(vec2 v, float c, float s){
	return mat2(c, s, -s, c) * v;
}

void main(){
	vec2 g = geom * vec2(len, 1.0);
	vec3 r = vec3(rotatez(g, dir.x, dir.y), 0.0);
	gl_Position = mvp * vec4(r + pos, 1.0);
	col = col0;
}
@end

@fs fs
precision mediump float;

in vec4 col;
out vec4 c0;

void main(){
	c0 = col;
}
@end

@program node vs fs
