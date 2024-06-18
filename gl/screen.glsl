#version 430

@vs vs
layout(location=0) in vec2 pos;
out vec2 uv0;
void main() {
	gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);
	uv0 = pos;
}
@end

@fs fs
uniform texture2D tx;
uniform sampler sx;
in vec2 uv0;
out vec4 frag_color;
void main() {
	vec3 c0 = texture(sampler2D(tx, sx), uv0).xyz;
	frag_color = vec4(c0, 1.0);
}
@end
