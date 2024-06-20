#version 430

layout(location=0) in vec2 pos;
out vec2 uv0;

void main(){
	gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
	uv0 = pos;
}
