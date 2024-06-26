#version 430
//precision mediump float;
precision lowp float;

uniform mat4 mvp;
uniform vec2 s;
layout(location=0) in float v;
layout(location=1) in vec2 p1;
layout(location=2) in vec2 p2;
layout(location=3) in vec4 col0;
layout(location=4) in uint idx0;
out vec4 col;
flat out uint idx;

void main(){
	vec2 p = gl_VertexID == 0 ? p1 : p2;
	float z = gl_InstanceID * -0.000001;
	gl_Position = mvp * vec4(p / s, z, 1.0);
	col = col0;
	idx = idx0;
}
