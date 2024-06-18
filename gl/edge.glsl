#version 430

@vs vs
precision lowp float;
uniform ue {
	mat4 mvp;
};
layout(location=0) in vec2 v;
layout(location=1) in vec2 p1;
layout(location=2) in vec2 p2;
layout(location=3) in vec3 col0;
layout(location=4) in uint idx0;
out vec3 col;
flat out uint idx;
void main() {
	vec2 scale = vec2(8.0,8.0);
	vec2 p = gl_VertexIndex == 0 ? p1 : p2;
	float z = gl_InstanceIndex * -0.000001;
	gl_Position = mvp * vec4(p / scale, z, 1.0);
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
	c0 = vec4(col, 0.25);
	c1 = idx;
}
@end
