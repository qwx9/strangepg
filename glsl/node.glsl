@module node

@vs vs
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec2 geom;
in vec3 pos;
in vec3 dir;
in vec4 col;
in float len;
out vec4 c;

vec2 rotatez(vec2 v, float c, float s){
	return mat2(c, s, -s, c) * v;
}

void main(){
	vec2 g = geom * vec2(len, 1.0);
	vec3 r = vec3(rotatez(g, dir.x, dir.y), 0.0);
	gl_Position = mvp * vec4(r + pos, 1.0);
	c = col;
}
@end

@fs fs
precision mediump float;

in vec4 c;
out vec4 fc;

void main(){
	fc = c;
}
@end

@vs vsoff
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec2 geom;
in vec3 pos;
in vec3 dir;
in float len;
flat out uint idx;

vec2 rotatez(vec2 v, float c, float s){
	return mat2(c, s, -s, c) * v;
}

void main(){
	vec2 g = geom * vec2(len, 1.0);
	vec3 r = vec3(rotatez(g, dir.x, dir.y), 0.0);
	gl_Position = mvp * vec4(r + pos, 1.0);
	idx = gl_InstanceIndex + 1;
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
