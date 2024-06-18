#version 430

@vs vs
precision lowp float;
uniform un {
	mat4 mvp;
};
layout(location=0) in vec2 geom;
layout(location=1) in vec2 pos;
layout(location=2) in vec3 col0;
layout(location=3) in float theta;
layout(location=4) in uint idx0;
out vec3 col;
flat out uint idx;
vec2 rotatez(vec2 v, float angle){
	float s = sin(angle);
	float c = cos(angle);
	//return mat3(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0) * v;
	return mat2(c, -s, s, c) * v;
}
void main() {
	vec2 scale = vec2(8.0,8.0);
	float z = gl_InstanceIndex * 0.000001;
	gl_Position = mvp * vec4((rotatez(geom, theta) + pos) / scale, z, 1.0);
	col = col0;
	idx = idx0;
}
@end

@fs fs
in vec3 col;
flat in uint idx;
layout(location=0) out vec4 c0;
layout(location=1) out uint c1;
void main() {
	c0 = vec4(col, 0.6);
	c1 = idx;
}
@end
