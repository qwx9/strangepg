@module node

@vs vs
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec2 geom;
in vec3 pos;
in vec4 dir;
in vec4 col;
in float len;
out vec4 c;

vec3 rotate(vec3 v, vec4 q)
{
	vec3 u = q.xyz;
	float s = q.w;
	return 2.0 * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0 * s * cross(u, v);
}

void main(){
	vec3 g = vec3(geom * vec2(len, 1.0), 0.0);
	vec3 r = rotate(g, dir);
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
in vec4 dir;
in float len;
flat out uint idx;

vec3 rotate(vec3 v, vec4 q)
{
	vec3 u = q.xyz;
	float s = q.w;
	return 2.0 * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0 * s * cross(u, v);
}

void main(){
	vec3 g = vec3(geom * vec2(len, 1.0), 0.0);
	vec3 r = rotate(g, dir);
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

@vs vs3d
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 geom;
in vec3 pos;
in vec4 dir;
in vec4 col;
in float len;
out vec4 c;
out flat uint ci;

vec3 rotate(vec3 v, vec4 q)
{
	vec3 u = q.xyz;
	float s = q.w;
	return 2.0 * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0 * s * cross(u, v);
}

void main(){
	vec3 g = geom * vec3(len, 1.0, 1.0);
	vec3 r = rotate(g, dir);
	gl_Position = mvp * vec4(r + pos, 1.0);
	c = col;
	ci = gl_VertexIndex / 2;
}
@end

@fs fs3d
precision mediump float;

in vec4 c;
in flat uint ci;
out vec4 fc;

void main(){
	vec4 m = vec4(0.0115, 0.0115, 0.0115, 0.0) * ci;
	fc = c - m;
}
@end

@vs vsoff3d
@glsl_options fixup_clipspace

precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 geom;
in vec3 pos;
in vec4 dir;
in float len;
flat out uint idx;

vec3 rotate(vec3 v, vec4 q)
{
	vec3 u = q.xyz;
	float s = q.w;
	return 2.0 * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0 * s * cross(u, v);
}

void main(){
	vec3 g = geom * vec3(len, 1.0, 1.0);
	vec3 r = rotate(g, dir);
	gl_Position = mvp * vec4(r + pos, 1.0);
	idx = gl_InstanceIndex + 1;
}
@end

@fs fsoff3d
precision mediump float;

flat in uint idx;
out uint fc;

void main(){
	fc = idx;
}
@end

@program idx vsoff fsoff
@program s vs fs
@program idx3d vsoff3d fsoff3d
@program s3d vs3d fs3d
