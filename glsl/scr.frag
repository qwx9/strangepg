#version 430

uniform sampler2D tex0;
in vec2 uv0;
out vec4 col;

void main(){
	vec3 c0 = texture(tex0, uv0).xyz;
	col = vec4(c0, 1.0);
}
