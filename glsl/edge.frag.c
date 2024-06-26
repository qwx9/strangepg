char *edge_fragsh = \
	"#version 430\n"
	"in vec4 col;\n"
	"flat in uint idx;\n"
	"layout(location=0) out vec4 c0;\n"
	"layout(location=1) out uint c1;\n"
	"void main(){\n"
	"	c0 = col;\n"
	"	c1 = idx;\n"
	"}\n";
