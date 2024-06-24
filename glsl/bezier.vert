#version 430

uniform mat4 mvp;

in vec2 pos;

void
main(){
	// note: clipping is done in vertex postproc, after the geom shader
	gl_Position = mvp * vec4(pos, 8.0, 1.0);
}
