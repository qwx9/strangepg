char *bezier_tcssh = \
	"#version 430\n"
	"layout(vertices=4) out;\n"
	"void\n"
	"main()\n"
	"{\n"
	"	if(gl_InvocationID == 0){\n"
	"		gl_TessLevelOuter[0] = 1;\n"
	"		gl_TessLevelOuter[1] = 4;\n"
	"	}\n"
	"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
	"}\n";
