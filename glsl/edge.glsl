@module e

@vs vs
precision mediump float;

layout(binding=0) uniform Vparam {
	mat4 mvp;
};
in vec3 pos;

void main(){
	gl_Position = mvp * vec4(pos, 1.0);
}
@end

@fs fs
precision mediump float;

layout(binding=1) uniform Fparam {
	vec4 color;
};
out vec4 fc;

void main(){
	fc = color;
}
@end

@program edge vs fs
