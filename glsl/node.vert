#version 430
//precision mediump float;
precision lowp float;

uniform mat4 mvp;
layout(location=0) in vec2 geom;
layout(location=1) in vec3 pos;
layout(location=2) in vec3 dir;
layout(location=3) in vec4 col0;
layout(location=4) in float len;
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
	idx = gl_InstanceID + 1;
}
