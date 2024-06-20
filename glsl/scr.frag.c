char *scr_fragsh = \
	"#version 430\n"
	"uniform sampler2D tex0;\n"
	"in vec2 uv0;\n"
	"out vec4 frag_color;\n"
	"void main(){\n"
	"	vec3 c0 = texture(tex0, uv0).xyz;\n"
	"	frag_color = vec4(c0, 1.0);\n"
	"}\n";
