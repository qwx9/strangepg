#version 430
//precision mediump float;
precision lowp float;

uniform mat4 mvp;
uniform vec2 s;
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

void main(){
	float z = gl_InstanceID * 0.000001;
	gl_Position = mvp * vec4((rotatez(geom, theta) + pos) / s, z, 1.0);
	col = col0;
	idx = idx0;
	//idx = uint(gl_InstanceID);
}
