#version 430
//precision mediump float;
precision lowp float;

uniform mat4 mvp;
layout(location=0) in float v;
layout(location=1) in vec3 p1;
layout(location=2) in vec3 p2;
layout(location=3) in vec4 col0;
out vec4 col;
flat out uint idx;

void main(){
	vec3 p = gl_VertexID == 0 ? p1 : p2;
	gl_Position = mvp * vec4(p, 1.0);
	col = col0;
	idx = gl_InstanceID + 1 | 1<<31;
}
