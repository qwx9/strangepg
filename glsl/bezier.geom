#version 430

layout(lines) in;
layout(triangle_strip, max_vertices=6) out;

uniform mat4 mvp;

const float thicc = 1.0;

void
main(){
	/* FIXME: need to offset vertices for a proper miter, but there's no
	 * way to know where the next segment lies; allegedly solvable by
	 * using 6 control points instead of 4??
	 */
	vec2 n = normalize(gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy);
	vec4 cw = mvp * vec4(thicc * vec2(n.y, -n.x), 0.0, 0.0);
	gl_Position = gl_in[1].gl_Position + cw;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + cw;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position - cw;
	EmitVertex();
	EndPrimitive();
	gl_Position = gl_in[0].gl_Position - cw;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position - cw;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position + cw;
	EmitVertex();
	EndPrimitive();
}
