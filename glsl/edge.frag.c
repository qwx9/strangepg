char *edge_fragsh = \
	"#version 430\n"
	"in vec3 col;\n"
	"flat in uint idx;\n"
	"layout(location=0) out vec4 c0;\n"
	"layout(location=1) out uint c1;\n"
	"void main(){\n"
	"	c0 = vec4(col, 0.2);\n"
	"	c1 = idx;\n"
	"}\n";
