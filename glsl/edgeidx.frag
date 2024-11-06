#version 430

in vec4 col;
flat in uint idx;
layout(location=0) out vec4 c0;
layout(location=1) out uint c1;

void main(){
	c0 = col;
	c1 = idx;
}
