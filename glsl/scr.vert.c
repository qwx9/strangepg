char *scr_vertsh = \
	"#version 430\n"
	"layout(location=0) in vec2 pos;\n"
	"out vec2 uv0;\n"
	"void main(){\n"
	"	gl_Position = vec4(pos*2.0-1.0, 0.5, 1.0);\n"
	"	uv0 = pos;\n"
	"}\n";
